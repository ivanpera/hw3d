#include "Mouse.h"
#include "CustomWin.h"

std::pair<int, int> Mouse::GetPos() const noexcept
{
    return { x,y };
}

int Mouse::GetPosX() const noexcept
{
    return x;
}

int Mouse::GetPosY() const noexcept
{
    return y;
}

bool Mouse::IsInWindow() const noexcept
{
    return isInWindow;
}

bool Mouse::LeftIsPressed() const noexcept
{
    return leftIsPressed;
}

bool Mouse::RightIsPressed() const noexcept
{
    return rightIsPressed;
}

bool Mouse::MiddleIsPressed() const noexcept
{
    return middleIsPressed;
}

std::optional<Mouse::Event> Mouse::Read() noexcept
{
    if (buffer.size() > 0u)
    {
        Event e = buffer.front();
        buffer.pop();
        return e;
    }
    return {};
}

bool Mouse::IsEmpty() const noexcept
{
    return buffer.empty();
}

void Mouse::Flush() noexcept
{
    buffer = std::queue<Event>();
}

void Mouse::OnMouseMove(int newx, int newy) noexcept
{
    x = newx;
    y = newy;

    buffer.push(Event(Event::Type::Move, *this));
    TrimBuffer();
}

void Mouse::OnMouseLeave() noexcept
{
    isInWindow = false;
    buffer.push(Event(Event::Type::Leave, *this));
    TrimBuffer();
}

void Mouse::OnMouseEnter() noexcept
{
    isInWindow = true;
    buffer.push(Event(Event::Type::Enter, *this));
    TrimBuffer();
}

void Mouse::OnLeftPressed(int x, int y) noexcept
{
    leftIsPressed = true;
    buffer.push(Event(Event::Type::Press, *this));
    TrimBuffer();
}

void Mouse::OnRightPressed(int x, int y) noexcept
{
    rightIsPressed = true;
    buffer.push(Event(Event::Type::Press, *this));
    TrimBuffer();
}

void Mouse::OnMiddlePressed(int x, int y) noexcept
{
    middleIsPressed = true;
    buffer.push(Event(Event::Type::Press, *this));
    TrimBuffer();
}

void Mouse::OnLeftReleased(int x, int y) noexcept
{
    leftIsPressed = false;
    buffer.push(Event(Event::Type::Release, *this));
    TrimBuffer();
}

void Mouse::OnRightReleased(int x, int y) noexcept
{
    rightIsPressed = false;
    buffer.push(Event(Event::Type::Release, *this));
    TrimBuffer();
}

void Mouse::OnMiddleReleased(int x, int y) noexcept
{
    middleIsPressed = false;
    buffer.push(Event(Event::Type::Release, *this));
    TrimBuffer();
}

void Mouse::OnWheelUp(int x, int y) noexcept
{
    buffer.push(Event(Event::Type::WheelUp, *this));
    TrimBuffer();
}

void Mouse::OnWheelDown(int x, int y) noexcept
{
    buffer.push(Event(Event::Type::WheelDown, *this));
    TrimBuffer();
}

void Mouse::TrimBuffer() noexcept
{
    while (buffer.size() > bufferSize)
    {
        buffer.pop();
    }
}

void Mouse::OnWheelDelta(int newx, int newy, int delta) noexcept
{
    wheelDeltaCarry += delta;
    if (wheelDeltaCarry >= WHEEL_DELTA)
    {
        wheelDeltaCarry -= WHEEL_DELTA;
        OnWheelUp(x, y);
    }
    if (wheelDeltaCarry <= -WHEEL_DELTA)
    {
        wheelDeltaCarry += WHEEL_DELTA;
        OnWheelDown(x, y);
    }
}
