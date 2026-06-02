#pragma once

#include <HaruFrame>

#include "../localization/GameText.hpp"

#include <optional>

namespace haru::game::scenes {

enum class SettingsAction {
    SetLocaleEnglish,
    SetLocaleSimplifiedChinese,
    SetLocaleJapanese,
    Back,
};

class SettingsScene {
public:
    explicit SettingsScene(localization::GameText text = localization::GameText::loadDefault());

    void render(engine::graphics::RenderQueue& queue,
                engine::graphics::Size surfaceSize) const;
    std::optional<SettingsAction> actionAt(engine::graphics::Point point,
                                           engine::graphics::Size surfaceSize) const;

private:
    localization::GameText text_;
};

} // namespace haru::game::scenes
