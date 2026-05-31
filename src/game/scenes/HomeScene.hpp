#pragma once

#include <HaruFrame>

#include <optional>

namespace haru::game::scenes {

enum class HomeAction {
    NewGame,
    OpenSaves,
    OpenSettings,
    Back,
    Quit,
};

enum class HomePanel {
    Main,
    Saves,
    Settings,
};

class HomeScene {
public:
    void render(engine::graphics::RenderQueue& queue,
                engine::graphics::Size surfaceSize,
                HomePanel panel = HomePanel::Main) const;
    std::optional<HomeAction> actionAt(engine::graphics::Point point,
                                       engine::graphics::Size surfaceSize,
                                       HomePanel panel = HomePanel::Main) const;
};

} // namespace haru::game::scenes
