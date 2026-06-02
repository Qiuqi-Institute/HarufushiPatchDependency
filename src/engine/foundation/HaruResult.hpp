#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace haru::engine::foundation {

template <typename T>
class HaruResult {
public:
    static HaruResult ok(T value) {
        HaruResult result;
        result.hasValue_ = true;
        result.value_ = std::move(value);
        return result;
    }

    static HaruResult error(std::string message) {
        HaruResult result;
        result.error_ = std::move(message);
        return result;
    }

    bool hasValue() const {
        return hasValue_;
    }

    const T& value() const {
        if (!hasValue_) {
            throw std::logic_error("HaruResult has no value");
        }
        return value_;
    }

    const std::string& error() const {
        return error_;
    }

private:
    bool hasValue_ = false;
    T value_{};
    std::string error_;
};

} // namespace haru::engine::foundation
