#pragma once

#include <HaruFrame>

#include "../localization/GameText.hpp"

#include <optional>
#include <string>
#include <vector>

namespace haru::game::scenes {

enum class HomeAction {
    NewGame,
    OpenSaves,
    OpenSettings,
    LoadSave0,
    LoadSave1,
    LoadSave2,
    LoadSave3,
    Back,
    Quit,
};

enum class HomePanel {
    Main,
    Saves,
};

class HomeScene {
public:
    explicit HomeScene(localization::GameText text = localization::GameText::loadDefault());

    void render(engine::graphics::RenderQueue& queue,
                engine::graphics::Size surfaceSize,
                HomePanel panel = HomePanel::Main,
                const std::vector<std::string>& saveSummaries = {}) const;
    std::optional<HomeAction> actionAt(engine::graphics::Point point,
                                       engine::graphics::Size surfaceSize,
                                       HomePanel panel = HomePanel::Main,
                                       std::size_t saveCount = 0) const;

private:
    localization::GameText text_;
};

} // namespace haru::game::scenes
