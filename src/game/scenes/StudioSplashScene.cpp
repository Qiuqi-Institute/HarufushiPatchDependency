#include "StudioSplashScene.hpp"

#include <algorithm>
#include <cmath>

namespace haru::game::scenes {

namespace {

int scaledOffset(int value, double progress) {
    return static_cast<int>(std::round(static_cast<double>(value) * progress));
}

} // namespace

StudioSplashScene::StudioSplashScene(double durationSeconds)
    : durationSeconds_(durationSeconds > 0.0 ? durationSeconds : 0.01) {}

void StudioSplashScene::update(double deltaSeconds) {
    elapsedSeconds_ += std::max(deltaSeconds, 0.0);
}

bool StudioSplashScene::active() const {
    return elapsedSeconds_ < durationSeconds_;
}

double StudioSplashScene::progress() const {
    return std::clamp(elapsedSeconds_ / durationSeconds_, 0.0, 1.0);
}

void StudioSplashScene::render(engine::graphics::RenderQueue& queue,
                               engine::graphics::Size surfaceSize) const {
    const auto dark = engine::graphics::Color{7, 9, 24, 255};
    const auto cyan = engine::graphics::Color{62, 220, 255, 255};
    const auto blue = engine::graphics::Color{44, 118, 244, 255};
    const auto violet = engine::graphics::Color{132, 82, 255, 255};
    const auto white = engine::graphics::Color{235, 246, 255, 255};
    const auto shadow = engine::graphics::Color{16, 20, 42, 255};
    const int width = std::max(surfaceSize.width, 1);
    const int height = std::max(surfaceSize.height, 1);
    const int centerX = width / 2;
    const int centerY = height / 2;
    const double eased = std::sin(progress() * 3.14159265358979323846 * 0.5);
    const int lift = scaledOffset(34, eased);

    queue.clear(dark);

    queue.fillRect({centerX - 176, centerY + 168, 352, 4}, cyan);
    queue.fillRect({centerX - 130, centerY + 184, 260, 3}, violet);
    queue.fillRect({centerX - 82, centerY + 198, 164, 2}, blue);

    for (int index = 0; index < 7; ++index) {
        const int beamWidth = 44 + (index * 24);
        const int beamY = centerY + 148 - (index * 28);
        const auto beamColor = index % 2 == 0 ? blue : violet;
        queue.fillRect({centerX - (beamWidth / 2), beamY, beamWidth, 3}, beamColor);
    }

    const int pigX = centerX - 118;
    const int pigY = centerY - 104 + (34 - lift);

    queue.fillRect({pigX + 18, pigY + 14, 36, 44}, violet);
    queue.fillRect({pigX + 182, pigY + 14, 36, 44}, violet);
    queue.fillRect({pigX + 42, pigY, 152, 28}, blue);
    queue.fillRect({pigX + 18, pigY + 28, 200, 126}, violet);
    queue.fillRect({pigX + 36, pigY + 44, 164, 94}, shadow);
    queue.fillRect({pigX + 48, pigY + 56, 140, 70}, blue);
    queue.fillRect({pigX + 66, pigY + 76, 28, 24}, dark);
    queue.fillRect({pigX + 142, pigY + 76, 28, 24}, dark);
    queue.fillRect({pigX + 73, pigY + 83, 14, 10}, cyan);
    queue.fillRect({pigX + 149, pigY + 83, 14, 10}, cyan);
    queue.fillRect({pigX + 88, pigY + 108, 60, 34}, cyan);
    queue.fillRect({pigX + 100, pigY + 119, 12, 12}, dark);
    queue.fillRect({pigX + 124, pigY + 119, 12, 12}, dark);
    queue.fillRect({pigX + 88, pigY + 144, 60, 6}, violet);

    for (int index = 0; index < 6; ++index) {
        queue.fillRect({pigX + 34, pigY + 48 + (index * 16), 168, 2}, cyan);
    }

    queue.fillRect({pigX + 2, pigY + 68, 18, 4}, cyan);
    queue.fillRect({pigX + 216, pigY + 68, 18, 4}, cyan);
    queue.fillRect({pigX + 8, pigY + 88, 10, 4}, violet);
    queue.fillRect({pigX + 218, pigY + 88, 10, 4}, violet);

    queue.drawText({centerX - 180, centerY + 226, 360, 30}, "Qiuqi Institute", white);
    queue.drawText({centerX - 66, centerY + 260, 132, 24}, "presents", cyan);
}

} // namespace haru::game::scenes
