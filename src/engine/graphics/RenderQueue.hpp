#pragma once

#include "engine/graphics/Color.hpp"
#include "engine/graphics/Geometry.hpp"

#include <vector>

namespace haru::engine::graphics {

enum class DrawCommandKind {
    Clear,
    FillRect,
};

struct DrawCommand {
    DrawCommandKind kind;
    Rect rect;
    Color color;
};

class RenderQueue {
public:
    void clear(Color color);
    void fillRect(Rect rect, Color color);
    void reset();

    const std::vector<DrawCommand>& commands() const;

private:
    std::vector<DrawCommand> commands_;
};

} // namespace haru::engine::graphics
