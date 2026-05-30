#include "engine/graphics/SoftwareSurface.hpp"

#include <algorithm>
#include <stdexcept>

namespace haru::engine::graphics {

namespace {

constexpr int placeholderGlyphWidth = 5;
constexpr int placeholderGlyphHeight = 9;
constexpr int placeholderGlyphGap = 2;

} // namespace

SoftwareSurface::SoftwareSurface(int width, int height)
    : width_(width), height_(height), pixels_(static_cast<std::size_t>(width * height)) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("software surface dimensions must be positive");
    }
}

int SoftwareSurface::width() const {
    return width_;
}

int SoftwareSurface::height() const {
    return height_;
}

void SoftwareSurface::clear(Color color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void SoftwareSurface::fillRect(Rect rect, Color color) {
    const int left = std::max(rect.x, 0);
    const int top = std::max(rect.y, 0);
    const int right = std::min(rect.x + rect.width, width_);
    const int bottom = std::min(rect.y + rect.height, height_);

    if (left >= right || top >= bottom) {
        return;
    }

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            pixels_[static_cast<std::size_t>(y * width_ + x)] = color;
        }
    }
}

void SoftwareSurface::draw(const RenderQueue& queue) {
    for (const auto& command : queue.commands()) {
        switch (command.kind) {
        case DrawCommandKind::Clear:
            clear(command.color);
            break;
        case DrawCommandKind::FillRect:
            fillRect(command.rect, command.color);
            break;
        case DrawCommandKind::Text: {
            int cursorX = command.rect.x;
            for (const char character : command.text) {
                if (character != ' ') {
                    fillRect({cursorX, command.rect.y, placeholderGlyphWidth,
                              std::min(placeholderGlyphHeight, command.rect.height)},
                             command.color);
                }

                cursorX += placeholderGlyphWidth + placeholderGlyphGap;
                if (cursorX >= command.rect.x + command.rect.width) {
                    break;
                }
            }
            break;
        }
        }
    }
}

Color SoftwareSurface::pixelAt(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        throw std::out_of_range("software surface pixel coordinate out of range");
    }

    return pixels_[static_cast<std::size_t>(y * width_ + x)];
}

const std::vector<Color>& SoftwareSurface::pixels() const {
    return pixels_;
}

} // namespace haru::engine::graphics
