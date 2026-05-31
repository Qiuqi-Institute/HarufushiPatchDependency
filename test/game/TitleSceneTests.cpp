#include "support/TestHarness.hpp"

#include "game/localization/GameText.hpp"
#include "game/scenes/TitleScene.hpp"

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
    HARU_EXPECT_TRUE(findText(queue, "Harufushi Patch Dependency")->rect.height <= 48);
}

HARU_TEST(title_scene_maps_button_points_to_daily_actions) {
    haru::game::scenes::TitleScene titleScene;

    const std::optional<haru::game::systems::DailyAction> study =
        titleScene.actionAt({120, 250}, {1280, 720});
    const std::optional<haru::game::systems::DailyAction> modding =
        titleScene.actionAt({120, 315}, {1280, 720});
    const std::optional<haru::game::systems::DailyAction> harufushi =
        titleScene.actionAt({120, 380}, {1280, 720});
    const std::optional<haru::game::systems::DailyAction> blank =
        titleScene.actionAt({16, 16}, {1280, 720});

    HARU_EXPECT_TRUE(study.has_value());
    HARU_EXPECT_TRUE(modding.has_value());
    HARU_EXPECT_TRUE(harufushi.has_value());
    HARU_EXPECT_FALSE(blank.has_value());
    HARU_EXPECT_EQ(*study, haru::game::systems::DailyAction::Study);
    HARU_EXPECT_EQ(*modding, haru::game::systems::DailyAction::Modding);
    HARU_EXPECT_EQ(*harufushi, haru::game::systems::DailyAction::SpendTimeWithHarufushi);
}

HARU_TEST(title_scene_renders_daily_loop_stats_feedback) {
    haru::game::scenes::TitleScene titleScene;
    haru::game::systems::DailyLoopState state;
    haru::engine::graphics::RenderQueue queue;

    state.apply(haru::game::systems::DailyAction::Modding);
    titleScene.render(queue, {1280, 720}, state.stats());

    HARU_EXPECT_TRUE(hasText(queue, statsLine(state.stats())));
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
}
