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

HARU_TEST(render_queue_records_text_commands) {
    haru::engine::graphics::RenderQueue queue;
    const haru::engine::graphics::Color textColor{240, 230, 220, 255};

    queue.drawText({1, 2, 80, 20}, "秋起", textColor);

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(1));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Text);
    HARU_EXPECT_EQ(queue.commands()[0].rect.x, 1);
    HARU_EXPECT_EQ(queue.commands()[0].rect.y, 2);
    HARU_EXPECT_EQ(queue.commands()[0].text, "秋起");
    HARU_EXPECT_EQ(queue.commands()[0].color, textColor);
}

HARU_TEST(render_queue_records_modern_shape_commands) {
    haru::engine::graphics::RenderQueue queue;
    const haru::engine::graphics::Color pink{255, 183, 205, 255};
    const haru::engine::graphics::Color blue{185, 226, 232, 255};
    const haru::engine::graphics::Color ink{67, 76, 104, 255};

    queue.fillRoundedRect({10, 20, 120, 48}, pink, 18);
    queue.fillEllipse({24, 32, 64, 40}, blue);
    queue.strokeRect({6, 8, 144, 72}, ink, 3);
    queue.fillVerticalGradient({0, 0, 80, 60}, blue, pink);

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(4));
    HARU_EXPECT_EQ(queue.commands()[0].kind,
                   haru::engine::graphics::DrawCommandKind::FillRoundedRect);
    HARU_EXPECT_EQ(queue.commands()[0].radius, 18);
    HARU_EXPECT_EQ(queue.commands()[1].kind,
                   haru::engine::graphics::DrawCommandKind::FillEllipse);
    HARU_EXPECT_EQ(queue.commands()[2].kind,
                   haru::engine::graphics::DrawCommandKind::StrokeRect);
    HARU_EXPECT_EQ(queue.commands()[2].thickness, 3);
    HARU_EXPECT_EQ(queue.commands()[3].kind,
                   haru::engine::graphics::DrawCommandKind::FillVerticalGradient);
    HARU_EXPECT_EQ(queue.commands()[3].secondaryColor, pink);
}
