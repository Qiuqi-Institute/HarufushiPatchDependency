#include "support/TestHarness.hpp"

#include "engine/ui/UiNode.hpp"

#include <cstddef>

HARU_TEST(ui_node_emits_background_rects_with_child_offsets) {
    using namespace haru::engine;

    ui::UiNode root({10, 20, 100, 80}, {20, 20, 24, 255});
    root.addChild(ui::UiNode({5, 6, 30, 12}, {180, 80, 120, 255}));

    graphics::RenderQueue queue;
    root.render(queue);

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(2));
    HARU_EXPECT_EQ(queue.commands()[0].rect.x, 10);
    HARU_EXPECT_EQ(queue.commands()[0].rect.y, 20);
    HARU_EXPECT_EQ(queue.commands()[1].rect.x, 15);
    HARU_EXPECT_EQ(queue.commands()[1].rect.y, 26);
}

HARU_TEST(ui_node_emits_text_after_background) {
    using namespace haru::engine;

    const graphics::Color textColor{240, 230, 220, 255};
    ui::UiNode node({10, 20, 100, 80}, {20, 20, 24, 255});
    node.setText("春伏", textColor);

    graphics::RenderQueue queue;
    node.render(queue);

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(2));
    HARU_EXPECT_EQ(queue.commands()[0].kind, graphics::DrawCommandKind::FillRect);
    HARU_EXPECT_EQ(queue.commands()[1].kind, graphics::DrawCommandKind::Text);
    HARU_EXPECT_EQ(queue.commands()[1].text, "春伏");
    HARU_EXPECT_EQ(queue.commands()[1].color, textColor);
}
