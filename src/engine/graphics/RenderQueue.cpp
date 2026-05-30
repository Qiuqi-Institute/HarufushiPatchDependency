#include "engine/graphics/RenderQueue.hpp"

#include <utility>

namespace haru::engine::graphics {

void RenderQueue::clear(Color color) {
    commands_.push_back({DrawCommandKind::Clear, {0, 0, 0, 0}, color, {}});
}

void RenderQueue::fillRect(Rect rect, Color color) {
    commands_.push_back({DrawCommandKind::FillRect, rect, color, {}});
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
