#include "HomeStartupPreparation.hpp"

#include <sstream>
#include <utility>

namespace haru::game::systems {

bool HomeStartupPreparation::start(std::vector<GameSave> saves,
                                   std::string dayLabel,
                                   std::string modLabel) {
    return task_.start([this,
                        saves = std::move(saves),
                        dayLabel = std::move(dayLabel),
                        modLabel = std::move(modLabel)]() {
        auto summaries = buildSaveSummaries(saves, dayLabel, modLabel);
        std::lock_guard<std::mutex> lock(mutex_);
        saveSummaries_ = std::move(summaries);
        prepared_ = true;
    });
}

bool HomeStartupPreparation::started() const {
    return task_.started();
}

bool HomeStartupPreparation::ready() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return prepared_;
}

std::vector<std::string> HomeStartupPreparation::saveSummaries() {
    if (task_.started()) {
        task_.wait();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    return saveSummaries_;
}

std::vector<std::string> HomeStartupPreparation::buildSaveSummaries(
    const std::vector<GameSave>& saves,
    const std::string& dayLabel,
    const std::string& modLabel) {
    std::vector<std::string> summaries;
    summaries.reserve(saves.size());
    int slot = 1;
    for (const auto& save : saves) {
        std::ostringstream summary;
        summary << "Save " << slot << "  " << dayLabel << ' ' << save.stats.day
                << "  " << modLabel << ' ' << save.stats.modProgress;
        summaries.push_back(summary.str());
        ++slot;
    }
    return summaries;
}

} // namespace haru::game::systems
