#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace haru::engine::resources {

class ResourceId {
public:
    static std::optional<ResourceId> parse(std::string_view text);

    const std::string& value() const;

    friend bool operator==(const ResourceId& left, const ResourceId& right) {
        return left.value_ == right.value_;
    }

    friend bool operator!=(const ResourceId& left, const ResourceId& right) {
        return !(left == right);
    }

private:
    explicit ResourceId(std::string value);

    std::string value_;
};

} // namespace haru::engine::resources
