#include "engine/ui/Button.hpp"

#include <algorithm>
#include <utility>

namespace haru::engine::ui {

Button::Button(graphics::Rect bounds, std::string label, ButtonStyle style)
    : bounds_(bounds), label_(std::move(label)), style_(style) {}

void Button::render(graphics::RenderQueue& queue) const {
    queue.fillRect(bounds_, style_.background);
    queue.drawText({bounds_.x + style_.textPadding,
                    bounds_.y + style_.textPadding,
                    std::max(bounds_.width - (style_.textPadding * 2), 0),
                    std::max(bounds_.height - (style_.textPadding * 2), 0)},
                   label_, style_.text);
}

bool Button::contains(graphics::Point point) const {
    return point.x >= bounds_.x && point.y >= bounds_.y &&
           point.x < bounds_.x + bounds_.width &&
           point.y < bounds_.y + bounds_.height;
}

const graphics::Rect& Button::bounds() const {
    return bounds_;
}

const std::string& Button::label() const {
    return label_;
}

} // namespace haru::engine::ui
