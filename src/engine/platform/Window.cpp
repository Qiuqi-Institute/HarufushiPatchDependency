#include "engine/platform/Window.hpp"

#include <algorithm>
#include <cmath>

namespace haru::engine::platform {

WindowConfig WindowConfig::defaultGameWindow() {
    return {"Harufushi Patch Dependency", 1280, 720};
}

bool WindowConfig::valid() const {
    return !title.empty() && width > 0 && height > 0;
}

WindowBounds WindowSizingPolicy::constrainAspectRatio(WindowBounds proposed,
                                                      ResizeEdge edge,
                                                      int aspectWidth,
                                                      int aspectHeight) {
    if (aspectWidth <= 0 || aspectHeight <= 0) {
        return proposed;
    }

    const int minWidth = 320;
    const int minHeight = 180;
    int width = std::max(proposed.right - proposed.left, minWidth);
    int height = std::max(proposed.bottom - proposed.top, minHeight);

    const auto heightForWidth = [&](int value) {
        return std::max(minHeight,
                        static_cast<int>(std::lround(
                            static_cast<double>(value) * aspectHeight /
                            static_cast<double>(aspectWidth))));
    };
    const auto widthForHeight = [&](int value) {
        return std::max(minWidth,
                        static_cast<int>(std::lround(
                            static_cast<double>(value) * aspectWidth /
                            static_cast<double>(aspectHeight))));
    };

    switch (edge) {
    case ResizeEdge::Left:
    case ResizeEdge::Right:
        height = heightForWidth(width);
        break;
    case ResizeEdge::Top:
    case ResizeEdge::Bottom:
        width = widthForHeight(height);
        break;
    case ResizeEdge::TopLeft:
    case ResizeEdge::TopRight:
    case ResizeEdge::BottomLeft:
    case ResizeEdge::BottomRight:
        if (static_cast<double>(width) / static_cast<double>(height) >
            static_cast<double>(aspectWidth) / static_cast<double>(aspectHeight)) {
            height = heightForWidth(width);
        } else {
            width = widthForHeight(height);
        }
        break;
    }

    switch (edge) {
    case ResizeEdge::Left:
        proposed.left = proposed.right - width;
        proposed.bottom = proposed.top + height;
        break;
    case ResizeEdge::Right:
        proposed.right = proposed.left + width;
        proposed.bottom = proposed.top + height;
        break;
    case ResizeEdge::Top:
        proposed.top = proposed.bottom - height;
        proposed.right = proposed.left + width;
        break;
    case ResizeEdge::Bottom:
        proposed.bottom = proposed.top + height;
        proposed.right = proposed.left + width;
        break;
    case ResizeEdge::TopLeft:
        proposed.left = proposed.right - width;
        proposed.top = proposed.bottom - height;
        break;
    case ResizeEdge::TopRight:
        proposed.right = proposed.left + width;
        proposed.top = proposed.bottom - height;
        break;
    case ResizeEdge::BottomLeft:
        proposed.left = proposed.right - width;
        proposed.bottom = proposed.top + height;
        break;
    case ResizeEdge::BottomRight:
        proposed.right = proposed.left + width;
        proposed.bottom = proposed.top + height;
        break;
    }

    return proposed;
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
