#include "engine/platform/Window.hpp"

namespace haru::engine::platform {

WindowConfig WindowConfig::defaultGameWindow() {
    return {"Harufushi Patch Dependency", 1280, 720};
}

bool WindowConfig::valid() const {
    return !title.empty() && width > 0 && height > 0;
}

WindowEvent WindowEvent::closeRequested() {
    return {WindowEventKind::CloseRequested};
}

WindowEvent WindowEvent::resized(int width, int height) {
    WindowEvent event{WindowEventKind::Resized};
    event.width = width;
    event.height = height;
    return event;
}

WindowEvent WindowEvent::mouseButtonReleased(int x, int y, MouseButton button) {
    WindowEvent event{WindowEventKind::MouseButtonReleased};
    event.x = x;
    event.y = y;
    event.button = button;
    return event;
}

} // namespace haru::engine::platform
