#include "support/TestHarness.hpp"

#include "game/localization/GameText.hpp"

HARU_TEST(game_text_registers_supported_i18n_locales_without_scene_branches) {
    haru::game::localization::GameText text =
        haru::game::localization::GameText::loadDefault("zh-CN");

    HARU_EXPECT_TRUE(text.locales().contains("en-US"));
    HARU_EXPECT_TRUE(text.locales().contains("zh-CN"));
    HARU_EXPECT_TRUE(text.locales().contains("ja-JP"));
    HARU_EXPECT_EQ(text.get(haru::game::localization::TextId::NewGame), "开始游戏");

    HARU_EXPECT_TRUE(text.setLocale("ja-JP"));
    HARU_EXPECT_EQ(text.get(haru::game::localization::TextId::Settings), "設定");
    HARU_EXPECT_EQ(text.get(haru::game::localization::TextId::GameTitle),
                   "ハルフシ・パッチ・ディペンデンシー");
    HARU_EXPECT_FALSE(text.setLocale("ko-KR"));
    HARU_EXPECT_EQ(text.activeLocale(), "ja-JP");
}

HARU_TEST(game_text_formats_daily_stats_per_active_locale) {
    haru::game::localization::GameText text =
        haru::game::localization::GameText::loadDefault("zh-CN");
    haru::game::systems::DailyStats stats;
    stats.energy = 50;
    stats.modProgress = 10;
    stats.dependence = 2;

    HARU_EXPECT_EQ(text.formatDailyStats(stats),
                   "第 1 天  体力 50  学习 0  Mod 10  羁绊 0  依存 2");
}

HARU_TEST(game_text_loads_from_harulang_resources_instead_of_cpp_tables) {
    haru::game::localization::GameText text =
        haru::game::localization::GameText::loadFromDirectory(
            HARUFUSHI_SOURCE_DIR "/resources/localization",
            "ja-JP");

    HARU_EXPECT_EQ(text.get(haru::game::localization::TextId::GameTitle),
                   "ハルフシ・パッチ・ディペンデンシー");
    HARU_EXPECT_EQ(text.get(haru::game::localization::TextId::LanguageJapanese), "日本語");
    HARU_EXPECT_EQ(text.get(haru::game::localization::TextId::Rest), "休む");
    HARU_EXPECT_EQ(text.get(haru::game::localization::TextId::DailyBoardTitle),
                   "今日のMod計画");
}
