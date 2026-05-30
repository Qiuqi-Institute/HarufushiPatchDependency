#include "engine/core/HaruFrame.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace haru::engine {

namespace {

int openingTitleLetterWidth(char letter) {
    switch (letter) {
    case 'i':
    case 'l':
    case 'I':
        return 24;
    case 'm':
    case 'M':
    case 'w':
    case 'W':
        return 52;
    case ' ':
        return 18;
    default:
        return 36;
    }
}

} // namespace

HaruFrame::HaruFrame(double openingSeconds)
    : openingSeconds_(openingSeconds > 0.0 ? openingSeconds : 0.01) {}

bool HaruFrame::render(graphics::RenderQueue& queue,
                       graphics::Size surfaceSize,
                       double deltaSeconds,
                       const VisualContentRenderer& renderVisualContent) {
    if (openingActive()) {
        updateOpening(deltaSeconds);
        renderOpening(queue, surfaceSize);
        return true;
    }

    renderVisualContent(queue);
    return false;
}

bool HaruFrame::openingActive() const {
    return openingElapsedSeconds_ < openingSeconds_;
}

double HaruFrame::openingProgress() const {
    return std::min(openingElapsedSeconds_ / openingSeconds_, 1.0);
}

void HaruFrame::updateOpening(double deltaSeconds) {
    openingElapsedSeconds_ += std::max(deltaSeconds, 0.0);
}

void HaruFrame::renderOpening(graphics::RenderQueue& queue, graphics::Size surfaceSize) const {
    const graphics::Color white{255, 255, 255, 255};
    const graphics::Color splashTeal{11, 119, 155, 255};
    const double beat = openingElapsedSeconds_ * 8.0;
    const int armSwing = static_cast<int>(std::round(std::sin(beat) * 10.0));
    const int legSwing = static_cast<int>(std::round(std::sin(beat + 1.2) * 8.0));
    queue.clear(white);

    const int centerX = surfaceSize.width / 2;
    const int centerY = surfaceSize.height / 2;
    const int markSize = 184;
    const int markX = centerX - (markSize / 2);
    const int markY = centerY - 150;

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

    queue.fillRect({markX + 22, markY + 76 + armSwing, 20, 8}, splashTeal);
    queue.fillRect({markX + 10, markY + 84 + armSwing, 12, 12}, splashTeal);
    queue.fillRect({markX + markSize - 42, markY + 76 - armSwing, 20, 8}, splashTeal);
    queue.fillRect({markX + markSize - 22, markY + 84 - armSwing, 12, 12}, splashTeal);
    queue.fillRect({markX + 86 - legSwing, markY + markSize - 54, 12, 12}, splashTeal);
    queue.fillRect({markX + markSize - 98 + legSwing, markY + markSize - 54, 12, 12}, splashTeal);

    const std::string title = "Harufushi Frame";
    const int letterGap = 4;
    int titleWidth = 0;
    for (const char letter : title) {
        titleWidth += openingTitleLetterWidth(letter) + letterGap;
    }
    if (!title.empty()) {
        titleWidth -= letterGap;
    }

    int letterX = centerX - (titleWidth / 2);
    for (std::size_t index = 0; index < title.size(); ++index) {
        const char letter = title[index];
        const int letterWidth = openingTitleLetterWidth(letter);
        const int advance = letterWidth + letterGap;
        const double delaySeconds = static_cast<double>(index) * 0.055;
        const double entrance =
            std::clamp((openingElapsedSeconds_ - delaySeconds) / 0.24, 0.0, 1.0);

        if (letter != ' ' && entrance > 0.0) {
            const int dropOffset =
                static_cast<int>(std::round((1.0 - entrance) * 34.0));
            const int bounceOffset =
                static_cast<int>(std::round(std::sin(entrance * 3.14159265358979323846) *
                                            18.0));
            const int idleBounce =
                static_cast<int>(std::round(std::abs(std::sin(beat + (index * 0.55))) * 6.0));
            queue.drawText({letterX, centerY + 76 + dropOffset - bounceOffset - idleBounce,
                            letterWidth, 68},
                           std::string(1, letter),
                           splashTeal);
        }

        letterX += advance;
    }
}

} // namespace haru::engine
