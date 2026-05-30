#pragma once

#include <HaruFrame>

#include "../systems/DailyLoopState.hpp"

#include <optional>

namespace haru::game::scenes {

class TitleScene {
public:
    void render(engine::graphics::RenderQueue& queue,
                engine::graphics::Size surfaceSize,
                const systems::DailyStats& stats = systems::DailyStats{}) const;
    std::optional<systems::DailyAction> actionAt(engine::graphics::Point point,
                                                 engine::graphics::Size surfaceSize) const;
};

} // namespace haru::game::scenes
