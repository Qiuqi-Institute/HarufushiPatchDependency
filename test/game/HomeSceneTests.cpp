#include "support/TestHarness.hpp"

#include "game/localization/GameText.hpp"
#include "game/scenes/HomeScene.hpp"

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

bool hasFillColor(const haru::engine::graphics::RenderQueue& queue,
                  haru::engine::graphics::Color color) {
    for (const auto& command : queue.commands()) {
        if ((command.kind == haru::engine::graphics::DrawCommandKind::FillRect ||
             command.kind == haru::engine::graphics::DrawCommandKind::FillRoundedRect ||
             command.kind == haru::engine::graphics::DrawCommandKind::FillEllipse ||
             command.kind == haru::engine::graphics::DrawCommandKind::FillVerticalGradient) &&
            command.color == color) {
            return true;
        }
    }

    return false;
}

bool hasCommandKind(const haru::engine::graphics::RenderQueue& queue,
                    haru::engine::graphics::DrawCommandKind kind) {
    for (const auto& command : queue.commands()) {
        if (command.kind == kind) {
            return true;
        }
    }

    return false;
}

std::size_t countFillColor(const haru::engine::graphics::RenderQueue& queue,
                           haru::engine::graphics::Color color) {
    std::size_t count = 0;
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::FillRect &&
            command.color == color) {
            ++count;
        }
    }

    return count;
}

std::size_t countText(const haru::engine::graphics::RenderQueue& queue,
                      const std::string& text) {
    std::size_t count = 0;
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::Text &&
            command.text == text) {
            ++count;
        }
    }

    return count;
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

HARU_TEST(home_scene_renders_main_menu_actions_after_splashes) {
    haru::game::scenes::HomeScene homeScene;
    haru::engine::graphics::RenderQueue queue;

    homeScene.render(queue, {1280, 720}, haru::game::scenes::HomePanel::Main);

    HARU_EXPECT_TRUE(queue.commands().size() >= static_cast<std::size_t>(12));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Clear);
    HARU_EXPECT_TRUE(hasText(queue, "Harufushi Patch Dependency"));
    HARU_EXPECT_TRUE(hasText(queue, "New Game"));
    HARU_EXPECT_TRUE(hasText(queue, "Load"));
    HARU_EXPECT_TRUE(hasText(queue, "Settings"));
    HARU_EXPECT_TRUE(hasText(queue, "Quit"));
    HARU_EXPECT_TRUE(hasText(queue, "Patch Board"));
    HARU_EXPECT_TRUE(hasText(queue, "Spring UI pass"));
    HARU_EXPECT_TRUE(hasText(queue, "Harufushi is watching your commits"));
    HARU_EXPECT_EQ(countText(queue, "Harufushi Patch Dependency"),
                   static_cast<std::size_t>(1));
}

HARU_TEST(home_scene_uses_japanese_fresh_galgame_visual_style) {
    haru::game::scenes::HomeScene homeScene;
    haru::engine::graphics::RenderQueue queue;

    homeScene.render(queue, {1280, 720}, haru::game::scenes::HomePanel::Main);

    HARU_EXPECT_EQ(queue.commands()[0].color,
                   (haru::engine::graphics::Color{251, 248, 241, 255}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {255, 210, 222, 255}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {185, 226, 232, 255}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {255, 252, 248, 255}));
    HARU_EXPECT_TRUE(hasCommandKind(queue,
                                    haru::engine::graphics::DrawCommandKind::
                                        FillRoundedRect));
    HARU_EXPECT_TRUE(hasCommandKind(queue,
                                    haru::engine::graphics::DrawCommandKind::
                                        FillVerticalGradient));
    HARU_EXPECT_TRUE(hasCommandKind(queue,
                                    haru::engine::graphics::DrawCommandKind::
                                        StrokeRect));
    HARU_EXPECT_TRUE(findText(queue, "Harufushi Patch Dependency")->rect.width >= 560);
}

HARU_TEST(home_scene_maps_main_menu_points_to_home_actions) {
    haru::game::scenes::HomeScene homeScene;

    const std::optional<haru::game::scenes::HomeAction> newGame =
        homeScene.actionAt({140, 268}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> load =
        homeScene.actionAt({140, 332}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> settings =
        homeScene.actionAt({140, 396}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> quit =
        homeScene.actionAt({140, 460}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> blank =
        homeScene.actionAt({24, 24}, {1280, 720}, haru::game::scenes::HomePanel::Main);

    HARU_EXPECT_TRUE(newGame.has_value());
    HARU_EXPECT_TRUE(load.has_value());
    HARU_EXPECT_TRUE(settings.has_value());
    HARU_EXPECT_TRUE(quit.has_value());
    HARU_EXPECT_FALSE(blank.has_value());
    HARU_EXPECT_EQ(*newGame, haru::game::scenes::HomeAction::NewGame);
    HARU_EXPECT_EQ(*load, haru::game::scenes::HomeAction::OpenSaves);
    HARU_EXPECT_EQ(*settings, haru::game::scenes::HomeAction::OpenSettings);
    HARU_EXPECT_EQ(*quit, haru::game::scenes::HomeAction::Quit);
}

HARU_TEST(home_scene_maps_settings_language_buttons_to_locale_actions) {
    haru::game::scenes::HomeScene homeScene;

    const std::optional<haru::game::scenes::HomeAction> english =
        homeScene.actionAt({560, 360}, {1280, 720}, haru::game::scenes::HomePanel::Settings);
    const std::optional<haru::game::scenes::HomeAction> chinese =
        homeScene.actionAt({560, 420}, {1280, 720}, haru::game::scenes::HomePanel::Settings);
    const std::optional<haru::game::scenes::HomeAction> japanese =
        homeScene.actionAt({560, 480}, {1280, 720}, haru::game::scenes::HomePanel::Settings);

    HARU_EXPECT_TRUE(english.has_value());
    HARU_EXPECT_TRUE(chinese.has_value());
    HARU_EXPECT_TRUE(japanese.has_value());
    HARU_EXPECT_EQ(*english, haru::game::scenes::HomeAction::SetLocaleEnglish);
    HARU_EXPECT_EQ(*chinese, haru::game::scenes::HomeAction::SetLocaleSimplifiedChinese);
    HARU_EXPECT_EQ(*japanese, haru::game::scenes::HomeAction::SetLocaleJapanese);
}

HARU_TEST(home_scene_renders_saves_and_settings_panels) {
    haru::game::scenes::HomeScene homeScene;
    haru::engine::graphics::RenderQueue savesQueue;
    haru::engine::graphics::RenderQueue settingsQueue;

    homeScene.render(savesQueue, {1280, 720}, haru::game::scenes::HomePanel::Saves);
    homeScene.render(settingsQueue, {1280, 720}, haru::game::scenes::HomePanel::Settings);

    HARU_EXPECT_TRUE(hasText(savesQueue, "Save Files"));
    HARU_EXPECT_TRUE(hasText(savesQueue, "No save data yet"));
    HARU_EXPECT_TRUE(hasText(savesQueue, "Back"));
    HARU_EXPECT_TRUE(hasText(settingsQueue, "Settings"));
    HARU_EXPECT_TRUE(hasText(settingsQueue, "Audio 80"));
    HARU_EXPECT_TRUE(hasText(settingsQueue, "Back"));
}

HARU_TEST(home_scene_uses_game_text_localization) {
    haru::game::localization::GameText text =
        haru::game::localization::GameText::loadDefault("zh-CN");
    haru::game::scenes::HomeScene homeScene(text);
    haru::engine::graphics::RenderQueue queue;

    homeScene.render(queue, {1280, 720}, haru::game::scenes::HomePanel::Main);

    HARU_EXPECT_TRUE(hasText(queue, "春伏补丁依存症"));
    HARU_EXPECT_TRUE(hasText(queue, "开始游戏"));
    HARU_EXPECT_TRUE(hasText(queue, "读取存档"));
    HARU_EXPECT_TRUE(hasText(queue, "设置"));
    HARU_EXPECT_TRUE(hasText(queue, "退出"));
    HARU_EXPECT_TRUE(hasText(queue, "补丁看板"));
}

HARU_TEST(home_scene_renders_language_switcher_in_settings) {
    haru::game::localization::GameText text =
        haru::game::localization::GameText::loadDefault("zh-CN");
    haru::game::scenes::HomeScene homeScene(text);
    haru::engine::graphics::RenderQueue queue;

    homeScene.render(queue, {1280, 720}, haru::game::scenes::HomePanel::Settings);

    HARU_EXPECT_TRUE(hasText(queue, "语言"));
    HARU_EXPECT_TRUE(hasText(queue, "English"));
    HARU_EXPECT_TRUE(hasText(queue, "简体中文"));
    HARU_EXPECT_TRUE(hasText(queue, "日本語"));
}

HARU_TEST(home_scene_keeps_settings_text_inside_galgame_panel) {
    haru::game::scenes::HomeScene homeScene;
    haru::engine::graphics::RenderQueue queue;

    homeScene.render(queue, {1280, 720}, haru::game::scenes::HomePanel::Settings);

    const auto* textSpeed = findText(queue, "Text Speed Normal");
    HARU_EXPECT_TRUE(textSpeed != nullptr);
    HARU_EXPECT_TRUE(textSpeed->rect.y + textSpeed->rect.height <= 580);
}
