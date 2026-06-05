#include "support/TestHarness.hpp"

#include "engine/ui/TextBox.hpp"

#include <cstddef>
#include <string>

namespace {

const haru::engine::graphics::DrawCommand* firstTextCommand(
    const haru::engine::graphics::RenderQueue& queue) {
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::Text) {
            return &command;
        }
    }

    return nullptr;
}

} // namespace

HARU_TEST(text_box_uses_adaptive_width_for_short_and_long_languages) {
    haru::engine::ui::TextBoxStyle style;
    style.text = {67, 76, 104, 255};
    style.minHorizontalPadding = 18;
    style.maxHorizontalPadding = 72;

    haru::engine::ui::TextBox chinese({100, 80, 640, 52}, "春伏补丁依存症", style);
    haru::engine::ui::TextBox english({100, 80, 640, 52},
                                      "Harufushi Patch Dependency",
                                      style);
    haru::engine::graphics::RenderQueue chineseQueue;
    haru::engine::graphics::RenderQueue englishQueue;

    chinese.render(chineseQueue);
    english.render(englishQueue);

    const auto* chineseText = firstTextCommand(chineseQueue);
    const auto* englishText = firstTextCommand(englishQueue);
    HARU_EXPECT_TRUE(chineseText != nullptr);
    HARU_EXPECT_TRUE(englishText != nullptr);
    HARU_EXPECT_TRUE(chineseText->rect.width < englishText->rect.width);
    HARU_EXPECT_EQ(chineseText->rect.x + (chineseText->rect.width / 2),
                   englishText->rect.x + (englishText->rect.width / 2));
    HARU_EXPECT_TRUE(chineseText->rect.x > 100);
    HARU_EXPECT_TRUE(englishText->rect.width <= 640);
}

HARU_TEST(text_box_clamps_very_long_text_inside_container) {
    haru::engine::ui::TextBoxStyle style;
    style.text = {67, 76, 104, 255};
    style.minHorizontalPadding = 12;
    style.maxHorizontalPadding = 72;

    haru::engine::ui::TextBox textBox({40, 20, 220, 44},
                                      "ハルフシ・パッチ・ディペンデンシー",
                                      style);
    haru::engine::graphics::RenderQueue queue;

    textBox.render(queue);

    const auto* text = firstTextCommand(queue);
    HARU_EXPECT_TRUE(text != nullptr);
    HARU_EXPECT_TRUE(text->rect.x >= 40);
    HARU_EXPECT_TRUE(text->rect.x + text->rect.width <= 260);
    HARU_EXPECT_TRUE(text->rect.width >= 180);
}

HARU_TEST(text_box_normalizes_negative_padding_style_without_invalid_clamp_bounds) {
    haru::engine::ui::TextBoxStyle style;
    style.minHorizontalPadding = -24;
    style.maxHorizontalPadding = -8;
    style.minWidth = -40;

    haru::engine::ui::TextBox textBox({12, 34, 96, 28}, "Patch", style);

    const auto rect = textBox.textRect();

    HARU_EXPECT_TRUE(rect.x >= 12);
    HARU_EXPECT_TRUE(rect.x + rect.width <= 108);
    HARU_EXPECT_TRUE(rect.width >= 0);
}
