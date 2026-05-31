#include "engine/localization/HaruLanguageDocument.hpp"

#include <cctype>
#include <sstream>
#include <stdexcept>

namespace haru::engine::localization {

namespace {

std::string trim(const std::string& value) {
    std::size_t begin = 0;
    while (begin < value.size() &&
           std::isspace(static_cast<unsigned char>(value[begin])) != 0) {
        ++begin;
    }

    std::size_t end = value.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return value.substr(begin, end - begin);
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

std::string unquote(const std::string& value, int lineNumber) {
    const std::string trimmed = trim(value);
    if (trimmed.size() < 2 || trimmed.front() != '"' || trimmed.back() != '"') {
        throw std::runtime_error("harulang line " + std::to_string(lineNumber) +
                                 " must use a quoted string value");
    }

    std::string result;
    bool escaping = false;
    for (std::size_t index = 1; index + 1 < trimmed.size(); ++index) {
        const char character = trimmed[index];
        if (escaping) {
            switch (character) {
            case 'n':
                result.push_back('\n');
                break;
            case '"':
            case '\\':
                result.push_back(character);
                break;
            default:
                result.push_back(character);
                break;
            }
            escaping = false;
        } else if (character == '\\') {
            escaping = true;
        } else {
            result.push_back(character);
        }
    }

    if (escaping) {
        throw std::runtime_error("harulang line " + std::to_string(lineNumber) +
                                 " ends with an incomplete escape");
    }

    return result;
}

} // namespace

HaruLanguageDocument HaruLanguageDocument::parse(const std::string& source) {
    HaruLanguageDocument document;
    std::istringstream input(source);
    std::string line;
    bool headerSeen = false;
    int lineNumber = 0;

    while (std::getline(input, line)) {
        ++lineNumber;
        const std::string stripped = trim(line);
        if (stripped.empty() || startsWith(stripped, "#")) {
            continue;
        }

        if (!headerSeen) {
            if (stripped != "harulang v1") {
                throw std::runtime_error("harulang document must start with 'harulang v1'");
            }
            headerSeen = true;
            continue;
        }

        if (startsWith(stripped, "package ")) {
            document.packageName_ = trim(stripped.substr(8));
            continue;
        }

        if (startsWith(stripped, "locale ")) {
            document.locale_ = trim(stripped.substr(7));
            continue;
        }

        if (startsWith(stripped, "text ")) {
            const std::string textDeclaration = trim(stripped.substr(5));
            const std::size_t equals = textDeclaration.find('=');
            if (equals == std::string::npos) {
                throw std::runtime_error("harulang text entry missing '=' on line " +
                                         std::to_string(lineNumber));
            }

            const std::string key = trim(textDeclaration.substr(0, equals));
            const std::string value = unquote(textDeclaration.substr(equals + 1), lineNumber);
            if (key.empty()) {
                throw std::runtime_error("harulang text entry has an empty key on line " +
                                         std::to_string(lineNumber));
            }
            document.entries_[key] = value;
            continue;
        }

        throw std::runtime_error("unknown harulang directive on line " +
                                 std::to_string(lineNumber));
    }

    if (!headerSeen || document.packageName_.empty() || document.locale_.empty()) {
        throw std::runtime_error("harulang document missing header, package, or locale");
    }

    return document;
}

const std::string& HaruLanguageDocument::packageName() const {
    return packageName_;
}

const std::string& HaruLanguageDocument::locale() const {
    return locale_;
}

bool HaruLanguageDocument::contains(const std::string& key) const {
    return entries_.find(key) != entries_.end();
}

std::string HaruLanguageDocument::get(const std::string& key) const {
    const auto value = entries_.find(key);
    return value == entries_.end() ? std::string{} : value->second;
}

const std::map<std::string, std::string>& HaruLanguageDocument::entries() const {
    return entries_;
}

} // namespace haru::engine::localization
