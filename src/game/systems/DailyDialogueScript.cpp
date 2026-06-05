#include "DailyDialogueScript.hpp"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace haru::game::systems {

namespace {

std::string trim(const std::string& text) {
    std::size_t begin = 0;
    while (begin < text.size() &&
           std::isspace(static_cast<unsigned char>(text[begin])) != 0) {
        ++begin;
    }

    std::size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
        --end;
    }

    return text.substr(begin, end - begin);
}

bool startsWith(const std::string& text, const std::string& prefix) {
    return text.rfind(prefix, 0) == 0;
}

DailyAction actionFromToken(const std::string& token) {
    if (token == "study") {
        return DailyAction::Study;
    }
    if (token == "modding") {
        return DailyAction::Modding;
    }
    if (token == "harufushi") {
        return DailyAction::SpendTimeWithHarufushi;
    }
    if (token == "rest") {
        return DailyAction::Rest;
    }
    throw std::runtime_error("unknown daily dialogue action: " + token);
}

int statValue(const DailyStats& stats, const std::string& name) {
    if (name == "day") {
        return stats.day;
    }
    if (name == "energy") {
        return stats.energy;
    }
    if (name == "study") {
        return stats.studyFocus;
    }
    if (name == "mod") {
        return stats.modProgress;
    }
    if (name == "bond") {
        return stats.harufushiBond;
    }
    if (name == "dependence") {
        return stats.dependence;
    }
    throw std::runtime_error("unknown daily dialogue stat: " + name);
}

bool conditionMatches(const std::string& condition, const DailyStats& stats) {
    std::istringstream input(condition);
    std::string stat;
    std::string operation;
    int value = 0;
    input >> stat >> operation >> value;
    if (stat.empty() || operation.empty() || input.fail()) {
        throw std::runtime_error("daily dialogue condition is malformed: " + condition);
    }

    const int actual = statValue(stats, stat);
    if (operation == ">=") {
        return actual >= value;
    }
    if (operation == "<=") {
        return actual <= value;
    }
    if (operation == "==") {
        return actual == value;
    }
    if (operation == ">") {
        return actual > value;
    }
    if (operation == "<") {
        return actual < value;
    }
    throw std::runtime_error("unknown daily dialogue condition operation: " + operation);
}

int conditionScore(const DailyDialogueEntry& entry) {
    int score = static_cast<int>(entry.conditions.size()) * 1000;
    for (const auto& condition : entry.conditions) {
        std::istringstream input(condition);
        std::string stat;
        std::string operation;
        int value = 0;
        input >> stat >> operation >> value;
        score += value;
    }
    return score;
}

std::string quotedValue(const std::string& text, int lineNumber) {
    const std::string value = trim(text);
    if (value.size() < 2U || value.front() != '"' || value.back() != '"') {
        throw std::runtime_error("daily dialogue line must be quoted on line " +
                                 std::to_string(lineNumber));
    }

    std::string result;
    result.reserve(value.size() - 2U);
    for (std::size_t index = 1; index + 1 < value.size(); ++index) {
        const char character = value[index];
        if (character != '\\') {
            result.push_back(character);
            continue;
        }
        if (index + 2 >= value.size()) {
            throw std::runtime_error("daily dialogue escape is truncated on line " +
                                     std::to_string(lineNumber));
        }
        const char escaped = value[++index];
        if (escaped == 'n') {
            result.push_back('\n');
        } else {
            result.push_back(escaped);
        }
    }
    return result;
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to open daily dialogue script: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::filesystem::path defaultScriptPath() {
#ifdef HARUFUSHI_SOURCE_DIR
    return std::filesystem::path(HARUFUSHI_SOURCE_DIR) / "resources" / "data" /
           "scripts" / "daily_dialogues.harudlg";
#else
    return std::filesystem::path("resources") / "data" / "scripts" /
           "daily_dialogues.harudlg";
#endif
}

} // namespace

DailyDialogueScript DailyDialogueScript::parse(const std::string& source) {
    DailyDialogueScript script;
    std::istringstream input(source);
    std::string line;
    bool headerSeen = false;
    std::optional<DailyDialogueEntry> current;
    int lineNumber = 0;

    const auto flushCurrent = [&]() {
        if (!current.has_value()) {
            return;
        }
        if (current->speaker.empty() || current->lines.empty()) {
            throw std::runtime_error("daily dialogue entry is incomplete");
        }
        script.entries_.push_back(std::move(*current));
        current.reset();
    };

    while (std::getline(input, line)) {
        ++lineNumber;
        const std::string stripped = trim(line);
        if (stripped.empty() || startsWith(stripped, "#")) {
            continue;
        }
        if (!headerSeen) {
            if (stripped != "harudlg v1") {
                throw std::runtime_error("daily dialogue script must start with 'harudlg v1'");
            }
            headerSeen = true;
            continue;
        }

        if (startsWith(stripped, "dialogue ")) {
            flushCurrent();
            std::istringstream header(stripped.substr(9));
            std::string locale;
            std::string action;
            header >> locale >> action;
            std::string speaker;
            std::getline(header, speaker);
            speaker = trim(speaker);
            if (locale.empty() || action.empty() || speaker.empty()) {
                throw std::runtime_error("daily dialogue header is incomplete on line " +
                                         std::to_string(lineNumber));
            }
            current = DailyDialogueEntry{locale, actionFromToken(action), "default", speaker, {}, {}};
            continue;
        }

        if (startsWith(stripped, "branch ")) {
            if (!current.has_value()) {
                throw std::runtime_error("daily dialogue branch appears before a dialogue header");
            }
            const std::string branchId = trim(stripped.substr(7));
            if (branchId.empty()) {
                throw std::runtime_error("daily dialogue branch id is empty on line " +
                                         std::to_string(lineNumber));
            }
            current->branchId = branchId;
            continue;
        }

        if (startsWith(stripped, "when ")) {
            if (!current.has_value()) {
                throw std::runtime_error("daily dialogue condition appears before a dialogue header");
            }
            const std::string condition = trim(stripped.substr(5));
            if (condition.empty()) {
                throw std::runtime_error("daily dialogue condition is empty on line " +
                                         std::to_string(lineNumber));
            }
            current->conditions.push_back(condition);
            continue;
        }

        if (startsWith(stripped, "line ")) {
            if (!current.has_value()) {
                throw std::runtime_error("daily dialogue line appears before a dialogue header");
            }
            current->lines.push_back(quotedValue(stripped.substr(5), lineNumber));
            continue;
        }

        throw std::runtime_error("unknown daily dialogue directive on line " +
                                 std::to_string(lineNumber));
    }

    if (!headerSeen) {
        throw std::runtime_error("daily dialogue script missing header");
    }
    flushCurrent();
    return script;
}

DailyDialogueScript DailyDialogueScript::loadDefault() {
    return parse(readFile(defaultScriptPath()));
}

std::optional<DailyDialogueEntry> DailyDialogueScript::entryFor(
    const std::string& locale,
    DailyAction action) const {
    for (const auto& entry : entries_) {
        if (entry.locale == locale && entry.action == action && entry.conditions.empty()) {
            return entry;
        }
    }
    for (const auto& entry : entries_) {
        if (entry.locale == "en-US" && entry.action == action && entry.conditions.empty()) {
            return entry;
        }
    }
    return std::nullopt;
}

std::optional<DailyDialogueEntry> DailyDialogueScript::entryFor(const std::string& locale,
                                                                DailyAction action,
                                                                const DailyStats& stats) const {
    const auto findBest = [&](const std::string& targetLocale) -> std::optional<DailyDialogueEntry> {
        const DailyDialogueEntry* best = nullptr;
        int bestScore = -1;
        for (const auto& entry : entries_) {
            if (entry.locale != targetLocale || entry.action != action) {
                continue;
            }
            bool matches = true;
            for (const auto& condition : entry.conditions) {
                if (!conditionMatches(condition, stats)) {
                    matches = false;
                    break;
                }
            }
            if (!matches) {
                continue;
            }
            const int score = conditionScore(entry);
            if (score > bestScore) {
                best = &entry;
                bestScore = score;
            }
        }
        if (best == nullptr) {
            return std::nullopt;
        }
        return *best;
    };

    auto active = findBest(locale);
    if (active.has_value()) {
        return active;
    }
    return findBest("en-US");
}

} // namespace haru::game::systems
