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
    FillPolygon,
    Image,
    Text,
};

enum class TextRole {
    Default,
    ZenMaruBlack,
    ZenMaruBold,
};

struct DrawCommand {
    DrawCommandKind kind;
    Rect rect;
    Color color;
    std::string text;
    Color secondaryColor{0, 0, 0, 0};
    int radius = 0;
    int thickness = 0;
    std::vector<Point> points;
    TextRole textRole = TextRole::Default;
    int fontScalePercent = 100;
};

class RenderQueue {
public:
    void clear(Color color);
    void fillRect(Rect rect, Color color);
    void fillRoundedRect(Rect rect, Color color, int radius);
    void fillEllipse(Rect rect, Color color);
    void strokeRect(Rect rect, Color color, int thickness);
    void fillVerticalGradient(Rect rect, Color topColor, Color bottomColor);
    void fillPolygon(std::vector<Point> points, Color color);
    void drawImage(Rect rect, std::string imagePath);
    void drawText(Rect rect,
                  std::string text,
                  Color color,
                  TextRole textRole = TextRole::Default,
                  int fontScalePercent = 100);
    void reset();

    const std::vector<DrawCommand>& commands() const;

private:
    std::vector<DrawCommand> commands_;
};

} // namespace haru::engine::graphics
