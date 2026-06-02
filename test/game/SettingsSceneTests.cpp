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
    HARU_EXPECT_TRUE(hasText(queue, "Game"));
    HARU_EXPECT_TRUE(hasText(queue, "Audio"));
    HARU_EXPECT_TRUE(hasText(queue, "Display"));
    HARU_EXPECT_TRUE(hasText(queue, "Language"));
    HARU_EXPECT_TRUE(hasText(queue, "Master Volume"));
    HARU_EXPECT_TRUE(hasText(queue, "Window Scale"));
    HARU_EXPECT_TRUE(hasText(queue, "Text Speed"));
    HARU_EXPECT_TRUE(hasText(queue, "80"));
    HARU_EXPECT_TRUE(hasText(queue, "100"));
    HARU_EXPECT_TRUE(hasText(queue, "50"));
    HARU_EXPECT_TRUE(hasText(queue, "English"));
    HARU_EXPECT_TRUE(hasText(queue, "简体中文"));
    HARU_EXPECT_TRUE(hasText(queue, "日本語"));
    HARU_EXPECT_TRUE(hasText(queue, "Back"));
    HARU_EXPECT_FALSE(hasText(queue, "New Game"));
    HARU_EXPECT_FALSE(hasText(queue, "Load"));
    HARU_EXPECT_FALSE(hasText(queue, "Quit"));
}

HARU_TEST(settings_scene_maps_language_controls_sliders_tabs_and_back_to_actions) {
    haru::game::scenes::SettingsScene settingsScene;

    const std::optional<haru::game::scenes::SettingsAction> english =
        settingsScene.actionAt({492, 248}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> chinese =
        settingsScene.actionAt({640, 248}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> japanese =
        settingsScene.actionAt({790, 248}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> audioUp =
        settingsScene.actionAt({1028, 350}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> audioDown =
        settingsScene.actionAt({492, 350}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> displayTab =
        settingsScene.actionAt({238, 378}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> back =
        settingsScene.actionAt({94, 642}, {1280, 720});

    HARU_EXPECT_TRUE(english.has_value());
    HARU_EXPECT_TRUE(chinese.has_value());
    HARU_EXPECT_TRUE(japanese.has_value());
    HARU_EXPECT_TRUE(audioUp.has_value());
    HARU_EXPECT_TRUE(audioDown.has_value());
    HARU_EXPECT_TRUE(displayTab.has_value());
    HARU_EXPECT_TRUE(back.has_value());
    HARU_EXPECT_EQ(*english, haru::game::scenes::SettingsAction::SetLocaleEnglish);
    HARU_EXPECT_EQ(*chinese,
                   haru::game::scenes::SettingsAction::SetLocaleSimplifiedChinese);
    HARU_EXPECT_EQ(*japanese, haru::game::scenes::SettingsAction::SetLocaleJapanese);
    HARU_EXPECT_EQ(*audioUp, haru::game::scenes::SettingsAction::IncreaseMasterVolume);
    HARU_EXPECT_EQ(*audioDown, haru::game::scenes::SettingsAction::DecreaseMasterVolume);
    HARU_EXPECT_EQ(*displayTab, haru::game::scenes::SettingsAction::SelectDisplayTab);
    HARU_EXPECT_EQ(*back, haru::game::scenes::SettingsAction::Back);
}

HARU_TEST(settings_scene_uses_commercial_settings_layout_grid) {
    haru::game::scenes::SettingsScene settingsScene;
    haru::engine::graphics::RenderQueue queue;

    settingsScene.render(queue, {1280, 720});

    const auto* gameTab = findText(queue, "Game");
    const auto* language = findText(queue, "Language");
    const auto* masterVolume = findText(queue, "Master Volume");
    const auto* back = findText(queue, "Back");
    HARU_EXPECT_TRUE(gameTab != nullptr);
    HARU_EXPECT_TRUE(language != nullptr);
    HARU_EXPECT_TRUE(masterVolume != nullptr);
    HARU_EXPECT_TRUE(back != nullptr);
    HARU_EXPECT_TRUE(gameTab->rect.x < language->rect.x);
    HARU_EXPECT_TRUE(masterVolume->rect.x >= language->rect.x);
    HARU_EXPECT_TRUE(back->rect.y >= 620);
}
