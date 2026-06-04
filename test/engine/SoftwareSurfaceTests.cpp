#include "support/TestHarness.hpp"

#include "engine/graphics/SoftwareSurface.hpp"

HARU_TEST(software_surface_clips_filled_rectangles_to_surface_bounds) {
    haru::engine::graphics::SoftwareSurface surface(4, 4);
    const haru::engine::graphics::Color black{0, 0, 0, 255};
    const haru::engine::graphics::Color accent{200, 10, 20, 255};
    surface.clear(black);
    surface.fillRect({2, 2, 4, 4}, accent);

    HARU_EXPECT_EQ(surface.pixelAt(3, 3), accent);
    HARU_EXPECT_EQ(surface.pixelAt(1, 1), black);
}

HARU_TEST(software_surface_draws_render_queue_commands) {
    haru::engine::graphics::RenderQueue queue;
    const haru::engine::graphics::Color background{1, 2, 3, 255};
    const haru::engine::graphics::Color foreground{9, 8, 7, 255};
    queue.clear(background);
    queue.fillRect({0, 0, 2, 2}, foreground);

    haru::engine::graphics::SoftwareSurface surface(3, 3);
    surface.draw(queue);

    HARU_EXPECT_EQ(surface.pixelAt(0, 0), foreground);
    HARU_EXPECT_EQ(surface.pixelAt(2, 2), background);
}

HARU_TEST(software_surface_draws_text_placeholder_inside_bounds) {
    haru::engine::graphics::RenderQueue queue;
    const haru::engine::graphics::Color background{0, 0, 0, 255};
    const haru::engine::graphics::Color textColor{220, 210, 200, 255};
    queue.clear(background);
    queue.drawText({2, 2, 20, 12}, "Hi", textColor);

    haru::engine::graphics::SoftwareSurface surface(32, 20);
    surface.draw(queue);

    HARU_EXPECT_EQ(surface.pixelAt(2, 2), textColor);
    HARU_EXPECT_EQ(surface.pixelAt(0, 0), background);
}

HARU_TEST(software_surface_can_skip_text_placeholder_for_native_text_backends) {
    haru::engine::graphics::RenderQueue queue;
    const haru::engine::graphics::Color background{0, 0, 0, 255};
    const haru::engine::graphics::Color textColor{220, 210, 200, 255};
    queue.clear(background);
    queue.drawText({2, 2, 20, 12}, "Hi", textColor);

    haru::engine::graphics::SoftwareSurface surface(32, 20);
    surface.draw(queue, haru::engine::graphics::TextRasterization::Skip);

    HARU_EXPECT_EQ(surface.pixelAt(2, 2), background);
    HARU_EXPECT_EQ(surface.pixelAt(0, 0), background);
}

HARU_TEST(software_surface_draws_rounded_shapes_and_gradients) {
    haru::engine::graphics::RenderQueue queue;
    const haru::engine::graphics::Color background{250, 248, 242, 255};
    const haru::engine::graphics::Color pink{255, 183, 205, 255};
    const haru::engine::graphics::Color blue{185, 226, 232, 255};
    const haru::engine::graphics::Color ink{67, 76, 104, 255};
    queue.clear(background);
    queue.fillVerticalGradient({0, 0, 20, 20}, blue, pink);
    queue.fillRoundedRect({2, 2, 12, 12}, pink, 4);
    queue.fillEllipse({8, 8, 8, 8}, blue);
    queue.strokeRect({1, 1, 18, 18}, ink, 2);

    haru::engine::graphics::SoftwareSurface surface(20, 20);
    surface.draw(queue);

    HARU_EXPECT_EQ(surface.pixelAt(10, 10), blue);
    HARU_EXPECT_EQ(surface.pixelAt(0, 0), blue);
    HARU_EXPECT_EQ(surface.pixelAt(1, 1), ink);
    HARU_EXPECT_EQ(surface.pixelAt(10, 19), pink);
}

HARU_TEST(software_surface_draws_filled_polygons) {
    haru::engine::graphics::RenderQueue queue;
    const haru::engine::graphics::Color background{0, 0, 0, 255};
    const haru::engine::graphics::Color blue{11, 119, 155, 230};
    queue.clear(background);
    queue.fillPolygon({{2, 2}, {14, 2}, {10, 14}, {2, 14}}, blue);

    haru::engine::graphics::SoftwareSurface surface(18, 18);
    surface.draw(queue);

    HARU_EXPECT_EQ(surface.pixelAt(6, 6), blue);
    HARU_EXPECT_EQ(surface.pixelAt(16, 16), background);
}
