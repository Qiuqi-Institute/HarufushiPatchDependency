#pragma once

#include "SaveManager.hpp"

#include <HaruAsyncStartupTask>

#include <mutex>
#include <string>
#include <vector>

namespace haru::game::systems {

class HomeStartupPreparation {
public:
    bool start(std::vector<GameSave> saves, std::string dayLabel, std::string modLabel);
    bool started() const;
    bool ready() const;
    std::vector<std::string> saveSummaries();

    static std::vector<std::string> buildSaveSummaries(const std::vector<GameSave>& saves,
                                                       const std::string& dayLabel,
                                                       const std::string& modLabel);

private:
    mutable std::mutex mutex_;
    std::vector<std::string> saveSummaries_;
    bool prepared_ = false;
    engine::core::AsyncStartupTask task_;
};

} // namespace haru::game::systems
