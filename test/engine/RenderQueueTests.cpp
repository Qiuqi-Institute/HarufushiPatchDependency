#include "support/TestHarness.hpp"

#include "engine/graphics/RenderQueue.hpp"

#include <cstddef>

HARU_TEST(render_queue_records_clear_and_rect_commands_in_order) {
    haru::engine::graphics::RenderQueue queue;
    const haru::engine::graphics::Color clearColor{10, 20, 30, 255};
    const haru::engine::graphics::Color rectColor{1, 2, 3, 255};
    queue.clear(clearColor);
    queue.fillRect({4, 5, 6, 7}, rectColor);

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(2));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Clear);
    HARU_EXPECT_EQ(queue.commands()[0].color, clearColor);
    HARU_EXPECT_EQ(queue.commands()[1].kind, haru::engine::graphics::DrawCommandKind::FillRect);
    HARU_EXPECT_EQ(queue.commands()[1].rect.x, 4);
    HARU_EXPECT_EQ(queue.commands()[1].rect.y, 5);
}

HARU_TEST(render_queue_clear_commands_removes_previous_commands) {
    haru::engine::graphics::RenderQueue queue;
    queue.clear({10, 20, 30, 255});
    queue.fillRect({4, 5, 6, 7}, {1, 2, 3, 255});

    queue.reset();

    HARU_EXPECT_TRUE(queue.commands().empty());
}
