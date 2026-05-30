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

enum class WindowEventKind {
    CloseRequested,
    Resized,
};

struct WindowEvent {
    WindowEventKind kind;
    int width = 0;
    int height = 0;
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
