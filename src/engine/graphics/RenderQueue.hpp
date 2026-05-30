#pragma once

#include "Color.hpp"
#include "Geometry.hpp"

#include <string>
#include <vector>

namespace haru::engine::graphics {

enum class DrawCommandKind {
    Clear,
    FillRect,
    Text,
};

struct DrawCommand {
    DrawCommandKind kind;
    Rect rect;
    Color color;
    std::string text;
};

class RenderQueue {
public:
    void clear(Color color);
    void fillRect(Rect rect, Color color);
    void drawText(Rect rect, std::string text, Color color);
    void reset();

    const std::vector<DrawCommand>& commands() const;

private:
    std::vector<DrawCommand> commands_;
};

} // namespace haru::engine::graphics
