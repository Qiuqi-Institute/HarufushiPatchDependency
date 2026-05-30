#include "engine/ui/UiNode.hpp"

#include <utility>

namespace haru::engine::ui {

UiNode::UiNode(graphics::Rect localBounds, graphics::Color background)
    : localBounds_(localBounds), background_(background) {}

void UiNode::addChild(UiNode child) {
    children_.push_back(std::move(child));
}

void UiNode::render(graphics::RenderQueue& queue) const {
    renderAt(queue, {0, 0});
}

const graphics::Rect& UiNode::localBounds() const {
    return localBounds_;
}

const std::vector<UiNode>& UiNode::children() const {
    return children_;
}

void UiNode::renderAt(graphics::RenderQueue& queue, graphics::Point parentOrigin) const {
    const graphics::Rect absoluteRect{parentOrigin.x + localBounds_.x,
                                      parentOrigin.y + localBounds_.y,
                                      localBounds_.width,
                                      localBounds_.height};
    queue.fillRect(absoluteRect, background_);

    const graphics::Point childOrigin{absoluteRect.x, absoluteRect.y};
    for (const auto& child : children_) {
        child.renderAt(queue, childOrigin);
    }
}

} // namespace haru::engine::ui
