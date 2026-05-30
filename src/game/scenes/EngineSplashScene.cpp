#include "game/scenes/EngineSplashScene.hpp"

#include <algorithm>

namespace haru::game::scenes {

EngineSplashScene::EngineSplashScene(double durationSeconds)
    : durationSeconds_(durationSeconds > 0.0 ? durationSeconds : 0.01) {}

void EngineSplashScene::update(double deltaSeconds) {
    elapsedSeconds_ += std::max(deltaSeconds, 0.0);
}

void EngineSplashScene::render(engine::graphics::RenderQueue& queue,
                               engine::graphics::Size surfaceSize) const {
    queue.clear({8, 10, 14, 255});

    const int centerX = surfaceSize.width / 2;
    const int centerY = surfaceSize.height / 2;
    const int markSize = 132;
    const int markX = centerX - (markSize / 2);
    const int markY = centerY - 104;

    queue.fillRect({markX, markY, markSize, markSize}, {42, 47, 58, 255});
    queue.fillRect({markX + 14, markY + 14, markSize - 28, markSize - 28}, {16, 18, 24, 255});
    queue.fillRect({markX + 34, markY + 34, markSize - 68, 18}, {206, 86, 132, 255});
    queue.fillRect({markX + 34, markY + 74, markSize - 68, 18}, {88, 116, 156, 255});

    const int barWidth = 360;
    const int barHeight = 8;
    const int progressWidth = static_cast<int>(barWidth * progress());
    queue.fillRect({centerX - (barWidth / 2), centerY + 76, barWidth, barHeight},
                   {34, 38, 48, 255});
    queue.fillRect({centerX - (barWidth / 2), centerY + 76, progressWidth, barHeight},
                   {206, 86, 132, 255});

    queue.drawText({centerX - 168, centerY + 104, 336, 40},
                   "Harufushi Frame",
                   {245, 238, 232, 255});
}

bool EngineSplashScene::active() const {
    return !complete();
}

bool EngineSplashScene::complete() const {
    return elapsedSeconds_ >= durationSeconds_;
}

double EngineSplashScene::progress() const {
    return std::min(elapsedSeconds_ / durationSeconds_, 1.0);
}

} // namespace haru::game::scenes
