#pragma once

#include <map>
#include <string>

namespace haru::engine::localization {

class HaruLanguageDocument {
public:
    static HaruLanguageDocument parse(const std::string& source);

    const std::string& packageName() const;
    const std::string& locale() const;
    bool contains(const std::string& key) const;
    std::string get(const std::string& key) const;
    const std::map<std::string, std::string>& entries() const;

private:
    std::string packageName_;
    std::string locale_;
    std::map<std::string, std::string> entries_;
};

} // namespace haru::engine::localization
