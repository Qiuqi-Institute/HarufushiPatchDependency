#pragma once

#include "../graphics/Color.hpp"
#include "../graphics/Geometry.hpp"
#include "../graphics/RenderQueue.hpp"

#include <string>
#include <string_view>

namespace haru::engine::ui {

struct TextBoxStyle {
    graphics::Color text{245, 235, 228, 255};
    graphics::TextRole textRole = graphics::TextRole::Default;
    int minHorizontalPadding = 12;
    int maxHorizontalPadding = 56;
    int minWidth = 24;
};

class TextBox {
public:
    TextBox(graphics::Rect bounds, std::string text, TextBoxStyle style = {});

    void render(graphics::RenderQueue& queue) const;
    graphics::Rect textRect() const;

    static int estimateTextWidth(std::string_view text);

private:
    graphics::Rect bounds_;
    std::string text_;
    TextBoxStyle style_;
};

} // namespace haru::engine::ui
