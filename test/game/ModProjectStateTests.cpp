#include "support/TestHarness.hpp"

#include "game/systems/ModProjectState.hpp"

HARU_TEST(mod_project_state_starts_with_small_patch_backlog) {
    haru::game::systems::ModProjectState project;

    HARU_EXPECT_EQ(project.stats().backlog, 3);
    HARU_EXPECT_EQ(project.stats().stability, 68);
    HARU_EXPECT_EQ(project.stats().releaseReadiness, 0);
    HARU_EXPECT_EQ(project.currentEventKey(), "mod.event.feature_request");
}

HARU_TEST(mod_project_state_updates_through_modding_workflow) {
    haru::game::systems::ModProjectState project;

    project.apply(haru::game::systems::ModProjectAction::DesignFeature);
    HARU_EXPECT_EQ(project.stats().backlog, 4);
    HARU_EXPECT_EQ(project.stats().releaseReadiness, 12);
    HARU_EXPECT_EQ(project.currentEventKey(), "mod.event.regression_bug");

    project.apply(haru::game::systems::ModProjectAction::FixBug);
    HARU_EXPECT_EQ(project.stats().backlog, 3);
    HARU_EXPECT_EQ(project.stats().stability, 82);
    HARU_EXPECT_EQ(project.currentEventKey(), "mod.event.harufushi_ping");

    project.apply(haru::game::systems::ModProjectAction::PublishPreview);
    HARU_EXPECT_EQ(project.stats().releaseReadiness, 0);
    HARU_EXPECT_EQ(project.stats().feedbackHeat, 24);
    HARU_EXPECT_EQ(project.currentEventKey(), "mod.event.feedback_wave");
}

HARU_TEST(mod_project_state_clamps_values_to_playable_ranges) {
    haru::game::systems::ModProjectState project;

    for (int index = 0; index < 12; ++index) {
        project.apply(haru::game::systems::ModProjectAction::ReadComments);
    }

    HARU_EXPECT_EQ(project.stats().feedbackHeat, 100);
    HARU_EXPECT_EQ(project.stats().dependencePressure, 36);

    for (int index = 0; index < 12; ++index) {
        project.apply(haru::game::systems::ModProjectAction::FixBug);
    }

    HARU_EXPECT_EQ(project.stats().stability, 100);
    HARU_EXPECT_EQ(project.stats().backlog, 0);
}
