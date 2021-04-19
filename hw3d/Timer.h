#pragma once
#include <chrono>

class Timer
{
public:
    Timer();
    //Gets the time elapsed since the last time and resets the timer
    float Mark();
    //Gets the time elapsed since the last time without resetting the timer
    float Peek() const;
private:
    std::chrono::steady_clock::time_point last;
};