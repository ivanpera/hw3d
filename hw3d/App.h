#pragma once
#include "Window.h"
#include "Timer.h"

class App 
{
public:
    App();
    //First frame / game loop start
    int Go();
private:
    void DoFrame();
private:
    Window wnd;
    Timer timer;
};