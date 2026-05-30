#include "engine/resources/ResourceId.hpp"

#include <algorithm>
#include <cctype>

namespace haru::engine::resources {

namespace {

bool isAllowed(char character) {
    const auto byte = static_cast<unsigned char>(character);
    return std::isalnum(byte) != 0 || character == '_' || character == '-' || character == '.';
}

bool hasEmptySegment(std::string_view text) {
    if (text.front() == '.' || text.back() == '.') {
        return true;
    }

    for (std::size_t index = 1; index < text.size(); ++index) {
        if (text[index - 1] == '.' && text[index] == '.') {
            return true;
        }
    }

    return false;
}

} // namespace

std::optional<ResourceId> ResourceId::parse(std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }

    if (text.find('/') != std::string_view::npos || text.find('\\') != std::string_view::npos) {
        return std::nullopt;
    }

    if (hasEmptySegment(text)) {
        return std::nullopt;
    }

    if (!std::all_of(text.begin(), text.end(), isAllowed)) {
        return std::nullopt;
    }

    return ResourceId(std::string(text));
}

const std::string& ResourceId::value() const {
    return value_;
}

ResourceId::ResourceId(std::string value) : value_(std::move(value)) {}

} // namespace haru::engine::resources
