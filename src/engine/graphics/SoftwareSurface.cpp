#include "engine/graphics/SoftwareSurface.hpp"

#include <algorithm>
#include <stdexcept>

namespace haru::engine::graphics {

namespace {

constexpr int placeholderGlyphWidth = 5;
constexpr int placeholderGlyphHeight = 9;
constexpr int placeholderGlyphGap = 2;

std::uint8_t lerpChannel(std::uint8_t from, std::uint8_t to, int step, int steps) {
    if (steps <= 0) {
        return to;
    }

    const int value = static_cast<int>(from) +
                      ((static_cast<int>(to) - static_cast<int>(from)) * step) / steps;
    return static_cast<std::uint8_t>(std::clamp(value, 0, 255));
}

Color lerpColor(Color from, Color to, int step, int steps) {
    return {lerpChannel(from.r, to.r, step, steps),
            lerpChannel(from.g, to.g, step, steps),
            lerpChannel(from.b, to.b, step, steps),
            lerpChannel(from.a, to.a, step, steps)};
}

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

namespace {

void fillClippedRect(std::vector<Color>& pixels,
                     int surfaceWidth,
                     int surfaceHeight,
                     Rect rect,
                     Color color) {
    const int left = std::max(rect.x, 0);
    const int top = std::max(rect.y, 0);
    const int right = std::min(rect.x + rect.width, surfaceWidth);
    const int bottom = std::min(rect.y + rect.height, surfaceHeight);

    if (left >= right || top >= bottom) {
        return;
    }

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            pixels[static_cast<std::size_t>(y * surfaceWidth + x)] = color;
        }
    }
}

void fillRoundedRectPixels(std::vector<Color>& pixels,
                           int surfaceWidth,
                           int surfaceHeight,
                           Rect rect,
                           Color color,
                           int radius) {
    const int left = std::max(rect.x, 0);
    const int top = std::max(rect.y, 0);
    const int right = std::min(rect.x + rect.width, surfaceWidth);
    const int bottom = std::min(rect.y + rect.height, surfaceHeight);

    if (left >= right || top >= bottom) {
        return;
    }

    const int safeRadius = std::max(0, std::min(radius, std::min(rect.width, rect.height) / 2));
    const int radiusSquared = safeRadius * safeRadius;

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            const int localX = x - rect.x;
            const int localY = y - rect.y;
            int dx = 0;
            int dy = 0;

            if (localX < safeRadius) {
                dx = safeRadius - localX;
            } else if (localX >= rect.width - safeRadius) {
                dx = localX - (rect.width - safeRadius - 1);
            }

            if (localY < safeRadius) {
                dy = safeRadius - localY;
            } else if (localY >= rect.height - safeRadius) {
                dy = localY - (rect.height - safeRadius - 1);
            }

            if (dx == 0 || dy == 0 || (dx * dx) + (dy * dy) <= radiusSquared) {
                pixels[static_cast<std::size_t>(y * surfaceWidth + x)] = color;
            }
        }
    }
}

void fillEllipsePixels(std::vector<Color>& pixels,
                       int surfaceWidth,
                       int surfaceHeight,
                       Rect rect,
                       Color color) {
    const int left = std::max(rect.x, 0);
    const int top = std::max(rect.y, 0);
    const int right = std::min(rect.x + rect.width, surfaceWidth);
    const int bottom = std::min(rect.y + rect.height, surfaceHeight);

    if (left >= right || top >= bottom || rect.width <= 0 || rect.height <= 0) {
        return;
    }

    const double radiusX = static_cast<double>(rect.width) / 2.0;
    const double radiusY = static_cast<double>(rect.height) / 2.0;
    const double centerX = static_cast<double>(rect.x) + radiusX;
    const double centerY = static_cast<double>(rect.y) + radiusY;

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            const double normalizedX = (static_cast<double>(x) + 0.5 - centerX) / radiusX;
            const double normalizedY = (static_cast<double>(y) + 0.5 - centerY) / radiusY;
            if ((normalizedX * normalizedX) + (normalizedY * normalizedY) <= 1.0) {
                pixels[static_cast<std::size_t>(y * surfaceWidth + x)] = color;
            }
        }
    }
}

void fillVerticalGradientPixels(std::vector<Color>& pixels,
                                int surfaceWidth,
                                int surfaceHeight,
                                Rect rect,
                                Color topColor,
                                Color bottomColor) {
    const int left = std::max(rect.x, 0);
    const int top = std::max(rect.y, 0);
    const int right = std::min(rect.x + rect.width, surfaceWidth);
    const int bottom = std::min(rect.y + rect.height, surfaceHeight);

    if (left >= right || top >= bottom) {
        return;
    }

    const int steps = std::max(rect.height - 1, 1);
    for (int y = top; y < bottom; ++y) {
        const Color color = lerpColor(topColor, bottomColor, y - rect.y, steps);
        for (int x = left; x < right; ++x) {
            pixels[static_cast<std::size_t>(y * surfaceWidth + x)] = color;
        }
    }
}

bool pointInsidePolygon(const std::vector<Point>& points, double x, double y) {
    bool inside = false;
    std::size_t previous = points.size() - 1U;

    for (std::size_t current = 0; current < points.size(); ++current) {
        const Point a = points[current];
        const Point b = points[previous];
        const bool crosses = (static_cast<double>(a.y) > y) !=
                             (static_cast<double>(b.y) > y);
        if (crosses) {
            const double intersectX =
                static_cast<double>(b.x - a.x) * (y - static_cast<double>(a.y)) /
                    static_cast<double>(b.y - a.y) +
                static_cast<double>(a.x);
            if (x < intersectX) {
                inside = !inside;
            }
        }
        previous = current;
    }

    return inside;
}

void fillPolygonPixels(std::vector<Color>& pixels,
                       int surfaceWidth,
                       int surfaceHeight,
                       const std::vector<Point>& points,
                       Color color) {
    if (points.size() < 3U) {
        return;
    }

    int minX = points.front().x;
    int maxX = points.front().x;
    int minY = points.front().y;
    int maxY = points.front().y;
    for (const Point point : points) {
        minX = std::min(minX, point.x);
        maxX = std::max(maxX, point.x);
        minY = std::min(minY, point.y);
        maxY = std::max(maxY, point.y);
    }

    const int left = std::max(minX, 0);
    const int top = std::max(minY, 0);
    const int right = std::min(maxX + 1, surfaceWidth);
    const int bottom = std::min(maxY + 1, surfaceHeight);

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            if (pointInsidePolygon(points,
                                   static_cast<double>(x) + 0.5,
                                   static_cast<double>(y) + 0.5)) {
                pixels[static_cast<std::size_t>(y * surfaceWidth + x)] = color;
            }
        }
    }
}

} // namespace

void SoftwareSurface::draw(const RenderQueue& queue, TextRasterization textRasterization) {
    for (const auto& command : queue.commands()) {
        switch (command.kind) {
        case DrawCommandKind::Clear:
            clear(command.color);
            break;
        case DrawCommandKind::FillRect:
            fillRect(command.rect, command.color);
            break;
        case DrawCommandKind::FillRoundedRect:
            fillRoundedRectPixels(pixels_,
                                  width_,
                                  height_,
                                  command.rect,
                                  command.color,
                                  command.radius);
            break;
        case DrawCommandKind::FillEllipse:
            fillEllipsePixels(pixels_, width_, height_, command.rect, command.color);
            break;
        case DrawCommandKind::StrokeRect:
            fillClippedRect(pixels_,
                            width_,
                            height_,
                            {command.rect.x,
                             command.rect.y,
                             command.rect.width,
                             command.thickness},
                            command.color);
            fillClippedRect(pixels_,
                            width_,
                            height_,
                            {command.rect.x,
                             command.rect.y + command.rect.height - command.thickness,
                             command.rect.width,
                             command.thickness},
                            command.color);
            fillClippedRect(pixels_,
                            width_,
                            height_,
                            {command.rect.x,
                             command.rect.y,
                             command.thickness,
                             command.rect.height},
                            command.color);
            fillClippedRect(pixels_,
                            width_,
                            height_,
                            {command.rect.x + command.rect.width - command.thickness,
                             command.rect.y,
                             command.thickness,
                             command.rect.height},
                            command.color);
            break;
        case DrawCommandKind::FillVerticalGradient:
            fillVerticalGradientPixels(pixels_,
                                       width_,
                                       height_,
                                       command.rect,
                                       command.color,
                                       command.secondaryColor);
            break;
        case DrawCommandKind::FillPolygon:
            fillPolygonPixels(pixels_, width_, height_, command.points, command.color);
            break;
        case DrawCommandKind::Image:
            break;
        case DrawCommandKind::Text: {
            if (textRasterization == TextRasterization::Skip) {
                break;
            }

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
