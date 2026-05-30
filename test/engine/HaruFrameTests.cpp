#include "support/TestHarness.hpp"

#include <HaruFrame>

#include <cstddef>

HARU_TEST(haru_frame_renders_engine_opening_before_visual_content) {
    haru::engine::HaruFrame frame(1.0);
    haru::engine::graphics::RenderQueue queue;
    bool contentRendered = false;

    const bool openingRendered =
        frame.render(queue, {1280, 720}, 1.0, [&](haru::engine::graphics::RenderQueue&) {
            contentRendered = true;
        });

    HARU_EXPECT_TRUE(openingRendered);
    HARU_EXPECT_FALSE(contentRendered);
    HARU_EXPECT_TRUE(queue.commands().size() >= static_cast<std::size_t>(18));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Clear);
    HARU_EXPECT_EQ(queue.commands()[0].color,
                   (haru::engine::graphics::Color{255, 255, 255, 255}));
    HARU_EXPECT_EQ(queue.commands()[queue.commands().size() - 1].text, "Harufushi Frame");
    HARU_EXPECT_EQ(queue.commands()[queue.commands().size() - 1].color,
                   (haru::engine::graphics::Color{11, 119, 155, 255}));
}

HARU_TEST(haru_frame_renders_visual_content_after_opening_completes) {
    haru::engine::HaruFrame frame(0.5);
    haru::engine::graphics::RenderQueue queue;

    frame.render(queue, {1280, 720}, 0.5, [](haru::engine::graphics::RenderQueue&) {});
    queue.reset();

    bool contentRendered = false;
    const bool openingRendered =
        frame.render(queue, {1280, 720}, 0.016, [&](haru::engine::graphics::RenderQueue& target) {
            contentRendered = true;
            target.clear({1, 2, 3, 255});
        });

    HARU_EXPECT_FALSE(openingRendered);
    HARU_EXPECT_TRUE(contentRendered);
    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(1));
    HARU_EXPECT_EQ(queue.commands()[0].color, (haru::engine::graphics::Color{1, 2, 3, 255}));
}
