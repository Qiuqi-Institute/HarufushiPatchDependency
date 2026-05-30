#include "support/TestHarness.hpp"

#include "game/systems/DailyLoopState.hpp"

HARU_TEST(daily_loop_state_starts_with_akioki_baseline_stats) {
    const haru::game::systems::DailyLoopState state;

    HARU_EXPECT_EQ(state.stats().day, 1);
    HARU_EXPECT_EQ(state.stats().energy, 70);
    HARU_EXPECT_EQ(state.stats().studyFocus, 0);
    HARU_EXPECT_EQ(state.stats().modProgress, 0);
    HARU_EXPECT_EQ(state.stats().harufushiBond, 0);
    HARU_EXPECT_EQ(state.stats().dependence, 0);
}

HARU_TEST(daily_loop_state_applies_study_and_modding_actions) {
    haru::game::systems::DailyLoopState state;

    state.apply(haru::game::systems::DailyAction::Study);
    HARU_EXPECT_EQ(state.stats().energy, 55);
    HARU_EXPECT_EQ(state.stats().studyFocus, 12);

    state.apply(haru::game::systems::DailyAction::Modding);
    HARU_EXPECT_EQ(state.stats().energy, 35);
    HARU_EXPECT_EQ(state.stats().modProgress, 10);
    HARU_EXPECT_EQ(state.stats().dependence, 2);
}

HARU_TEST(daily_loop_state_applies_harufushi_and_rest_actions) {
    haru::game::systems::DailyLoopState state;

    state.apply(haru::game::systems::DailyAction::SpendTimeWithHarufushi);
    HARU_EXPECT_EQ(state.stats().energy, 60);
    HARU_EXPECT_EQ(state.stats().harufushiBond, 15);
    HARU_EXPECT_EQ(state.stats().dependence, 5);

    state.apply(haru::game::systems::DailyAction::Rest);
    HARU_EXPECT_EQ(state.stats().day, 2);
    HARU_EXPECT_EQ(state.stats().energy, 85);
}
