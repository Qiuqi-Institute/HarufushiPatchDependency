#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace haru::engine::foundation {

class HaruString {
public:
    HaruString() = default;
    explicit HaruString(std::string utf8);

    static HaruString fromUtf8(std::string utf8);

    const std::string& toUtf8() const;
    bool empty() const;
    std::size_t byteSize() const;
    std::size_t codePointCount() const;
    bool startsWith(const HaruString& prefix) const;

    friend bool operator==(const HaruString& left, const HaruString& right) {
        return left.utf8_ == right.utf8_;
    }

    friend bool operator!=(const HaruString& left, const HaruString& right) {
        return !(left == right);
    }

private:
    std::string utf8_;
};

} // namespace haru::engine::foundation
