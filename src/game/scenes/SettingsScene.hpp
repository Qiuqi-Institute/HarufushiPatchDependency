#pragma once

#include <HaruFrame>

#include "../localization/GameText.hpp"

#include <optional>

namespace haru::game::scenes {

enum class SettingsAction {
    SetLocaleEnglish,
    SetLocaleSimplifiedChinese,
    SetLocaleJapanese,
    SelectGameTab,
    SelectAudioTab,
    SelectDisplayTab,
    IncreaseMasterVolume,
    DecreaseMasterVolume,
    IncreaseWindowScale,
    DecreaseWindowScale,
    IncreaseTextSpeed,
    DecreaseTextSpeed,
    Back,
};

enum class SettingsTab {
    Game,
    Audio,
    Display,
};

struct SettingsState {
    SettingsTab activeTab = SettingsTab::Game;
    int masterVolume = 80;
    int windowScale = 100;
    int textSpeed = 50;
};

class SettingsScene {
public:
    explicit SettingsScene(localization::GameText text = localization::GameText::loadDefault(),
                           SettingsState state = {});

    void render(engine::graphics::RenderQueue& queue,
                engine::graphics::Size surfaceSize) const;
    std::optional<SettingsAction> actionAt(engine::graphics::Point point,
                                           engine::graphics::Size surfaceSize) const;

private:
    localization::GameText text_;
    SettingsState state_;
};

} // namespace haru::game::scenes
