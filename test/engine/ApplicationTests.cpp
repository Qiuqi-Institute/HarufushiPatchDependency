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
