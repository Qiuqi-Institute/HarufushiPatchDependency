#pragma once

#include "engine/graphics/SoftwareSurface.hpp"
#include "engine/platform/windows/Win32Window.hpp"

namespace haru::engine::platform::windows {

class Win32SoftwarePresenter {
public:
    void present(const Win32Window& window, const graphics::SoftwareSurface& surface) const;
};

} // namespace haru::engine::platform::windows
