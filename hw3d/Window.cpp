#include "Window.h"
#include "resource.h"
#include <sstream>

//Initialize static variable - Remember, we only need one window class
Window::WindowClass Window::WindowClass::wndClass;

//Window Class definitions
Window::WindowClass::WindowClass() noexcept
    :
    hInst(GetModuleHandle(nullptr))
{
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = HandleMsgSetup;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetInstance();
    wc.hIcon = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0));
    wc.hIconSm = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0));
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = GetName();
    RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass()
{
    UnregisterClass(GetName(), GetInstance());
}

const char* Window::WindowClass::GetName() noexcept
{
    return wndClassname;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
    return wndClass.hInst;
}

//Window definitions
Window::Window(int width, int height, const char* name)
    :
    width(width),
    height(height)
{
    //Calculate the window size based on the desired client region size
    RECT wr;
    wr.left = 100;
    wr.right = width + wr.left;
    wr.top = 100;
    wr.bottom = wr.top + height;
    const DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
    if (AdjustWindowRect(&wr, style, FALSE) == 0)
    {
        throw WND_LAST_EXCEPT();
    }

    //Create window and get hWnd
    hWnd = CreateWindow(
        WindowClass::GetName(), name,
        style,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, WindowClass::GetInstance(), this
    );
    if (hWnd == nullptr)
    {
        throw WND_LAST_EXCEPT();
    }
    //Show window
    ShowWindow(hWnd, SW_SHOWDEFAULT);
}

Window::~Window()
{
    DestroyWindow(hWnd);
}

void Window::SetTitle(const std::string title) noexcept
{
    SetWindowText(hWnd, title.c_str());
}

std::optional<int> Window::ProcessMessages()
{
    //Message pump
    MSG msg;

    //Non blocking message checking. If no messages are present in the queue we keep cycling
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        //We must manually check if a quit message was posted, since peekmessage does not automatically detect it
        if (msg.message == WM_QUIT)
        {
            return msg.wParam;
        }
        //TranslateMessage will add additional WM_CHAR if necessary
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    //Return empty optional when not quitting the app
    return {};
}

LRESULT Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        //Extract ptr to window class from creation data
        const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        Window const* pWnd = static_cast<Window*>(pCreate->lpCreateParams);
        //Set WinAPI-managed user data to store ptr to window class
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        //Set message proc to normal (non-setup) handler now that the setup is finished
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(pWnd->HandleMsgThunk));
        //Forward message to window class handler
        return pWnd->HandleMsgThunk(hWnd, msg, wParam, lParam);
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (msg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        //Window destruction is handled by the Window class destructor
        return 0;
    case WM_KILLFOCUS:
        //Clear the keyboard status when losing focus to prevent the application getting "stuck" on an input state
        kbd.ClearState();
        break;

    /***************** KEYBOARD MESSAGES **************/
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        //The 30th bit of lParam indicates whether the key was already pressed in the previous message
        if (!(lParam & 0x40000000) || kbd.autorepeatEnabled)
        {
            kbd.OnKeyPress(static_cast<unsigned char>(wParam));
        }
        break;
    case WM_SYSKEYUP:
    case WM_KEYUP:
        kbd.OnKeyRelease(static_cast<unsigned char>(wParam));
        break;
    case WM_CHAR:
        kbd.OnChar(static_cast<unsigned char>(wParam));
        break;
    /************* END KEYBOARD MESSAGES ***************/


    /****************** MOUSE MESSAGES ****************/
    case WM_MOUSEMOVE:
    {
        const POINTS pt = MAKEPOINTS(lParam);
        //In client region -> log move, log enter + capture mose (if not previously in window)
        if (IsClientRegion(pt.x, pt.y))
        {
            mouse.OnMouseMove(pt.x, pt.y);
            if (!mouse.IsInWindow())
            {
                SetCapture(hWnd);
                mouse.OnMouseEnter();
            }
        }
        //Not in client region -> log move/ maintain capture if button down
        else
        {
            if (wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON))
            {
                mouse.OnMouseMove(pt.x, pt.y);
            }
            //Button up -> release capture / log leave event
            else
            {
                ReleaseCapture();
                mouse.OnMouseLeave();
            }
        }
        break;
    }
    case WM_LBUTTONDOWN:
    {
        SetForegroundWindow(hWnd);
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnLeftPressed(pt.x, pt.y);
        break;
    }
    case WM_RBUTTONDOWN:
    {
        SetForegroundWindow(hWnd);
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnRightPressed(pt.x, pt.y);
        break;
    }
    case WM_MBUTTONDOWN:
    {
        SetForegroundWindow(hWnd);
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnMiddlePressed(pt.x, pt.y);
        break;
    }
    case WM_LBUTTONUP:
    {
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnLeftReleased(pt.x, pt.y);
        if (!IsClientRegion(pt.x, pt.y))
        {
            ReleaseCapture();
            mouse.OnMouseLeave();
        }
        break;
    }
    case WM_RBUTTONUP:
    {
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnRightReleased(pt.x, pt.y);
        if (!IsClientRegion(pt.x, pt.y))
        {
            ReleaseCapture();
            mouse.OnMouseLeave();
        }
        break;
    }
    case WM_MBUTTONUP:
    {
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnMiddleReleased(pt.x, pt.y);
        if (!IsClientRegion(pt.x, pt.y))
        {
            ReleaseCapture();
            mouse.OnMouseLeave();
        }
        break;
    }
    case WM_MOUSEWHEEL:
    {
        const POINTS pt = MAKEPOINTS(lParam);
        const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        mouse.OnWheelDelta(pt.x, pt.y, delta);
        break;
    }

    /******* END MOUSE MESSAGES ******/

    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool Window::IsClientRegion(int x, int y)
{
    return x >= 0 && x <= width && y >= 0 && y <= height;
}


//Windows Exception section
Window::Exception::Exception(int line, const char* file, HRESULT hr)
    :
    CustomException(line, file),
    hr(hr)
{}

const char* Window::Exception::what() const noexcept
{
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "[Error Code] " << GetErrorCode() << std::endl
        << "[Description] " << GetErrorString() << std::endl
        << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
    char* pMsgBuf = nullptr;
    DWORD nMsgLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
    );
    if (nMsgLen == 0)
    {
        return "Unidentified error code";
    }
    std::string errorString = pMsgBuf;
    LocalFree(pMsgBuf);
    return errorString;
}

const char* Window::Exception::GetType() const noexcept
{
    return "Custom Window Exception";
}

HRESULT Window::Exception::GetErrorCode() const noexcept
{
    return hr;
}

std::string Window::Exception::GetErrorString() const noexcept
{
    return TranslateErrorCode(hr);
}
