#include "support/TestHarness.hpp"

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
