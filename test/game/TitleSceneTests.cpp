#include "support/TestHarness.hpp"

#include "game/scenes/TitleScene.hpp"

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
}
