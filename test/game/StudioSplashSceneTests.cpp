#include "support/TestHarness.hpp"

#include "game/scenes/StudioSplashScene.hpp"

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

std::size_t countProjectionRects(const haru::engine::graphics::RenderQueue& queue) {
    std::size_t count = 0;
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::FillRect &&
            command.color.a == 255 &&
            command.color.b >= 160) {
            ++count;
        }
    }

    return count;
}

} // namespace

HARU_TEST(studio_splash_starts_active_and_completes_after_duration) {
    haru::game::scenes::StudioSplashScene splash(1.0);

    HARU_EXPECT_TRUE(splash.active());
    splash.update(0.4);
    HARU_EXPECT_TRUE(splash.active());
    splash.update(0.6);
    HARU_EXPECT_FALSE(splash.active());
}

HARU_TEST(studio_splash_renders_qiuqi_institute_hologram_pig) {
    haru::game::scenes::StudioSplashScene splash(2.0);
    haru::engine::graphics::RenderQueue queue;

    splash.update(0.5);
    splash.render(queue, {1280, 720});

    HARU_EXPECT_TRUE(queue.commands().size() >= static_cast<std::size_t>(24));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Clear);
    HARU_EXPECT_EQ(queue.commands()[0].color,
                   (haru::engine::graphics::Color{255, 255, 255, 255}));
    HARU_EXPECT_TRUE(hasText(queue, "Qiuqi Institute"));
    HARU_EXPECT_FALSE(hasText(queue, "presents"));
    HARU_EXPECT_TRUE(findText(queue, "Qiuqi Institute")->rect.height >= 56);
    HARU_EXPECT_TRUE(findText(queue, "Qiuqi Institute")->rect.y <= 460);
    HARU_EXPECT_TRUE(countProjectionRects(queue) >= static_cast<std::size_t>(12));
}
