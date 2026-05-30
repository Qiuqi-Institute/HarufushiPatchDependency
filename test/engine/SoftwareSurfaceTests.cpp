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
