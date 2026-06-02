#pragma once

#include "DailyLoopState.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace haru::game::systems {

enum class SaveOperationKind : std::uint8_t {
    NewGame = 1,
    DailyAction = 2,
    LocaleChanged = 3,
};

struct SaveOperation {
    std::uint64_t sequence = 0;
    SaveOperationKind kind = SaveOperationKind::NewGame;
    DailyAction dailyAction = DailyAction::Study;
    DailyStats stats;
    std::string localeTag;
};

struct GameSave {
    std::string id;
    std::string localeTag;
    DailyStats stats;
    std::vector<SaveOperation> history;
};

struct SaveLoadReport {
    int loaded = 0;
    int rejected = 0;
};

class SaveManager {
public:
    explicit SaveManager(std::filesystem::path saveRoot);

    SaveLoadReport loadAll();
    const std::vector<GameSave>& saves() const;

    const GameSave& createNewGame(std::string localeTag);
    void recordDailyAction(DailyAction action,
                           const DailyStats& statsAfter,
                           std::string localeTag);
    void recordLocaleChange(std::string localeTag);

    const GameSave* activeSave() const;
    GameSave* activeSave();
    bool activateSave(const std::string& saveId);

    std::filesystem::path savePath(const std::string& saveId) const;
    const std::filesystem::path& saveRoot() const;

private:
    void flush(const GameSave& save) const;
    GameSave& requireActiveSave(std::string localeTag);
    static std::string createSaveId();

    std::filesystem::path saveRoot_;
    std::vector<GameSave> saves_;
    std::optional<std::size_t> activeIndex_;
};

} // namespace haru::game::systems
