#include "support/TestHarness.hpp"

#include "engine/platform/Window.hpp"

HARU_TEST(window_config_rejects_empty_title_and_zero_size) {
    using haru::engine::platform::WindowConfig;

    const WindowConfig emptyTitle{"", 1280, 720};
    const WindowConfig zeroWidth{"春伏补丁依存症", 0, 720};
    const WindowConfig zeroHeight{"春伏补丁依存症", 1280, 0};
    const WindowConfig valid{"春伏补丁依存症", 1280, 720};

    HARU_EXPECT_FALSE(emptyTitle.valid());
    HARU_EXPECT_FALSE(zeroWidth.valid());
    HARU_EXPECT_FALSE(zeroHeight.valid());
    HARU_EXPECT_TRUE(valid.valid());
}

HARU_TEST(window_config_has_stable_default_game_size) {
    const auto config = haru::engine::platform::WindowConfig::defaultGameWindow();

    HARU_EXPECT_EQ(config.title, "春伏补丁依存症");
    HARU_EXPECT_EQ(config.width, 1280);
    HARU_EXPECT_EQ(config.height, 720);
}
