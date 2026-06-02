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

HARU_TEST(settings_scene_game_page_renders_only_game_settings) {
    haru::game::localization::GameText text =
        haru::game::localization::GameText::loadDefault("en-US");
    haru::game::scenes::SettingsScene settingsScene(text);
    haru::engine::graphics::RenderQueue queue;

    settingsScene.render(queue, {1280, 720});

    HARU_EXPECT_TRUE(hasText(queue, "System Setting"));
    HARU_EXPECT_TRUE(hasText(queue, "Game"));
    HARU_EXPECT_TRUE(hasText(queue, "Audio"));
    HARU_EXPECT_TRUE(hasText(queue, "Display"));
    HARU_EXPECT_TRUE(hasText(queue, "Language"));
    HARU_EXPECT_TRUE(hasText(queue, "Text Speed"));
    HARU_EXPECT_TRUE(hasText(queue, "Skip Mode"));
    HARU_EXPECT_TRUE(hasText(queue, "Auto Mode"));
    HARU_EXPECT_TRUE(hasText(queue, "50"));
    HARU_EXPECT_TRUE(hasText(queue, "English"));
    HARU_EXPECT_TRUE(hasText(queue, "简体中文"));
    HARU_EXPECT_TRUE(hasText(queue, "日本語"));
    HARU_EXPECT_TRUE(hasText(queue, "Back"));
    HARU_EXPECT_FALSE(hasText(queue, "Master Volume"));
    HARU_EXPECT_FALSE(hasText(queue, "BGM Volume"));
    HARU_EXPECT_FALSE(hasText(queue, "SE Volume"));
    HARU_EXPECT_FALSE(hasText(queue, "Window Scale"));
    HARU_EXPECT_FALSE(hasText(queue, "Aspect Ratio"));
    HARU_EXPECT_FALSE(hasText(queue, "New Game"));
    HARU_EXPECT_FALSE(hasText(queue, "Load"));
    HARU_EXPECT_FALSE(hasText(queue, "Quit"));
}

HARU_TEST(settings_scene_audio_page_renders_audio_mixer_content) {
    haru::game::scenes::SettingsState state;
    state.activeTab = haru::game::scenes::SettingsTab::Audio;
    haru::game::scenes::SettingsScene settingsScene(
        haru::game::localization::GameText::loadDefault("en-US"),
        state);
    haru::engine::graphics::RenderQueue queue;

    settingsScene.render(queue, {1280, 720});

    HARU_EXPECT_TRUE(hasText(queue, "Audio"));
    HARU_EXPECT_TRUE(hasText(queue, "Master Volume"));
    HARU_EXPECT_TRUE(hasText(queue, "BGM Volume"));
    HARU_EXPECT_TRUE(hasText(queue, "SE Volume"));
    HARU_EXPECT_TRUE(hasText(queue, "80"));
    HARU_EXPECT_TRUE(hasText(queue, "70"));
    HARU_EXPECT_FALSE(hasText(queue, "Language"));
    HARU_EXPECT_FALSE(hasText(queue, "Window Scale"));
}

HARU_TEST(settings_scene_display_page_renders_display_content) {
    haru::game::scenes::SettingsState state;
    state.activeTab = haru::game::scenes::SettingsTab::Display;
    haru::game::scenes::SettingsScene settingsScene(
        haru::game::localization::GameText::loadDefault("en-US"),
        state);
    haru::engine::graphics::RenderQueue queue;

    settingsScene.render(queue, {1280, 720});

    HARU_EXPECT_TRUE(hasText(queue, "Display"));
    HARU_EXPECT_TRUE(hasText(queue, "Window Scale"));
    HARU_EXPECT_TRUE(hasText(queue, "Aspect Ratio"));
    HARU_EXPECT_TRUE(hasText(queue, "Display Mode"));
    HARU_EXPECT_TRUE(hasText(queue, "Windowed"));
    HARU_EXPECT_TRUE(hasText(queue, "Fullscreen"));
    HARU_EXPECT_TRUE(hasText(queue, "100"));
    HARU_EXPECT_FALSE(hasText(queue, "Language"));
    HARU_EXPECT_FALSE(hasText(queue, "Master Volume"));
}

HARU_TEST(settings_scene_maps_visible_controls_tabs_and_back_to_actions) {
    haru::game::scenes::SettingsScene settingsScene;

    const std::optional<haru::game::scenes::SettingsAction> english =
        settingsScene.actionAt({492, 248}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> chinese =
        settingsScene.actionAt({640, 248}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> japanese =
        settingsScene.actionAt({790, 248}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> textSpeedUp =
        settingsScene.actionAt({1028, 350}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> textSpeedDown =
        settingsScene.actionAt({492, 350}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> displayTab =
        settingsScene.actionAt({790, 104}, {1280, 720});
    const std::optional<haru::game::scenes::SettingsAction> back =
        settingsScene.actionAt({94, 642}, {1280, 720});

    HARU_EXPECT_TRUE(english.has_value());
    HARU_EXPECT_TRUE(chinese.has_value());
    HARU_EXPECT_TRUE(japanese.has_value());
    HARU_EXPECT_TRUE(textSpeedUp.has_value());
    HARU_EXPECT_TRUE(textSpeedDown.has_value());
    HARU_EXPECT_TRUE(displayTab.has_value());
    HARU_EXPECT_TRUE(back.has_value());
    HARU_EXPECT_EQ(*english, haru::game::scenes::SettingsAction::SetLocaleEnglish);
    HARU_EXPECT_EQ(*chinese,
                   haru::game::scenes::SettingsAction::SetLocaleSimplifiedChinese);
    HARU_EXPECT_EQ(*japanese, haru::game::scenes::SettingsAction::SetLocaleJapanese);
    HARU_EXPECT_EQ(*textSpeedUp, haru::game::scenes::SettingsAction::IncreaseTextSpeed);
    HARU_EXPECT_EQ(*textSpeedDown, haru::game::scenes::SettingsAction::DecreaseTextSpeed);
    HARU_EXPECT_EQ(*displayTab, haru::game::scenes::SettingsAction::SelectDisplayTab);
    HARU_EXPECT_EQ(*back, haru::game::scenes::SettingsAction::Back);
}

HARU_TEST(settings_scene_maps_audio_and_display_page_controls_to_their_own_actions) {
    haru::game::scenes::SettingsState audioState;
    audioState.activeTab = haru::game::scenes::SettingsTab::Audio;
    haru::game::scenes::SettingsScene audioScene(
        haru::game::localization::GameText::loadDefault("en-US"),
        audioState);

    HARU_EXPECT_EQ(*audioScene.actionAt({1028, 350}, {1280, 720}),
                   haru::game::scenes::SettingsAction::IncreaseMasterVolume);
    HARU_EXPECT_EQ(*audioScene.actionAt({492, 350}, {1280, 720}),
                   haru::game::scenes::SettingsAction::DecreaseMasterVolume);
    HARU_EXPECT_EQ(*audioScene.actionAt({1028, 450}, {1280, 720}),
                   haru::game::scenes::SettingsAction::IncreaseBgmVolume);
    HARU_EXPECT_EQ(*audioScene.actionAt({492, 550}, {1280, 720}),
                   haru::game::scenes::SettingsAction::DecreaseSeVolume);
    HARU_EXPECT_FALSE(audioScene.actionAt({640, 248}, {1280, 720}).has_value());

    haru::game::scenes::SettingsState displayState;
    displayState.activeTab = haru::game::scenes::SettingsTab::Display;
    haru::game::scenes::SettingsScene displayScene(
        haru::game::localization::GameText::loadDefault("en-US"),
        displayState);

    HARU_EXPECT_EQ(*displayScene.actionAt({1028, 350}, {1280, 720}),
                   haru::game::scenes::SettingsAction::IncreaseWindowScale);
    HARU_EXPECT_EQ(*displayScene.actionAt({492, 350}, {1280, 720}),
                   haru::game::scenes::SettingsAction::DecreaseWindowScale);
    HARU_EXPECT_FALSE(displayScene.actionAt({790, 248}, {1280, 720}).has_value());
}

HARU_TEST(settings_scene_uses_commercial_settings_layout_grid) {
    haru::game::scenes::SettingsScene settingsScene;
    haru::engine::graphics::RenderQueue queue;

    settingsScene.render(queue, {1280, 720});

    const auto* gameTab = findText(queue, "Game");
    const auto* language = findText(queue, "Language");
    const auto* textSpeed = findText(queue, "Text Speed");
    const auto* back = findText(queue, "Back");
    HARU_EXPECT_TRUE(gameTab != nullptr);
    HARU_EXPECT_TRUE(language != nullptr);
    HARU_EXPECT_TRUE(textSpeed != nullptr);
    HARU_EXPECT_TRUE(back != nullptr);
    HARU_EXPECT_TRUE(gameTab->rect.y < language->rect.y);
    HARU_EXPECT_TRUE(textSpeed->rect.x >= language->rect.x);
    HARU_EXPECT_TRUE(back->rect.y >= 620);
}
