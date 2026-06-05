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

    HARU_EXPECT_EQ(config.title, "Harufushi Patch Dependency");
    HARU_EXPECT_EQ(config.width, 1280);
    HARU_EXPECT_EQ(config.height, 720);
}

HARU_TEST(window_event_can_describe_mouse_button_release_position) {
    using haru::engine::platform::MouseButton;
    using haru::engine::platform::WindowEvent;
    using haru::engine::platform::WindowEventKind;

    const WindowEvent event{WindowEventKind::MouseButtonReleased, 42, 64, MouseButton::Left};

    HARU_EXPECT_EQ(event.kind, WindowEventKind::MouseButtonReleased);
    HARU_EXPECT_EQ(event.x, 42);
    HARU_EXPECT_EQ(event.y, 64);
    HARU_EXPECT_EQ(event.button, MouseButton::Left);
}

HARU_TEST(window_sizing_policy_keeps_aspect_when_dragging_right_or_bottom_edges) {
    using haru::engine::platform::ResizeEdge;
    using haru::engine::platform::WindowBounds;
    using haru::engine::platform::WindowSizingPolicy;

    const WindowBounds draggedRight =
        WindowSizingPolicy::constrainAspectRatio({100, 100, 1500, 820},
                                                 ResizeEdge::Right,
                                                 16,
                                                 9);
    HARU_EXPECT_EQ(draggedRight.left, 100);
    HARU_EXPECT_EQ(draggedRight.right, 1500);
    HARU_EXPECT_EQ(draggedRight.bottom - draggedRight.top, 788);

    const WindowBounds draggedBottom =
        WindowSizingPolicy::constrainAspectRatio({100, 100, 1400, 900},
                                                 ResizeEdge::Bottom,
                                                 16,
                                                 9);
    HARU_EXPECT_EQ(draggedBottom.top, 100);
    HARU_EXPECT_EQ(draggedBottom.bottom, 900);
    HARU_EXPECT_EQ(draggedBottom.right - draggedBottom.left, 1422);
}
