#include "support/TestHarness.hpp"

#include "engine/graphics/RenderQueue.hpp"

#include <cstddef>
#include <vector>

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
    HARU_EXPECT_EQ(queue.commands()[0].textRole,
                   haru::engine::graphics::TextRole::Default);
}

HARU_TEST(render_queue_records_text_font_roles) {
    haru::engine::graphics::RenderQueue queue;

    queue.drawText({4, 8, 160, 32},
                   "春伏",
                   {67, 76, 104, 255},
                   haru::engine::graphics::TextRole::ZenMaruBlack);

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(1));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Text);
    HARU_EXPECT_EQ(queue.commands()[0].textRole,
                   haru::engine::graphics::TextRole::ZenMaruBlack);
}

HARU_TEST(render_queue_records_text_font_scale_independent_from_bounds) {
    haru::engine::graphics::RenderQueue queue;

    queue.drawText({4, 8, 160, 42},
                   "开始游戏",
                   {255, 255, 251, 255},
                   haru::engine::graphics::TextRole::ZenMaruBlack,
                   67);

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(1));
    HARU_EXPECT_EQ(queue.commands()[0].rect.height, 42);
    HARU_EXPECT_EQ(queue.commands()[0].fontScalePercent, 67);
}

HARU_TEST(render_queue_records_image_commands) {
    haru::engine::graphics::RenderQueue queue;

    queue.drawImage({0, 0, 1280, 720}, "resources/images/backgrounds/home_chunfu.png");

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(1));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Image);
    HARU_EXPECT_EQ(queue.commands()[0].rect.x, 0);
    HARU_EXPECT_EQ(queue.commands()[0].rect.width, 1280);
    HARU_EXPECT_EQ(queue.commands()[0].text, "resources/images/backgrounds/home_chunfu.png");
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

HARU_TEST(render_queue_records_filled_polygon_commands) {
    haru::engine::graphics::RenderQueue queue;
    const haru::engine::graphics::Color blue{11, 119, 155, 230};

    queue.fillPolygon({{10, 12}, {30, 16}, {24, 46}, {4, 42}}, blue);

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(1));
    HARU_EXPECT_EQ(queue.commands()[0].kind,
                   haru::engine::graphics::DrawCommandKind::FillPolygon);
    HARU_EXPECT_EQ(queue.commands()[0].color, blue);
    HARU_EXPECT_EQ(queue.commands()[0].points.size(), static_cast<std::size_t>(4));
    HARU_EXPECT_EQ(queue.commands()[0].points[1].x, 30);
    HARU_EXPECT_EQ(queue.commands()[0].points[3].y, 42);
}
