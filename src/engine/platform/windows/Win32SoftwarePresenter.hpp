#pragma once

#include "../../graphics/RenderQueue.hpp"
#include "../../graphics/SoftwareSurface.hpp"
#include "Win32Window.hpp"

namespace haru::engine::platform::windows {

class Win32SoftwarePresenter {
public:
    void present(const Win32Window& window, const graphics::SoftwareSurface& surface) const;
    void present(const Win32Window& window,
                 const graphics::SoftwareSurface& surface,
                 const graphics::RenderQueue& textSource) const;
};

} // namespace haru::engine::platform::windows
