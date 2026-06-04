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
             command.kind == haru::engine::graphics::DrawCommandKind::FillPolygon ||
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

bool hasVerticalGradientAt(const haru::engine::graphics::RenderQueue& queue,
                           haru::engine::graphics::Rect rect,
                           haru::engine::graphics::Color topColor,
                           haru::engine::graphics::Color bottomColor) {
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::FillVerticalGradient &&
            command.rect.x == rect.x && command.rect.y == rect.y &&
            command.rect.width == rect.width && command.rect.height == rect.height &&
            command.color == topColor && command.secondaryColor == bottomColor) {
            return true;
        }
    }

    return false;
}

bool hasImage(const haru::engine::graphics::RenderQueue& queue,
              const std::string& imagePath,
              haru::engine::graphics::Rect rect) {
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::Image &&
            command.text == imagePath && command.rect.x == rect.x &&
            command.rect.y == rect.y && command.rect.width == rect.width &&
            command.rect.height == rect.height) {
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

std::size_t countGlyphCells(const haru::engine::graphics::RenderQueue& queue) {
    std::size_t count = 0;
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::FillRoundedRect &&
            command.rect.x >= 900 && command.rect.x <= 1220 &&
            command.rect.y >= 460 && command.rect.y <= 610 &&
            command.rect.width == 7 && command.rect.height == 7) {
            ++count;
        }
    }

    return count;
}

std::size_t countLogoPolygons(const haru::engine::graphics::RenderQueue& queue) {
    std::size_t count = 0;
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::FillPolygon &&
            command.points.size() >= static_cast<std::size_t>(4)) {
            bool inLogoArea = true;
            for (const auto& point : command.points) {
                if (point.x < 780 || point.x > 1160 || point.y < 430 || point.y > 660) {
                    inLogoArea = false;
                    break;
                }
            }
            if (inLogoArea) {
                ++count;
            }
        }
    }

    return count;
}

} // namespace

HARU_TEST(home_scene_renders_main_menu_actions_after_splashes) {
    haru::game::scenes::HomeScene homeScene;
    haru::engine::graphics::RenderQueue queue;

    homeScene.render(queue, {1280, 720}, haru::game::scenes::HomePanel::Main);

    HARU_EXPECT_TRUE(queue.commands().size() >= static_cast<std::size_t>(12));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Clear);
    HARU_EXPECT_TRUE(hasText(queue, "New Game"));
    HARU_EXPECT_TRUE(hasText(queue, "Load"));
    HARU_EXPECT_TRUE(hasText(queue, "Settings"));
    HARU_EXPECT_TRUE(hasText(queue, "Quit"));
    HARU_EXPECT_FALSE(hasText(queue, "Harufushi Patch Dependency"));
    HARU_EXPECT_FALSE(hasText(queue, "Patch Board"));
    HARU_EXPECT_FALSE(hasText(queue, "Spring UI pass"));
    HARU_EXPECT_FALSE(hasText(queue, "Harufushi is watching your commits"));
}

HARU_TEST(home_scene_uses_png_background_transparent_text_menu_and_shape_logo) {
    haru::game::scenes::HomeScene homeScene;
    haru::engine::graphics::RenderQueue queue;

    homeScene.render(queue, {1280, 720}, haru::game::scenes::HomePanel::Main);

    HARU_EXPECT_EQ(queue.commands()[0].color, (haru::engine::graphics::Color{0, 0, 0, 0}));
    HARU_EXPECT_TRUE(hasImage(queue,
                              "resources/images/backgrounds/home_chunfu.png",
                              {0, 0, 1280, 720}));
    HARU_EXPECT_TRUE(hasVerticalGradientAt(queue,
                                           {0, 520, 1280, 200},
                                           {33, 47, 75, 0},
                                           {33, 47, 75, 176}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {11, 119, 155, 230}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {255, 183, 205, 230}));
    HARU_EXPECT_TRUE(hasCommandKind(queue,
                                    haru::engine::graphics::DrawCommandKind::
                                        FillRoundedRect));
    HARU_EXPECT_FALSE(hasText(queue, "Harufushi Patch Dependency"));
    HARU_EXPECT_FALSE(hasText(queue, "ハルフシ・パッチ・ディペンデンシー"));

    const auto* newGame = findText(queue, "New Game");
    const auto* load = findText(queue, "Load");
    const auto* settings = findText(queue, "Settings");
    const auto* quit = findText(queue, "Quit");
    HARU_EXPECT_TRUE(newGame != nullptr);
    HARU_EXPECT_TRUE(load != nullptr);
    HARU_EXPECT_TRUE(settings != nullptr);
    HARU_EXPECT_TRUE(quit != nullptr);
    HARU_EXPECT_TRUE(newGame->rect.y >= 662);
    HARU_EXPECT_TRUE(load->rect.y >= 662);
    HARU_EXPECT_TRUE(settings->rect.y >= 662);
    HARU_EXPECT_TRUE(quit->rect.y >= 662);
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
    HARU_EXPECT_TRUE(boardTitle == nullptr);
}

HARU_TEST(home_scene_menu_is_plain_text_without_button_decoration) {
    haru::game::scenes::HomeScene homeScene;
    haru::engine::graphics::RenderQueue queue;

    homeScene.render(queue, {1280, 720}, haru::game::scenes::HomePanel::Main);

    bool menuDecorationFound = false;
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::FillRoundedRect &&
            command.rect.y >= 620 && command.rect.height <= 8) {
            menuDecorationFound = true;
        }
        if (command.kind == haru::engine::graphics::DrawCommandKind::FillEllipse &&
            command.rect.y >= 620) {
            menuDecorationFound = true;
        }
    }

    HARU_EXPECT_FALSE(menuDecorationFound);
}

HARU_TEST(home_scene_art_title_is_shape_drawn_katakana_logo_strokes) {
    haru::game::scenes::HomeScene homeScene;
    haru::engine::graphics::RenderQueue queue;

    homeScene.render(queue, {1280, 720}, haru::game::scenes::HomePanel::Main);

    HARU_EXPECT_TRUE(countLogoPolygons(queue) >= static_cast<std::size_t>(12));
    HARU_EXPECT_EQ(countGlyphCells(queue), static_cast<std::size_t>(0));
    HARU_EXPECT_TRUE(hasFillColor(queue, {11, 119, 155, 230}));
    HARU_EXPECT_TRUE(hasFillColor(queue, {255, 183, 205, 230}));
    HARU_EXPECT_FALSE(hasText(queue, "ハルフシ"));
    HARU_EXPECT_FALSE(hasText(queue, "パッチ"));
}

HARU_TEST(home_scene_maps_main_menu_points_to_home_actions) {
    haru::game::scenes::HomeScene homeScene;

    const std::optional<haru::game::scenes::HomeAction> newGame =
        homeScene.actionAt({124, 684}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> load =
        homeScene.actionAt({384, 684}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> settings =
        homeScene.actionAt({704, 684}, {1280, 720}, haru::game::scenes::HomePanel::Main);
    const std::optional<haru::game::scenes::HomeAction> quit =
        homeScene.actionAt({1068, 684}, {1280, 720}, haru::game::scenes::HomePanel::Main);
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

    HARU_EXPECT_FALSE(hasText(queue, "春伏补丁依存症"));
    HARU_EXPECT_TRUE(hasText(queue, "开始游戏"));
    HARU_EXPECT_TRUE(hasText(queue, "读取存档"));
    HARU_EXPECT_TRUE(hasText(queue, "设置"));
    HARU_EXPECT_TRUE(hasText(queue, "退出"));
    HARU_EXPECT_FALSE(hasText(queue, "补丁看板"));
}
