#include "support/TestHarness.hpp"

#include "game/scenes/EngineSplashScene.hpp"

#include <cstddef>

HARU_TEST(engine_splash_scene_starts_active_and_completes_after_duration) {
    haru::game::scenes::EngineSplashScene splash(1.0);

    HARU_EXPECT_TRUE(splash.active());
    HARU_EXPECT_FALSE(splash.complete());

    splash.update(0.5);
    HARU_EXPECT_TRUE(splash.active());
    HARU_EXPECT_FALSE(splash.complete());

    splash.update(0.5);
    HARU_EXPECT_FALSE(splash.active());
    HARU_EXPECT_TRUE(splash.complete());
}

HARU_TEST(engine_splash_scene_emits_logo_progress_and_text_commands) {
    haru::game::scenes::EngineSplashScene splash(1.0);
    haru::engine::graphics::RenderQueue queue;

    splash.render(queue, {1280, 720});

    HARU_EXPECT_TRUE(queue.commands().size() >= static_cast<std::size_t>(4));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Clear);
    HARU_EXPECT_EQ(queue.commands()[queue.commands().size() - 1].kind,
                   haru::engine::graphics::DrawCommandKind::Text);
}
