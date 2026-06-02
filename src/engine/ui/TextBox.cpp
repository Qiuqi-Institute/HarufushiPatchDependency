#include "engine/ui/TextBox.hpp"

#include <algorithm>
#include <cstdint>
#include <utility>

namespace haru::engine::ui {

namespace {

std::uint32_t nextCodePoint(std::string_view text, std::size_t& cursor) {
    const unsigned char first = static_cast<unsigned char>(text[cursor]);
    if (first < 0x80U) {
        ++cursor;
        return first;
    }

    int trailing = 0;
    std::uint32_t codePoint = 0;
    if ((first & 0xE0U) == 0xC0U) {
        trailing = 1;
        codePoint = first & 0x1FU;
    } else if ((first & 0xF0U) == 0xE0U) {
        trailing = 2;
        codePoint = first & 0x0FU;
    } else if ((first & 0xF8U) == 0xF0U) {
        trailing = 3;
        codePoint = first & 0x07U;
    } else {
        ++cursor;
        return 0xFFFD;
    }

    ++cursor;
    for (int index = 0; index < trailing && cursor < text.size(); ++index) {
        const unsigned char byte = static_cast<unsigned char>(text[cursor]);
        if ((byte & 0xC0U) != 0x80U) {
            return 0xFFFD;
        }
        codePoint = (codePoint << 6U) | (byte & 0x3FU);
        ++cursor;
    }

    return codePoint;
}

bool isWideCodePoint(std::uint32_t codePoint) {
    return (codePoint >= 0x1100U && codePoint <= 0x11FFU) ||
           (codePoint >= 0x2E80U && codePoint <= 0xA4CFU) ||
           (codePoint >= 0xAC00U && codePoint <= 0xD7AFU) ||
           (codePoint >= 0xF900U && codePoint <= 0xFAFFU) ||
           (codePoint >= 0xFE10U && codePoint <= 0xFE6FU) ||
           (codePoint >= 0xFF00U && codePoint <= 0xFF60U);
}

int codePointWidth(std::uint32_t codePoint) {
    if (codePoint == ' ') {
        return 5;
    }
    if (codePoint < 0x80U) {
        return 10;
    }
    return isWideCodePoint(codePoint) ? 14 : 11;
}

} // namespace

TextBox::TextBox(graphics::Rect bounds, std::string text, TextBoxStyle style)
    : bounds_(bounds), text_(std::move(text)), style_(style) {}

void TextBox::render(graphics::RenderQueue& queue) const {
    queue.drawText(textRect(), text_, style_.text);
}

graphics::Rect TextBox::textRect() const {
    const int estimatedWidth = estimateTextWidth(text_);
    const int availableWidth = std::max(bounds_.width, 0);
    const int availableRoom = std::max(availableWidth - estimatedWidth, 0);
    const int padding = std::clamp(availableRoom / 6,
                                   std::max(style_.minHorizontalPadding, 0),
                                   std::max(style_.maxHorizontalPadding,
                                            style_.minHorizontalPadding));
    const int desiredWidth = std::max(style_.minWidth, estimatedWidth + (padding * 2));
    const int width = std::clamp(desiredWidth, 0, availableWidth);
    return {bounds_.x + ((availableWidth - width) / 2), bounds_.y, width, bounds_.height};
}

int TextBox::estimateTextWidth(std::string_view text) {
    int width = 0;
    std::size_t cursor = 0;
    while (cursor < text.size()) {
        width += codePointWidth(nextCodePoint(text, cursor));
    }
    return width;
}

} // namespace haru::engine::ui
