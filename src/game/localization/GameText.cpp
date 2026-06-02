#include "GameText.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace haru::game::localization {

namespace {

std::string textKey(TextId id) {
    switch (id) {
    case TextId::GameTitle:
        return "game.title";
    case TextId::Home:
        return "home.title";
    case TextId::HomePrompt:
        return "home.prompt";
    case TextId::NewGame:
        return "menu.new_game";
    case TextId::Load:
        return "menu.load";
    case TextId::Settings:
        return "menu.settings";
    case TextId::Quit:
        return "menu.quit";
    case TextId::SaveFiles:
        return "saves.title";
    case TextId::NoSaveDataYet:
        return "saves.empty";
    case TextId::Back:
        return "menu.back";
    case TextId::Audio80:
        return "settings.audio";
    case TextId::Visual100:
        return "settings.visual";
    case TextId::TextSpeedNormal:
        return "settings.text_speed";
    case TextId::Language:
        return "settings.language";
    case TextId::LanguageEnglish:
        return "language.english";
    case TextId::LanguageSimplifiedChinese:
        return "language.zh_cn";
    case TextId::LanguageJapanese:
        return "language.ja_jp";
    case TextId::SettingsTabGame:
        return "settings.tab.game";
    case TextId::SettingsTabAudio:
        return "settings.tab.audio";
    case TextId::SettingsTabDisplay:
        return "settings.tab.display";
    case TextId::SettingsMasterVolume:
        return "settings.master_volume";
    case TextId::SettingsWindowScale:
        return "settings.window_scale";
    case TextId::SettingsTextSpeed:
        return "settings.text_speed_label";
    case TextId::ReturnHome:
        return "nav.home";
    case TextId::Study:
        return "daily.study";
    case TextId::Modding:
        return "daily.modding";
    case TextId::Harufushi:
        return "daily.harufushi";
    case TextId::Rest:
        return "daily.rest";
    case TextId::HomeBoardTitle:
        return "home.board_title";
    case TextId::HomeBoardPatch:
        return "home.board_patch";
    case TextId::HomeBoardCompile:
        return "home.board_compile";
    case TextId::HomeHarufushiStatus:
        return "home.harufushi_status";
    case TextId::DailyBoardTitle:
        return "daily.board_title";
    case TextId::DailyEventPreview:
        return "daily.event_preview";
    case TextId::Day:
        return "stat.day";
    case TextId::Energy:
        return "stat.energy";
    case TextId::StudyStat:
        return "stat.study";
    case TextId::ModStat:
        return "stat.mod";
    case TextId::Bond:
        return "stat.bond";
    case TextId::Dependence:
        return "stat.dependence";
    }

    return {};
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to open harulang file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

} // namespace

GameText GameText::loadDefault(std::string activeLocale) {
    return loadFromDirectory(std::string{HARUFUSHI_SOURCE_DIR} + "/resources/localization",
                             activeLocale);
}

GameText GameText::loadFromDirectory(const std::string& localizationRoot,
                                     std::string activeLocale) {
    GameText text;
    const std::filesystem::path root(localizationRoot);

    if (!std::filesystem::exists(root)) {
        throw std::runtime_error("localization root does not exist: " + root.string());
    }

    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (!entry.is_directory()) {
            continue;
        }

        const auto documentPath = entry.path() / "game.harulang";
        if (!std::filesystem::exists(documentPath)) {
            continue;
        }

        const auto document =
            engine::localization::HaruLanguageDocument::parse(readFile(documentPath));
        text.locales_.add(document.locale());
        text.tables_.emplace(document.locale(), document.entries());
    }

    if (text.locales_.count() == 0) {
        throw std::runtime_error("no harulang localization files were loaded");
    }

    if (!text.setLocale(activeLocale)) {
        text.setLocale("en-US");
    }

    return text;
}

bool GameText::setLocale(const std::string& localeTag) {
    if (!locales_.contains(localeTag) || tables_.find(localeTag) == tables_.end()) {
        return false;
    }

    activeLocale_ = localeTag;
    return true;
}

const std::string& GameText::activeLocale() const {
    return activeLocale_;
}

const engine::localization::LocaleRegistry& GameText::locales() const {
    return locales_;
}

std::string GameText::get(TextId id) const {
    const std::string key = textKey(id);
    const auto activeTable = tables_.find(activeLocale_);
    if (activeTable != tables_.end()) {
        const auto value = activeTable->second.find(key);
        if (value != activeTable->second.end()) {
            return value->second;
        }
    }

    const auto fallbackTable = tables_.find("en-US");
    if (fallbackTable != tables_.end()) {
        const auto value = fallbackTable->second.find(key);
        if (value != fallbackTable->second.end()) {
            return value->second;
        }
    }

    return {};
}

std::string GameText::formatDailyStats(const systems::DailyStats& stats) const {
    std::ostringstream line;
    if (activeLocale_ == "zh-CN") {
        line << get(TextId::Day) << ' ' << stats.day << " 天  "
             << get(TextId::Energy) << ' ' << stats.energy << "  "
             << get(TextId::StudyStat) << ' ' << stats.studyFocus << "  "
             << get(TextId::ModStat) << ' ' << stats.modProgress << "  "
             << get(TextId::Bond) << ' ' << stats.harufushiBond << "  "
             << get(TextId::Dependence) << ' ' << stats.dependence;
        return line.str();
    }

    line << get(TextId::Day) << ' ' << stats.day << "  "
         << get(TextId::Energy) << ' ' << stats.energy << "  "
         << get(TextId::StudyStat) << ' ' << stats.studyFocus << "  "
         << get(TextId::ModStat) << ' ' << stats.modProgress << "  "
         << get(TextId::Bond) << ' ' << stats.harufushiBond << "  "
         << get(TextId::Dependence) << ' ' << stats.dependence;
    return line.str();
}

} // namespace haru::game::localization
