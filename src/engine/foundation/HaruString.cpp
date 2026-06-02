#include "engine/foundation/HaruString.hpp"

#include <algorithm>
#include <utility>

namespace haru::engine::foundation {

namespace {

std::size_t nextCodePointLength(unsigned char first) {
    if (first < 0x80U) {
        return 1;
    }
    if ((first & 0xE0U) == 0xC0U) {
        return 2;
    }
    if ((first & 0xF0U) == 0xE0U) {
        return 3;
    }
    if ((first & 0xF8U) == 0xF0U) {
        return 4;
    }
    return 1;
}

} // namespace

HaruString::HaruString(std::string utf8) : utf8_(std::move(utf8)) {}

HaruString HaruString::fromUtf8(std::string utf8) {
    return HaruString(std::move(utf8));
}

const std::string& HaruString::toUtf8() const {
    return utf8_;
}

bool HaruString::empty() const {
    return utf8_.empty();
}

std::size_t HaruString::byteSize() const {
    return utf8_.size();
}

std::size_t HaruString::codePointCount() const {
    std::size_t count = 0;
    for (std::size_t cursor = 0; cursor < utf8_.size();) {
        cursor += std::min(nextCodePointLength(static_cast<unsigned char>(utf8_[cursor])),
                           utf8_.size() - cursor);
        ++count;
    }
    return count;
}

bool HaruString::startsWith(const HaruString& prefix) const {
    return utf8_.rfind(prefix.utf8_, 0) == 0;
}

} // namespace haru::engine::foundation
