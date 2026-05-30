#include "support/TestHarness.hpp"

#include "engine/core/Application.hpp"
#include "game/systems/HarufushiGame.hpp"

HARU_TEST(application_runs_game_runtime_once) {
    haru::game::HarufushiGame game;
    haru::engine::core::Application app({"春伏补丁依存症", "0.0.1"});

    const int exitCode = app.run(game);

    HARU_EXPECT_EQ(exitCode, 0);
    HARU_EXPECT_EQ(game.startCount(), 1);
    HARU_EXPECT_EQ(game.lastBootLine(), "Harufushi Patch Dependency bootstrap");
}

HARU_TEST(application_runs_until_frame_callback_stops) {
    haru::game::HarufushiGame game;
    haru::engine::core::Application app({"春伏补丁依存症", "0.0.1"});
    int frames = 0;

    const int exitCode = app.run(game, [&](const haru::engine::core::FrameContext&) {
        ++frames;
        return frames == 2 ? haru::engine::core::LoopDecision::Stop
                           : haru::engine::core::LoopDecision::Continue;
    });

    HARU_EXPECT_EQ(exitCode, 0);
    HARU_EXPECT_EQ(frames, 2);
    HARU_EXPECT_EQ(game.startCount(), 1);
}
