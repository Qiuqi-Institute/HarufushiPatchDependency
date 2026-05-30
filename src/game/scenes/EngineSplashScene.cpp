#include "EngineSplashScene.hpp"

#include <algorithm>

namespace haru::game::scenes {

EngineSplashScene::EngineSplashScene(double durationSeconds)
    : durationSeconds_(durationSeconds > 0.0 ? durationSeconds : 0.01) {}

void EngineSplashScene::update(double deltaSeconds) {
    elapsedSeconds_ += std::max(deltaSeconds, 0.0);
}

void EngineSplashScene::render(engine::graphics::RenderQueue& queue,
                               engine::graphics::Size surfaceSize) const {
    const engine::graphics::Color white{255, 255, 255, 255};
    const engine::graphics::Color splashTeal{11, 119, 155, 255};
    queue.clear(white);

    const int centerX = surfaceSize.width / 2;
    const int centerY = surfaceSize.height / 2;
    const int markSize = 184;
    const int markX = centerX - (markSize / 2);
    const int markY = centerY - 150;

    // Patchwork crest: frame, twin ears, circuit teeth, and an H-shaped core.
    queue.fillRect({markX + 24, markY + 34, markSize - 48, 14}, splashTeal);
    queue.fillRect({markX + 24, markY + markSize - 48, markSize - 48, 14}, splashTeal);
    queue.fillRect({markX + 24, markY + 48, 14, markSize - 96}, splashTeal);
    queue.fillRect({markX + markSize - 38, markY + 48, 14, markSize - 96}, splashTeal);

    queue.fillRect({markX + 42, markY + 14, 18, 34}, splashTeal);
    queue.fillRect({markX + 60, markY + 28, 18, 20}, splashTeal);
    queue.fillRect({markX + markSize - 78, markY + 28, 18, 20}, splashTeal);
    queue.fillRect({markX + markSize - 60, markY + 14, 18, 34}, splashTeal);

    queue.fillRect({markX + 58, markY + 72, 20, 74}, splashTeal);
    queue.fillRect({markX + markSize - 78, markY + 72, 20, 74}, splashTeal);
    queue.fillRect({markX + 78, markY + 100, markSize - 156, 18}, splashTeal);

    queue.fillRect({markX + 88, markY + 66, 8, 34}, splashTeal);
    queue.fillRect({markX + markSize - 96, markY + 118, 8, 34}, splashTeal);
    queue.fillRect({markX + 96, markY + 66, 28, 8}, splashTeal);
    queue.fillRect({markX + markSize - 124, markY + 144, 28, 8}, splashTeal);

    queue.fillRect({markX + 42, markY + markSize - 26, 18, 18}, splashTeal);
    queue.fillRect({markX + 78, markY + markSize - 26, 18, 18}, splashTeal);
    queue.fillRect({markX + markSize - 96, markY + markSize - 26, 18, 18}, splashTeal);
    queue.fillRect({markX + markSize - 60, markY + markSize - 26, 18, 18}, splashTeal);

    queue.fillRect({markX + 22, markY + 76, 20, 8}, splashTeal);
    queue.fillRect({markX + 10, markY + 84, 12, 12}, splashTeal);
    queue.fillRect({markX + markSize - 42, markY + 76, 20, 8}, splashTeal);
    queue.fillRect({markX + markSize - 22, markY + 84, 12, 12}, splashTeal);
    queue.fillRect({markX + 86, markY + markSize - 54, 12, 12}, splashTeal);
    queue.fillRect({markX + markSize - 98, markY + markSize - 54, 12, 12}, splashTeal);

    queue.drawText({centerX - 260, centerY + 76, 520, 68}, "Harufushi Frame", splashTeal);
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
