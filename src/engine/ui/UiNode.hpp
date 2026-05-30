#pragma once

#include "engine/graphics/Color.hpp"
#include "engine/graphics/Geometry.hpp"
#include "engine/graphics/RenderQueue.hpp"

#include <vector>

namespace haru::engine::ui {

class UiNode {
public:
    UiNode(graphics::Rect localBounds, graphics::Color background);

    void addChild(UiNode child);
    void render(graphics::RenderQueue& queue) const;

    const graphics::Rect& localBounds() const;
    const std::vector<UiNode>& children() const;

private:
    void renderAt(graphics::RenderQueue& queue, graphics::Point parentOrigin) const;

    graphics::Rect localBounds_;
    graphics::Color background_;
    std::vector<UiNode> children_;
};

} // namespace haru::engine::ui
