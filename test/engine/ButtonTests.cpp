#include "support/TestHarness.hpp"

#include "engine/ui/Button.hpp"

#include <cstddef>

HARU_TEST(button_renders_background_and_text) {
    haru::engine::ui::Button button({10, 20, 100, 32}, "写 Mod");
    haru::engine::graphics::RenderQueue queue;

    button.render(queue);

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(2));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::FillRect);
    HARU_EXPECT_EQ(queue.commands()[0].rect.x, 10);
    HARU_EXPECT_EQ(queue.commands()[1].kind, haru::engine::graphics::DrawCommandKind::Text);
    HARU_EXPECT_EQ(queue.commands()[1].text, "写 Mod");
}

HARU_TEST(button_contains_points_inside_bounds_only) {
    haru::engine::ui::Button button({10, 20, 100, 32}, "写 Mod");

    HARU_EXPECT_TRUE(button.contains({20, 30}));
    HARU_EXPECT_FALSE(button.contains({1, 1}));
    HARU_EXPECT_FALSE(button.contains({110, 52}));
}
