#pragma once

#include <cstdint>

namespace haru::engine::graphics {

struct Color {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;

    friend bool operator==(const Color& left, const Color& right) {
        return left.r == right.r && left.g == right.g && left.b == right.b &&
               left.a == right.a;
    }

    friend bool operator!=(const Color& left, const Color& right) {
        return !(left == right);
    }
};

} // namespace haru::engine::graphics
