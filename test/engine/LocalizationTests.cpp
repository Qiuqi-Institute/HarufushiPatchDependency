#include "support/TestHarness.hpp"

#include "engine/localization/HaruLanguageDocument.hpp"
#include "engine/localization/LocaleRegistry.hpp"

HARU_TEST(locale_registry_accepts_dynamic_locales_without_hardcoded_count) {
    haru::engine::localization::LocaleRegistry registry;

    HARU_EXPECT_TRUE(registry.add("zh-CN"));
    HARU_EXPECT_TRUE(registry.add("en-US"));
    HARU_EXPECT_TRUE(registry.add("ja-JP"));
    HARU_EXPECT_TRUE(registry.add("ko-KR"));
    HARU_EXPECT_FALSE(registry.add("zh-CN"));

    HARU_EXPECT_EQ(registry.count(), static_cast<std::size_t>(4));
    HARU_EXPECT_TRUE(registry.contains("ko-KR"));
}

HARU_TEST(locale_registry_rejects_empty_locale_tags) {
    haru::engine::localization::LocaleRegistry registry;

    HARU_EXPECT_FALSE(registry.add(""));
    HARU_EXPECT_FALSE(registry.contains(""));
    HARU_EXPECT_EQ(registry.count(), static_cast<std::size_t>(0));
}

HARU_TEST(haru_language_document_parses_proto_like_text_entries) {
    const auto document = haru::engine::localization::HaruLanguageDocument::parse(
        "harulang v1\n"
        "package harufushi.game\n"
        "locale ja-JP\n"
        "text game.title = \"ハルフシ・パッチ・ディペンデンシー\"\n"
        "text menu.new_game = \"ニューゲーム\"\n");

    HARU_EXPECT_EQ(document.packageName(), "harufushi.game");
    HARU_EXPECT_EQ(document.locale(), "ja-JP");
    HARU_EXPECT_TRUE(document.contains("game.title"));
    HARU_EXPECT_EQ(document.get("game.title"), "ハルフシ・パッチ・ディペンデンシー");
    HARU_EXPECT_EQ(document.get("menu.new_game"), "ニューゲーム");
}

HARU_TEST(haru_language_document_rejects_missing_header) {
    bool rejected = false;

    try {
        haru::engine::localization::HaruLanguageDocument::parse(
            "locale en-US\n"
            "text game.title = \"Harufushi Patch Dependency\"\n");
    } catch (const std::exception&) {
        rejected = true;
    }

    HARU_EXPECT_TRUE(rejected);
}
