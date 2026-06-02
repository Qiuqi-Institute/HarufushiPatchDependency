#pragma once

#include "../../core/EngineOpeningGate.hpp"
#include "../../graphics/RenderQueue.hpp"
#include "../../graphics/SoftwareSurface.hpp"
#include "Win32Window.hpp"

namespace haru::engine::platform::windows {

class Win32SoftwarePresenter {
public:
    explicit Win32SoftwarePresenter(double openingSeconds = 2.0);

    void present(const Win32Window& window, const graphics::SoftwareSurface& surface) const;
    void present(const Win32Window& window,
                 const graphics::SoftwareSurface& surface,
                 const graphics::RenderQueue& textSource) const;

    bool engineOpeningActive() const;

private:
    void presentWithEngineGate(const Win32Window& window,
                               const graphics::SoftwareSurface& surface,
                               const graphics::RenderQueue* textSource) const;

    mutable core::EngineOpeningGate openingGate_;
};

} // namespace haru::engine::platform::windows
