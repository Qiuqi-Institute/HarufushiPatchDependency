#include "support/TestHarness.hpp"

#include "engine/core/AsyncStartupTask.hpp"
#include "game/systems/HomeStartupPreparation.hpp"

#include <string>
#include <vector>

HARU_TEST(home_startup_preparation_formats_save_summaries_before_home_render) {
    std::vector<haru::game::systems::GameSave> saves;
    haru::game::systems::GameSave first;
    first.id = "save-a";
    first.stats.day = 3;
    first.stats.modProgress = 20;
    saves.push_back(first);
    haru::game::systems::GameSave second;
    second.id = "save-b";
    second.stats.day = 8;
    second.stats.modProgress = 71;
    saves.push_back(second);

    haru::game::systems::HomeStartupPreparation preparation;

    const bool started = preparation.start(saves, "Day", "Mod");
    const std::vector<std::string> summaries = preparation.saveSummaries();

    HARU_EXPECT_TRUE(started);
    HARU_EXPECT_TRUE(preparation.ready());
    HARU_EXPECT_EQ(summaries.size(), static_cast<std::size_t>(2));
    HARU_EXPECT_EQ(summaries[0], "Save 1  Day 3  Mod 20");
    HARU_EXPECT_EQ(summaries[1], "Save 2  Day 8  Mod 71");
}

HARU_TEST(home_startup_preparation_keeps_game_work_separate_from_engine_startup_task) {
    haru::game::systems::HomeStartupPreparation preparation;
    haru::engine::core::AsyncStartupTask engineTask;

    HARU_EXPECT_TRUE(engineTask.start([]() {}));
    engineTask.wait();

    HARU_EXPECT_FALSE(preparation.started());
    HARU_EXPECT_TRUE(preparation.saveSummaries().empty());
}
