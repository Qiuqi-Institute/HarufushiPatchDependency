#include "support/TestHarness.hpp"

#include "game/localization/GameText.hpp"
#include "game/scenes/TitleScene.hpp"
#include "game/systems/DailyDialogueScript.hpp"

#include <optional>
#include <sstream>
#include <string>

namespace {

bool hasText(const haru::engine::graphics::RenderQueue& queue, const std::string& text) {
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::Text &&
            command.text == text) {
            return true;
        }
    }

    return false;
}

bool hasFillColor(const haru::engine::graphics::RenderQueue& queue,
                  haru::engine::graphics::Color color) {
    for (const auto& command : queue.commands()) {
        if ((command.kind == haru::engine::graphics::DrawCommandKind::FillRect ||
             command.kind == haru::engine::graphics::DrawCommandKind::FillRoundedRect ||
             command.kind == haru::engine::graphics::DrawCommandKind::FillEllipse ||
             command.kind == haru::engine::graphics::DrawCommandKind::FillVerticalGradient) &&
            command.color == color) {
            return true;
        }
    }

    return false;
}

bool hasCommandKind(const haru::engine::graphics::RenderQueue& queue,
                    haru::engine::graphics::DrawCommandKind kind) {
    for (const auto& command : queue.commands()) {
        if (command.kind == kind) {
            return true;
        }
    }

    return false;
}

const haru::engine::graphics::DrawCommand* findText(
    const haru::engine::graphics::RenderQueue& queue,
    const std::string& text) {
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::Text &&
            command.text == text) {
            return &command;
        }
    }

    return nullptr;
}

std::string statsLine(const haru::game::systems::DailyStats& stats) {
    std::ostringstream line;
    line << "Day " << stats.day << "  Energy " << stats.energy << "  Study "
         << stats.studyFocus << "  Mod " << stats.modProgress << "  Bond "
         << stats.harufushiBond << "  Dependence " << stats.dependence;
    return line.str();
}

} // namespace

HARU_TEST(title_scene_renders_title_and_daily_loop_actions) {
    haru::game::scenes::TitleScene titleScene;
    haru::engine::graphics::RenderQueue queue;

    titleScene.render(queue, {1280, 720});

    HARU_EXPECT_TRUE(queue.commands().size() >= 12);
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Clear);
    HARU_EXPECT_TRUE(hasText(queue, "Harufushi Patch Dependency"));
    HARU_EXPECT_TRUE(hasText(queue, "Study"));
    HARU_EXPECT_TRUE(hasText(queue, "Modding"));
    HARU_EXPECT_TRUE(hasText(queue, "Harufushi"));
    HARU_EXPECT_TRUE(hasText(queue, "Rest"));
    HARU_EXPECT_TRUE(hasText(queue, "Today's Mod Plan"));
    HARU_EXPECT_TRUE(hasText(queue, "Plan: review notes, then one verifiable patch."));
    HARU_EXPECT_TRUE(findText(queue, "Harufushi Patch Dependency")->rect.height <= 56);
}

HARU_TEST(title_scene_uses_light_visual_novel_daily_layout) {
    haru::game::scenes::TitleScene titleScene;
    haru::game::systems::DailyLoopState state;
    haru::engine::graphics::RenderQueue queue;

    titleScene.render(queue, {1280, 720}, state.stats());

    HARU_EXPECT_EQ(queue.commands()[0].color,
                   (haru::engine::graphics::Color{250, 248, 242, 255}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {255, 252, 248, 255}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {185, 226, 232, 255}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {255, 210, 222, 255}));
    HARU_EXPECT_TRUE(hasCommandKind(queue,
                                    haru::engine::graphics::DrawCommandKind::
                                        FillRoundedRect));
    HARU_EXPECT_TRUE(hasCommandKind(queue,
                                    haru::engine::graphics::DrawCommandKind::
                                        FillVerticalGradient));
    HARU_EXPECT_TRUE(hasCommandKind(queue,
                                    haru::engine::graphics::DrawCommandKind::
                                        StrokeRect));
    HARU_EXPECT_TRUE(findText(queue, statsLine(state.stats()))->rect.y >= 560);
}

HARU_TEST(title_scene_maps_button_points_to_daily_actions) {
    haru::game::scenes::TitleScene titleScene;
    haru::game::systems::DailyStats activeStats;
    activeStats.day = 2;
    activeStats.energy = 80;
    activeStats.studyFocus = 12;

    const std::optional<haru::game::systems::DailyAction> study =
        titleScene.actionAt({120, 250}, {1280, 720}, activeStats);
    const std::optional<haru::game::systems::DailyAction> modding =
        titleScene.actionAt({120, 315}, {1280, 720}, activeStats);
    const std::optional<haru::game::systems::DailyAction> harufushi =
        titleScene.actionAt({120, 380}, {1280, 720}, activeStats);
    const std::optional<haru::game::systems::DailyAction> rest =
        titleScene.actionAt({120, 444}, {1280, 720}, activeStats);
    const std::optional<haru::game::systems::DailyAction> blank =
        titleScene.actionAt({16, 16}, {1280, 720}, activeStats);

    HARU_EXPECT_TRUE(study.has_value());
    HARU_EXPECT_TRUE(modding.has_value());
    HARU_EXPECT_TRUE(harufushi.has_value());
    HARU_EXPECT_TRUE(rest.has_value());
    HARU_EXPECT_FALSE(blank.has_value());
    HARU_EXPECT_EQ(*study, haru::game::systems::DailyAction::Study);
    HARU_EXPECT_EQ(*modding, haru::game::systems::DailyAction::Modding);
    HARU_EXPECT_EQ(*harufushi, haru::game::systems::DailyAction::SpendTimeWithHarufushi);
    HARU_EXPECT_EQ(*rest, haru::game::systems::DailyAction::Rest);
}

HARU_TEST(title_scene_does_not_map_locked_daily_actions) {
    haru::game::scenes::TitleScene titleScene;
    const haru::game::systems::DailyStats firstDayStats;

    const std::optional<haru::game::systems::DailyAction> study =
        titleScene.actionAt({120, 250}, {1280, 720}, firstDayStats);
    const std::optional<haru::game::systems::DailyAction> modding =
        titleScene.actionAt({120, 315}, {1280, 720}, firstDayStats);
    const std::optional<haru::game::systems::DailyAction> harufushi =
        titleScene.actionAt({120, 380}, {1280, 720}, firstDayStats);
    const std::optional<haru::game::systems::DailyAction> rest =
        titleScene.actionAt({120, 444}, {1280, 720}, firstDayStats);

    HARU_EXPECT_TRUE(study.has_value());
    HARU_EXPECT_FALSE(modding.has_value());
    HARU_EXPECT_FALSE(harufushi.has_value());
    HARU_EXPECT_TRUE(rest.has_value());
}

HARU_TEST(title_scene_renders_and_maps_return_home_action) {
    haru::game::scenes::TitleScene titleScene;
    haru::engine::graphics::RenderQueue queue;

    titleScene.render(queue, {1280, 720});

    HARU_EXPECT_TRUE(hasText(queue, "Home"));
    const auto action = titleScene.navigationActionAt({1112, 88}, {1280, 720});
    HARU_EXPECT_TRUE(action.has_value());
    HARU_EXPECT_EQ(*action, haru::game::scenes::TitleNavigationAction::ReturnHome);
}

HARU_TEST(title_scene_renders_daily_loop_stats_feedback) {
    haru::game::scenes::TitleScene titleScene;
    haru::game::systems::DailyLoopState state;
    haru::engine::graphics::RenderQueue queue;

    state.apply(haru::game::systems::DailyAction::Modding);
    titleScene.render(queue, {1280, 720}, state.stats());

    HARU_EXPECT_TRUE(hasText(queue, statsLine(state.stats())));
}

HARU_TEST(title_scene_renders_daily_activity_dialogue_panel) {
    haru::game::scenes::TitleScene titleScene;
    haru::game::systems::DailyDialogueEntry dialogue;
    dialogue.speaker = "Harufushi ping";
    dialogue.lines = {"Your patch queue blinked again.",
                      "Pick one task before opening another editor.",
                      "A branch opened because the build finally stabilized."};
    haru::engine::graphics::RenderQueue queue;

    titleScene.render(queue, {1280, 720}, {}, &dialogue);

    const auto* speaker = findText(queue, "Harufushi ping");
    const auto* firstLine = findText(queue, "Your patch queue blinked again.");
    const auto* secondLine = findText(queue, "Pick one task before opening another editor.");
    const auto* thirdLine =
        findText(queue, "A branch opened because the build finally stabilized.");
    HARU_EXPECT_TRUE(speaker != nullptr);
    HARU_EXPECT_TRUE(firstLine != nullptr);
    HARU_EXPECT_TRUE(secondLine != nullptr);
    HARU_EXPECT_TRUE(thirdLine != nullptr);
    HARU_EXPECT_TRUE(speaker->rect.y >= 540);
    HARU_EXPECT_TRUE(firstLine->rect.y > speaker->rect.y);
    HARU_EXPECT_TRUE(secondLine->rect.y > firstLine->rect.y);
    HARU_EXPECT_TRUE(thirdLine->rect.y > secondLine->rect.y);
}

HARU_TEST(title_scene_renders_later_dialogue_pages) {
    haru::game::scenes::TitleScene titleScene;
    haru::game::systems::DailyDialogueEntry dialogue;
    dialogue.speaker = "Harufushi";
    dialogue.lines = {"Page one line one",
                      "Page one line two",
                      "Page one line three",
                      "Page two line one",
                      "Page two line two",
                      "Page two line three"};
    haru::engine::graphics::RenderQueue queue;

    titleScene.render(queue, {1280, 720}, {}, &dialogue, 1);

    HARU_EXPECT_FALSE(hasText(queue, "Page one line one"));
    HARU_EXPECT_TRUE(hasText(queue, "Page two line one"));
    HARU_EXPECT_TRUE(hasText(queue, "Page two line two"));
    HARU_EXPECT_TRUE(hasText(queue, "Page two line three"));
    HARU_EXPECT_TRUE(hasText(queue, "2/2"));
}

HARU_TEST(title_scene_maps_dialogue_panel_to_advance_action) {
    haru::game::scenes::TitleScene titleScene;

    HARU_EXPECT_TRUE(titleScene.dialogueAdvanceAt({640, 610}, {1280, 720}));
    HARU_EXPECT_FALSE(titleScene.dialogueAdvanceAt({640, 120}, {1280, 720}));
    HARU_EXPECT_FALSE(titleScene.dialogueAdvanceAt({640, 610}, {0, 720}));
}

HARU_TEST(title_scene_uses_game_text_localization) {
    haru::game::localization::GameText text =
        haru::game::localization::GameText::loadDefault("zh-CN");
    haru::game::scenes::TitleScene titleScene(text);
    haru::engine::graphics::RenderQueue queue;

    titleScene.render(queue, {1280, 720});

    HARU_EXPECT_TRUE(hasText(queue, "春伏补丁依存症"));
    HARU_EXPECT_TRUE(hasText(queue, "学习"));
    HARU_EXPECT_TRUE(hasText(queue, "写 Mod"));
    HARU_EXPECT_TRUE(hasText(queue, "春伏"));
    HARU_EXPECT_TRUE(hasText(queue, "休息"));
    HARU_EXPECT_TRUE(hasText(queue, "今日 Mod 计划"));
}
