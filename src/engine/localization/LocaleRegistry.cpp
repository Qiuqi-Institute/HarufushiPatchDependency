#include "engine/localization/LocaleRegistry.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace haru::engine::localization {

namespace {

bool isBlank(const std::string& value) {
    return std::all_of(value.begin(), value.end(), [](unsigned char character) {
        return std::isspace(character) != 0;
    });
}

} // namespace

bool LocaleRegistry::add(std::string localeTag) {
    if (localeTag.empty() || isBlank(localeTag)) {
        return false;
    }

    const auto [_, inserted] = localeTags_.insert(std::move(localeTag));
    return inserted;
}

bool LocaleRegistry::contains(const std::string& localeTag) const {
    return localeTags_.find(localeTag) != localeTags_.end();
}

std::size_t LocaleRegistry::count() const {
    return localeTags_.size();
}

std::vector<std::string> LocaleRegistry::locales() const {
    return {localeTags_.begin(), localeTags_.end()};
}

} // namespace haru::engine::localization
