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

HARU_TEST(engine_splash_scene_emits_white_background_teal_icon_and_engine_name) {
    haru::game::scenes::EngineSplashScene splash(1.0);
    haru::engine::graphics::RenderQueue queue;
    const haru::engine::graphics::Color pureWhite{255, 255, 255, 255};
    const haru::engine::graphics::Color splashTeal{11, 119, 155, 255};

    splash.render(queue, {1280, 720});

    HARU_EXPECT_TRUE(queue.commands().size() >= static_cast<std::size_t>(18));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Clear);
    HARU_EXPECT_EQ(queue.commands()[0].color, pureWhite);

    for (std::size_t index = 1; index + 1 < queue.commands().size(); ++index) {
        HARU_EXPECT_EQ(queue.commands()[index].kind,
                       haru::engine::graphics::DrawCommandKind::FillRect);
        HARU_EXPECT_EQ(queue.commands()[index].color, splashTeal);
    }

    HARU_EXPECT_EQ(queue.commands()[queue.commands().size() - 1].kind,
                   haru::engine::graphics::DrawCommandKind::Text);
    HARU_EXPECT_EQ(queue.commands()[queue.commands().size() - 1].text, "Harufushi Frame");
    HARU_EXPECT_EQ(queue.commands()[queue.commands().size() - 1].color, splashTeal);
}
