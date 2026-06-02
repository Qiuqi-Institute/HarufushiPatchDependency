#include "support/TestHarness.hpp"

#include "game/localization/GameText.hpp"
#include "game/scenes/HomeScene.hpp"

#include <optional>
#include <string>
#include <vector>

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

bool hasVerticalGradient(const haru::engine::graphics::RenderQueue& queue,
                         haru::engine::graphics::Color topColor,
                         haru::engine::graphics::Color bottomColor) {
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::FillVerticalGradient &&
            command.color == topColor && command.secondaryColor == bottomColor) {
            return true;
        }
    }

    return false;
}

bool hasRoundedRectAt(const haru::engine::graphics::RenderQueue& queue,
                      haru::engine::graphics::Rect rect,
                      haru::engine::graphics::Color color) {
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::FillRoundedRect &&
            command.rect.x == rect.x && command.rect.y == rect.y &&
            command.rect.width == rect.width && command.rect.height == rect.height &&
            command.color == color) {
            return true;
        }
    }

    return false;
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

HARU_TEST(home_scene_uses_modern_full_bleed_title_screen_layout) {
    haru::game::scenes::HomeScene homeScene;
    haru::engine::graphics::RenderQueue queue;

    homeScene.render(queue, {1280, 720}, haru::game::scenes::HomePanel::Main);

    HARU_EXPECT_EQ(queue.commands()[0].color,
                   (haru::engine::graphics::Color{18, 22, 32, 255}));
    HARU_EXPECT_TRUE(hasVerticalGradient(queue,
                                         {18, 22, 32, 255},
                                         {226, 238, 232, 255}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {236, 84, 104, 255}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {56, 216, 196, 255}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {39, 47, 68, 232}));
    HARU_EXPECT_FALSE(hasFillColor(queue, {255, 210, 222, 255}));
    HARU_EXPECT_FALSE(hasFillColor(queue, {185, 226, 232, 255}));
    HARU_EXPECT_TRUE(hasCommandKind(queue,
                                    haru::engine::graphics::DrawCommandKind::
                                        FillRoundedRect));
    HARU_EXPECT_TRUE(hasCommandKind(queue,
                                    haru::engine::graphics::DrawCommandKind::
                                        FillVerticalGradient));
    HARU_EXPECT_TRUE(hasCommandKind(queue,
                                    haru::engine::graphics::DrawCommandKind::
                                        StrokeRect));
    const auto* title = findText(queue, "Harufushi Patch Dependency");
    HARU_EXPECT_TRUE(title != nullptr);
    HARU_EXPECT_TRUE(title->rect.x <= 96);
    HARU_EXPECT_TRUE(title->rect.y >= 500);
    HARU_EXPECT_TRUE(title->rect.width >= 560);
    HARU_EXPECT_TRUE(title->rect.height >= 48);

    const auto* newGame = findText(queue, "New Game");
    const auto* load = findText(queue, "Load");
    const auto* settings = findText(queue, "Settings");
    const auto* quit = findText(queue, "Quit");
    HARU_EXPECT_TRUE(newGame != nullptr);
    HARU_EXPECT_TRUE(load != nullptr);
    HARU_EXPECT_TRUE(settings != nullptr);
    HARU_EXPECT_TRUE(quit != nullptr);
    HARU_EXPECT_TRUE(newGame->rect.y >= 632);
    HARU_EXPECT_TRUE(load->rect.y >= 632);
    HARU_EXPECT_TRUE(settings->rect.y >= 632);
    HARU_EXPECT_TRUE(quit->rect.y >= 632);
    HARU_EXPECT_TRUE(newGame->rect.x < load->rect.x);
    HARU_EXPECT_TRUE(load->rect.x < settings->rect.x);
    HARU_EXPECT_TRUE(settings->rect.x < quit->rect.x);
    HARU_EXPECT_FALSE(hasRoundedRectAt(queue,
                                       {72, 634, 236, 48},
                                       {236, 84, 104, 255}));
    HARU_EXPECT_FALSE(hasRoundedRectAt(queue,
                                       {336, 634, 236, 48},
                                       {39, 47, 68, 232}));

    const auto* boardTitle = findText(queue, "Patch Board");
    HARU_EXPECT_TRUE(boardTitle != nullptr);
    HARU_EXPECT_TRUE(boardTitle->rect.y <= 180);
}

HARU_TEST(home_scene_maps_main_menu_points_to_home_actions) {
    haru::game::scenes::HomeScene homeScene;

    const std::optional<haru::game::scenes::HomeAction> newGame =
        homeScene.actionAt({124, 660}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> load =
        homeScene.actionAt({384, 660}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> settings =
        homeScene.actionAt({704, 660}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> quit =
        homeScene.actionAt({1068, 660}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> blank =
        homeScene.actionAt({24, 24}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> oldLeftRailPoint =
        homeScene.actionAt({140, 268}, {1280, 720}, haru::game::scenes::HomePanel::Main);

    HARU_EXPECT_TRUE(newGame.has_value());
    HARU_EXPECT_TRUE(load.has_value());
    HARU_EXPECT_TRUE(settings.has_value());
    HARU_EXPECT_TRUE(quit.has_value());
    HARU_EXPECT_FALSE(blank.has_value());
    HARU_EXPECT_FALSE(oldLeftRailPoint.has_value());
    HARU_EXPECT_EQ(*newGame, haru::game::scenes::HomeAction::NewGame);
    HARU_EXPECT_EQ(*load, haru::game::scenes::HomeAction::OpenSaves);
    HARU_EXPECT_EQ(*settings, haru::game::scenes::HomeAction::OpenSettings);
    HARU_EXPECT_EQ(*quit, haru::game::scenes::HomeAction::Quit);
}

HARU_TEST(home_scene_renders_saves_panel_without_settings_controls) {
    haru::game::scenes::HomeScene homeScene;
    haru::engine::graphics::RenderQueue savesQueue;

    homeScene.render(savesQueue, {1280, 720}, haru::game::scenes::HomePanel::Saves);

    HARU_EXPECT_TRUE(hasText(savesQueue, "Save Files"));
    HARU_EXPECT_TRUE(hasText(savesQueue, "No save data yet"));
    HARU_EXPECT_TRUE(hasText(savesQueue, "Back"));
    HARU_EXPECT_FALSE(hasText(savesQueue, "Audio 80"));
    HARU_EXPECT_FALSE(hasText(savesQueue, "English"));
}

HARU_TEST(home_scene_renders_save_summaries_and_maps_slots) {
    haru::game::scenes::HomeScene homeScene;
    haru::engine::graphics::RenderQueue queue;

    homeScene.render(queue,
                     {1280, 720},
                     haru::game::scenes::HomePanel::Saves,
                     {"Save 1  Day 3  Mod 20", "Save 2  Day 8  Mod 71"});

    HARU_EXPECT_TRUE(hasText(queue, "Save 1  Day 3  Mod 20"));
    HARU_EXPECT_TRUE(hasText(queue, "Save 2  Day 8  Mod 71"));
    HARU_EXPECT_FALSE(hasText(queue, "No save data yet"));

    const auto first =
        homeScene.actionAt({260, 282}, {1280, 720}, haru::game::scenes::HomePanel::Saves, 2);
    const auto second =
        homeScene.actionAt({260, 346}, {1280, 720}, haru::game::scenes::HomePanel::Saves, 2);

    HARU_EXPECT_TRUE(first.has_value());
    HARU_EXPECT_TRUE(second.has_value());
    HARU_EXPECT_EQ(*first, haru::game::scenes::HomeAction::LoadSave0);
    HARU_EXPECT_EQ(*second, haru::game::scenes::HomeAction::LoadSave1);
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
