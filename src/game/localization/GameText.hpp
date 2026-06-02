#pragma once

#include <HaruFrame>

#include "../systems/DailyLoopState.hpp"

#include <map>
#include <string>

namespace haru::game::localization {

enum class TextId {
    GameTitle,
    Home,
    HomePrompt,
    NewGame,
    Load,
    Settings,
    Quit,
    SaveFiles,
    NoSaveDataYet,
    Back,
    Audio80,
    Visual100,
    TextSpeedNormal,
    Language,
    LanguageEnglish,
    LanguageSimplifiedChinese,
    LanguageJapanese,
    SettingsTabGame,
    SettingsTabAudio,
    SettingsTabDisplay,
    SettingsMasterVolume,
    SettingsWindowScale,
    SettingsTextSpeed,
    ReturnHome,
    Study,
    Modding,
    Harufushi,
    Rest,
    HomeBoardTitle,
    HomeBoardPatch,
    HomeBoardCompile,
    HomeHarufushiStatus,
    DailyBoardTitle,
    DailyEventPreview,
    Day,
    Energy,
    StudyStat,
    ModStat,
    Bond,
    Dependence,
};

class GameText {
public:
    static GameText loadDefault(std::string activeLocale = "en-US");
    static GameText loadFromDirectory(const std::string& localizationRoot,
                                      std::string activeLocale = "en-US");

    bool setLocale(const std::string& localeTag);
    const std::string& activeLocale() const;
    const engine::localization::LocaleRegistry& locales() const;
    std::string get(TextId id) const;
    std::string formatDailyStats(const systems::DailyStats& stats) const;

private:
    using TextTable = std::map<std::string, std::string>;

    std::string activeLocale_ = "en-US";
    engine::localization::LocaleRegistry locales_;
    std::map<std::string, TextTable> tables_;
};

} // namespace haru::game::localization
