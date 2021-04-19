#pragma once
#include <queue>
#include <optional>

class Mouse
{
    friend class Window;
public:
    class Event
    {
    public:
        enum class Type
        {
            Press,
            Release,
            WheelUp,
            WheelDown,
            Move,
            Leave,
            Enter
        };
    private:
        Type type;
        bool leftIsPressed;
        bool rightIsPressed;
        bool middleIsPressed;
        int x;
        int y;
    public:
        Event(Type type, const Mouse& parent) noexcept
            :
            type(type),
            leftIsPressed(parent.leftIsPressed),
            rightIsPressed(parent.rightIsPressed),
            middleIsPressed(parent.middleIsPressed),
            x(parent.x),
            y(parent.y)
        {}
        Type GetType() const noexcept
        {
            return type;
        }
        bool LeftIsPressed() const noexcept
        {
            return leftIsPressed;
        }
        bool RightIsPressed() const noexcept
        {
            return rightIsPressed;
        }
        bool MiddleIsPressed() const noexcept
        {
            return middleIsPressed;
        }
        std::pair<int, int> GetPos() const noexcept
        {
            return { x,y };
        }
        int GetX() const noexcept
        {
            return x;
        }
        int GetY() const noexcept
        {
            return y;
        }
    };
public:
    Mouse() = default;
    Mouse(const Mouse&) = delete;
    Mouse& operator=(Mouse& other) = delete;
    std::pair<int, int> GetPos() const noexcept;
    int GetPosX() const noexcept;
    int GetPosY() const noexcept;
    bool IsInWindow() const noexcept;
    bool LeftIsPressed() const noexcept;
    bool RightIsPressed() const noexcept;
    bool MiddleIsPressed() const noexcept;
    std::optional<Mouse::Event> Read() noexcept;
    bool IsEmpty() const noexcept;
    void Flush() noexcept;
private:
    void OnMouseMove(int x, int y) noexcept;
    void OnMouseLeave() noexcept;
    void OnMouseEnter() noexcept;
    void OnLeftPressed(int x, int y) noexcept;
    void OnRightPressed(int x, int y) noexcept;
    void OnMiddlePressed(int x, int y) noexcept;
    void OnLeftReleased(int x, int y) noexcept;
    void OnRightReleased(int x, int y) noexcept;
    void OnMiddleReleased(int x, int y) noexcept;
    void OnWheelUp(int x, int y) noexcept;
    void OnWheelDown(int x, int y) noexcept;
    void TrimBuffer() noexcept;
    void OnWheelDelta(int x, int y, int delta) noexcept;
private:
    static constexpr unsigned int bufferSize = 16u;
    int x;
    int y;
    bool leftIsPressed = false;
    bool rightIsPressed = false;
    bool middleIsPressed = false;
    bool isInWindow = false;
    int wheelDeltaCarry = 0;
    std::queue<Event> buffer;
};