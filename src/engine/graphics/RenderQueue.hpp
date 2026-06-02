#pragma once

#include "Color.hpp"
#include "Geometry.hpp"

#include <string>
#include <vector>

namespace haru::engine::graphics {

enum class DrawCommandKind {
    Clear,
    FillRect,
    FillRoundedRect,
    FillEllipse,
    StrokeRect,
    FillVerticalGradient,
    Text,
};

struct DrawCommand {
    DrawCommandKind kind;
    Rect rect;
    Color color;
    std::string text;
    Color secondaryColor{0, 0, 0, 0};
    int radius = 0;
    int thickness = 0;
};

class RenderQueue {
public:
    void clear(Color color);
    void fillRect(Rect rect, Color color);
    void fillRoundedRect(Rect rect, Color color, int radius);
    void fillEllipse(Rect rect, Color color);
    void strokeRect(Rect rect, Color color, int thickness);
    void fillVerticalGradient(Rect rect, Color topColor, Color bottomColor);
    void drawText(Rect rect, std::string text, Color color);
    void reset();

    const std::vector<DrawCommand>& commands() const;

private:
    std::vector<DrawCommand> commands_;
};

} // namespace haru::engine::graphics
