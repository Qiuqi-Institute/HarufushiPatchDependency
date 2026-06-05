#pragma once

#include <string>
#include <vector>

namespace haru::engine::platform {

struct WindowConfig {
    std::string title;
    int width;
    int height;

    static WindowConfig defaultGameWindow();

    bool valid() const;
};

struct WindowBounds {
    int left;
    int top;
    int right;
    int bottom;
};

enum class ResizeEdge {
    Left,
    Right,
    Top,
    Bottom,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
};

class WindowSizingPolicy {
public:
    static WindowBounds constrainAspectRatio(WindowBounds proposed,
                                             ResizeEdge edge,
                                             int aspectWidth,
                                             int aspectHeight);
};

enum class WindowEventKind {
    CloseRequested,
    Resized,
    MouseButtonReleased,
};

enum class MouseButton {
    None,
    Left,
    Right,
    Middle,
};

struct WindowEvent {
    WindowEventKind kind;
    int x = 0;
    int y = 0;
    MouseButton button = MouseButton::None;
    int width = 0;
    int height = 0;

    static WindowEvent closeRequested();
    static WindowEvent resized(int width, int height);
    static WindowEvent mouseButtonReleased(int x, int y, MouseButton button);
};

class Window {
public:
    virtual ~Window() = default;

    virtual void show() = 0;
    virtual std::vector<WindowEvent> pollEvents() = 0;
    virtual void requestClose() = 0;
    virtual bool shouldClose() const = 0;
};

} // namespace haru::engine::platform
