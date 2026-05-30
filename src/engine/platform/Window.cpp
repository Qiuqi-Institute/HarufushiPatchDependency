#include "engine/platform/Window.hpp"

namespace haru::engine::platform {

WindowConfig WindowConfig::defaultGameWindow() {
    return {"春伏补丁依存症", 1280, 720};
}

bool WindowConfig::valid() const {
    return !title.empty() && width > 0 && height > 0;
}

} // namespace haru::engine::platform
