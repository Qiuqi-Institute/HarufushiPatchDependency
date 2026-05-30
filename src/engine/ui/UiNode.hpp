#pragma once

#include "../graphics/Color.hpp"
#include "../graphics/Geometry.hpp"
#include "../graphics/RenderQueue.hpp"

#include <optional>
#include <string>
#include <vector>

namespace haru::engine::ui {

class UiNode {
public:
    UiNode(graphics::Rect localBounds, graphics::Color background);

    void addChild(UiNode child);
    void setText(std::string text, graphics::Color color);
    void render(graphics::RenderQueue& queue) const;

    const graphics::Rect& localBounds() const;
    const std::vector<UiNode>& children() const;

private:
    void renderAt(graphics::RenderQueue& queue, graphics::Point parentOrigin) const;

    graphics::Rect localBounds_;
    graphics::Color background_;
    std::optional<std::string> text_;
    graphics::Color textColor_{255, 255, 255, 255};
    std::vector<UiNode> children_;
};

} // namespace haru::engine::ui
