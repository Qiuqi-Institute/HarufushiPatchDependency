#include "support/TestHarness.hpp"

#include "game/localization/GameText.hpp"
#include "game/scenes/SettingsScene.hpp"

#include <optional>
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

} // namespace

HARU_TEST(settings_scene_renders_a_dedicated_page_without_home_menu_buttons) {
    haru::game::localization::GameText text =
        haru::game::localization::GameText::loadDefault("en-US");
    haru::game::scenes::SettingsScene settingsScene(text);
    haru::engine::graphics::RenderQueue queue;

    settingsScene.render(queue, {1280, 720});

    HARU_EXPECT_TRUE(hasText(queue, "Settings"));
    HARU_EXPECT_TRUE(hasText(queue, "Language"));
    HARU_EXPECT_TRUE(hasText(queue, "Audio 80"));
    HARU_EXPECT_TRUE(hasText(queue, "Visual 100"));
    HARU_EXPECT_TRUE(hasText(queue, "Text Speed Normal"));
    HARU_EXPECT_TRUE(hasText(queue, "English"));
    HARU_EXPECT_TRUE(hasText(queue, "简体中文"));
    HARU_EXPECT_TRUE(hasText(queue, "日本語"));
    HARU_EXPECT_TRUE(hasText(queue, "Back"));
    HARU_EXPECT_FALSE(hasText(queue, "New Game"));
    HARU_EXPECT_FALSE(hasText(queue, "Load"));
    HARU_EXPECT_FALSE(hasText(queue, "Quit"));
}

HARU_TEST(settings_scene_maps_language_buttons_and_back_to_actions) {
    haru::game::scenes::SettingsScene settingsScene;

    const std::optional<haru::game::scenes::SettingsAction> english =
        settingsScene.actionAt({484, 306}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> chinese =
        settingsScene.actionAt({484, 370}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> japanese =
        settingsScene.actionAt({484, 434}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> back =
        settingsScene.actionAt({96, 620}, {1280, 720});

    HARU_EXPECT_TRUE(english.has_value());
    HARU_EXPECT_TRUE(chinese.has_value());
    HARU_EXPECT_TRUE(japanese.has_value());
    HARU_EXPECT_TRUE(back.has_value());
    HARU_EXPECT_EQ(*english, haru::game::scenes::SettingsAction::SetLocaleEnglish);
    HARU_EXPECT_EQ(*chinese,
                   haru::game::scenes::SettingsAction::SetLocaleSimplifiedChinese);
    HARU_EXPECT_EQ(*japanese, haru::game::scenes::SettingsAction::SetLocaleJapanese);
    HARU_EXPECT_EQ(*back, haru::game::scenes::SettingsAction::Back);
}

HARU_TEST(settings_scene_uses_adaptive_language_text_widths) {
    haru::game::scenes::SettingsScene settingsScene;
    haru::engine::graphics::RenderQueue queue;

    settingsScene.render(queue, {1280, 720});

    const auto* english = findText(queue, "English");
    const auto* chinese = findText(queue, "简体中文");
    const auto* japanese = findText(queue, "日本語");
    HARU_EXPECT_TRUE(english != nullptr);
    HARU_EXPECT_TRUE(chinese != nullptr);
    HARU_EXPECT_TRUE(japanese != nullptr);
    HARU_EXPECT_TRUE(chinese->rect.width < english->rect.width);
    HARU_EXPECT_TRUE(japanese->rect.width <= english->rect.width);
}
