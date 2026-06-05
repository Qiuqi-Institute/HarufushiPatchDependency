#pragma once

#include "../graphics/Color.hpp"
#include "../graphics/Geometry.hpp"
#include "../graphics/RenderQueue.hpp"

#include <string>

namespace haru::engine::ui {

struct ButtonStyle {
    graphics::Color background{68, 48, 72, 255};
    graphics::Color text{245, 235, 228, 255};
    int textPadding = 12;
    graphics::TextRole textRole = graphics::TextRole::Default;
};

class Button {
public:
    Button(graphics::Rect bounds, std::string label, ButtonStyle style = {});

    void render(graphics::RenderQueue& queue) const;
    bool contains(graphics::Point point) const;

    const graphics::Rect& bounds() const;
    const std::string& label() const;

private:
    graphics::Rect bounds_;
    std::string label_;
    ButtonStyle style_;
};

} // namespace haru::engine::ui
