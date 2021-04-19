#pragma once
#include "CustomWin.h"
#include "CustomException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include <optional>

class Window
{
public:
    class Exception : CustomException
    {
    public:
        Exception(int line, const char* file, HRESULT hr);
        const char* what() const noexcept override;
        virtual const char* GetType() const noexcept override;
        static std::string TranslateErrorCode(HRESULT hr) noexcept;
        HRESULT GetErrorCode() const noexcept;
        std::string GetErrorString() const noexcept;
    private:
        HRESULT hr;
    };
private:
    //Singleton that manages registration/cleanup of window class
    class WindowClass
    {
    public:
        static const char* GetName() noexcept;
        static HINSTANCE GetInstance() noexcept;
    private:
        WindowClass() noexcept;
        ~WindowClass();
        WindowClass(const WindowClass&) = delete;
        WindowClass& operator=(const WindowClass&) = delete;
        //This application uses only one window class, as such the following varaibles are singletons
        static constexpr const char* wndClassname = "Direct3D Engine Window";
        static WindowClass wndClass;
        HINSTANCE hInst;
    };
public:
    Window(int width, int height, const char* name);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    void SetTitle(const std::string title) noexcept;
    static std::optional<int> ProcessMessages();
private:
    //The following two functions are declared as static since Win32 Api doesn't have the concept of member functions
    //an additional parameter (the this pointer) would be passed to the api
    static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    bool IsClientRegion(int x, int y);
public:
    Keyboard kbd;
    Mouse mouse;
private:
    int width;
    int height;
    HWND hWnd;
};

//Error exception helper macro
#define WND_LAST_EXCEPT() Window::Exception(__LINE__, __FILE__, GetLastError())
#define WND_EXCEPT(hr) Window::Exception(__LINE__,__FILE__,hr)