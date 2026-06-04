#include "engine/graphics/RenderQueue.hpp"

#include <utility>

namespace haru::engine::graphics {

void RenderQueue::clear(Color color) {
    commands_.push_back({DrawCommandKind::Clear, {0, 0, 0, 0}, color, {}});
}

void RenderQueue::fillRect(Rect rect, Color color) {
    commands_.push_back({DrawCommandKind::FillRect, rect, color, {}});
}

void RenderQueue::fillRoundedRect(Rect rect, Color color, int radius) {
    DrawCommand command{DrawCommandKind::FillRoundedRect, rect, color, {}};
    command.radius = radius;
    commands_.push_back(command);
}

void RenderQueue::fillEllipse(Rect rect, Color color) {
    commands_.push_back({DrawCommandKind::FillEllipse, rect, color, {}});
}

void RenderQueue::strokeRect(Rect rect, Color color, int thickness) {
    DrawCommand command{DrawCommandKind::StrokeRect, rect, color, {}};
    command.thickness = thickness;
    commands_.push_back(command);
}

void RenderQueue::fillVerticalGradient(Rect rect, Color topColor, Color bottomColor) {
    DrawCommand command{DrawCommandKind::FillVerticalGradient, rect, topColor, {}};
    command.secondaryColor = bottomColor;
    commands_.push_back(command);
}

void RenderQueue::fillPolygon(std::vector<Point> points, Color color) {
    DrawCommand command{DrawCommandKind::FillPolygon, {0, 0, 0, 0}, color, {}};
    command.points = std::move(points);
    commands_.push_back(std::move(command));
}

void RenderQueue::drawImage(Rect rect, std::string imagePath) {
    commands_.push_back({DrawCommandKind::Image, rect, {255, 255, 255, 255},
                         std::move(imagePath)});
}

void RenderQueue::drawText(Rect rect, std::string text, Color color) {
    commands_.push_back({DrawCommandKind::Text, rect, color, std::move(text)});
}

void RenderQueue::reset() {
    commands_.clear();
}

const std::vector<DrawCommand>& RenderQueue::commands() const {
    return commands_;
}

} // namespace haru::engine::graphics
