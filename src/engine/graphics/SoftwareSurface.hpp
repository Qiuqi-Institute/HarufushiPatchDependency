#pragma once

#include "engine/graphics/Color.hpp"
#include "engine/graphics/Geometry.hpp"
#include "engine/graphics/RenderQueue.hpp"

#include <vector>

namespace haru::engine::graphics {

enum class TextRasterization {
    Placeholder,
    Skip,
};

class SoftwareSurface {
public:
    SoftwareSurface(int width, int height);

    int width() const;
    int height() const;

    void clear(Color color);
    void fillRect(Rect rect, Color color);
    void draw(const RenderQueue& queue,
              TextRasterization textRasterization = TextRasterization::Placeholder);

    Color pixelAt(int x, int y) const;
    const std::vector<Color>& pixels() const;

private:
    int width_;
    int height_;
    std::vector<Color> pixels_;
};

} // namespace haru::engine::graphics
