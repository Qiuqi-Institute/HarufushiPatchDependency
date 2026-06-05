#pragma once

#include "DailyLoopState.hpp"

#include <optional>
#include <string>
#include <vector>

namespace haru::game::systems {

struct DailyDialogueEntry {
    std::string locale;
    DailyAction action = DailyAction::Study;
    std::string branchId = "default";
    std::string speaker;
    std::vector<std::string> lines;
    std::vector<std::string> conditions;
};

class DailyDialogueScript {
public:
    static DailyDialogueScript parse(const std::string& source);
    static DailyDialogueScript loadDefault();

    std::optional<DailyDialogueEntry> entryFor(const std::string& locale,
                                               DailyAction action) const;
    std::optional<DailyDialogueEntry> entryFor(const std::string& locale,
                                               DailyAction action,
                                               const DailyStats& stats) const;

private:
    std::vector<DailyDialogueEntry> entries_;
};

} // namespace haru::game::systems
