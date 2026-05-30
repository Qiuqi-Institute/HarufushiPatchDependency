#pragma once

#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace haru::engine::localization {

class LocaleRegistry {
public:
    bool add(std::string localeTag);
    bool contains(const std::string& localeTag) const;
    std::size_t count() const;
    std::vector<std::string> locales() const;

private:
    std::set<std::string> localeTags_;
};

} // namespace haru::engine::localization
