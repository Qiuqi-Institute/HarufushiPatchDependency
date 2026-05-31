#pragma once

#include <HaruFrame>

#include "../localization/GameText.hpp"
#include "../systems/DailyLoopState.hpp"

#include <optional>

namespace haru::game::scenes {

class TitleScene {
public:
    explicit TitleScene(localization::GameText text = localization::GameText::loadDefault());

    void render(engine::graphics::RenderQueue& queue,
                engine::graphics::Size surfaceSize,
                const systems::DailyStats& stats = systems::DailyStats{}) const;
    std::optional<systems::DailyAction> actionAt(engine::graphics::Point point,
                                                 engine::graphics::Size surfaceSize) const;

private:
    localization::GameText text_;
};

} // namespace haru::game::scenes
