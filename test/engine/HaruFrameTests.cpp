#include "support/TestHarness.hpp"

#include <HaruFrame>

#include <cstddef>
#include <string>

namespace {

std::size_t countTextCommands(const haru::engine::graphics::RenderQueue& queue) {
    std::size_t count = 0;
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::Text) {
            ++count;
        }
    }

    return count;
}

std::size_t firstTextCommandIndex(const haru::engine::graphics::RenderQueue& queue) {
    for (std::size_t index = 0; index < queue.commands().size(); ++index) {
        if (queue.commands()[index].kind == haru::engine::graphics::DrawCommandKind::Text) {
            return index;
        }
    }

    return queue.commands().size();
}

bool hasTextCommand(const haru::engine::graphics::RenderQueue& queue, const std::string& text) {
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::Text &&
            command.text == text) {
            return true;
        }
    }

    return false;
}

const haru::engine::graphics::DrawCommand* findTextCommand(
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

} // namespace

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
    HARU_EXPECT_TRUE(hasTextCommand(queue, "H"));
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

HARU_TEST(haru_frame_animates_opening_title_letters_with_staggered_bounce) {
    haru::engine::HaruFrame frame(2.0);
    haru::engine::graphics::RenderQueue firstFrame;
    haru::engine::graphics::RenderQueue secondFrame;

    frame.render(firstFrame, {1280, 720}, 0.10, [](haru::engine::graphics::RenderQueue&) {});
    frame.render(secondFrame, {1280, 720}, 0.35, [](haru::engine::graphics::RenderQueue&) {});

    const auto firstTextIndex = firstTextCommandIndex(firstFrame);
    const auto secondTextIndex = firstTextCommandIndex(secondFrame);

    HARU_EXPECT_TRUE(firstTextIndex < firstFrame.commands().size());
    HARU_EXPECT_TRUE(secondTextIndex < secondFrame.commands().size());
    HARU_EXPECT_TRUE(countTextCommands(firstFrame) < countTextCommands(secondFrame));
    HARU_EXPECT_EQ(firstFrame.commands()[firstTextIndex].text, "H");
    HARU_EXPECT_EQ(secondFrame.commands()[secondTextIndex].text, "H");
    HARU_EXPECT_TRUE(firstFrame.commands()[firstTextIndex].rect.y !=
                     secondFrame.commands()[secondTextIndex].rect.y);
    HARU_EXPECT_TRUE(firstFrame.commands()[firstTextIndex].text.size() == 1U);
}

HARU_TEST(haru_frame_animates_opening_icon_limbs) {
    haru::engine::HaruFrame frame(2.0);
    haru::engine::graphics::RenderQueue firstFrame;
    haru::engine::graphics::RenderQueue secondFrame;

    frame.render(firstFrame, {1280, 720}, 0.0, [](haru::engine::graphics::RenderQueue&) {});
    frame.render(secondFrame, {1280, 720}, 0.25, [](haru::engine::graphics::RenderQueue&) {});

    const auto firstTextIndex = firstTextCommandIndex(firstFrame);
    const auto secondTextIndex = firstTextCommandIndex(secondFrame);
    const auto firstLimbIndex = firstTextIndex - static_cast<std::size_t>(6);
    const auto lastLimbIndex = firstTextIndex - static_cast<std::size_t>(1);

    HARU_EXPECT_TRUE(firstTextIndex >= static_cast<std::size_t>(6));
    HARU_EXPECT_TRUE(secondTextIndex >= static_cast<std::size_t>(6));
    HARU_EXPECT_EQ(firstFrame.commands()[firstLimbIndex].kind,
                   haru::engine::graphics::DrawCommandKind::FillRect);
    HARU_EXPECT_EQ(firstFrame.commands()[lastLimbIndex].kind,
                   haru::engine::graphics::DrawCommandKind::FillRect);
    HARU_EXPECT_TRUE(firstFrame.commands()[firstLimbIndex].rect.y !=
                     secondFrame.commands()[secondTextIndex - static_cast<std::size_t>(6)].rect.y);
    HARU_EXPECT_TRUE(firstFrame.commands()[lastLimbIndex].rect.x !=
                     secondFrame.commands()[secondTextIndex - static_cast<std::size_t>(1)].rect.x);
}

HARU_TEST(haru_frame_gives_wide_title_letters_enough_text_bounds) {
    haru::engine::HaruFrame frame(2.0);
    haru::engine::graphics::RenderQueue queue;

    frame.render(queue, {1280, 720}, 1.0, [](haru::engine::graphics::RenderQueue&) {});

    const auto* narrowLetter = findTextCommand(queue, "i");
    const auto* wideLetter = findTextCommand(queue, "m");

    HARU_EXPECT_TRUE(narrowLetter != nullptr);
    HARU_EXPECT_TRUE(wideLetter != nullptr);
    HARU_EXPECT_TRUE(wideLetter->rect.width >= 48);
    HARU_EXPECT_TRUE(wideLetter->rect.width > narrowLetter->rect.width);
}
