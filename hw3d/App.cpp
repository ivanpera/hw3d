#include "App.h"
#include <sstream>
#include <iomanip>

App::App()
    :
    wnd(Window(800,600,"My Window"))
{}

int App::Go()
{
    while (true)
    {
        //Exit the loop only if Process messages returns a non empty optional
        //The only occasion in which this happens is if the function detects a WM_QUIT message
        if (const auto ecode = Window::ProcessMessages())
        {
            return *ecode;
        }
        DoFrame();
    }
}

void App::DoFrame()
{
    const float t = timer.Peek();
    std::ostringstream oss;
    oss << "Time elapsed: " << std::setprecision(2) << t << "s";
    wnd.SetTitle(oss.str());
}
