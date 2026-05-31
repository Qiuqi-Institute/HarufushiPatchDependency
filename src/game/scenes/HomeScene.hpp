#pragma once

#include <HaruFrame>

#include "../localization/GameText.hpp"

#include <optional>

namespace haru::game::scenes {

enum class HomeAction {
    NewGame,
    OpenSaves,
    OpenSettings,
    SetLocaleEnglish,
    SetLocaleSimplifiedChinese,
    SetLocaleJapanese,
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
    explicit HomeScene(localization::GameText text = localization::GameText::loadDefault());

    void render(engine::graphics::RenderQueue& queue,
                engine::graphics::Size surfaceSize,
                HomePanel panel = HomePanel::Main) const;
    std::optional<HomeAction> actionAt(engine::graphics::Point point,
                                       engine::graphics::Size surfaceSize,
                                       HomePanel panel = HomePanel::Main) const;

private:
    localization::GameText text_;
};

} // namespace haru::game::scenes
