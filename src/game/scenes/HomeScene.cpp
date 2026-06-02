#include "HomeScene.hpp"

#include <algorithm>
#include <array>
#include <utility>

namespace haru::game::scenes {

namespace {

constexpr engine::graphics::Color nightInk{18, 22, 32, 255};
constexpr engine::graphics::Color stormInk{39, 47, 68, 232};
constexpr engine::graphics::Color slate{63, 72, 96, 255};
constexpr engine::graphics::Color mist{226, 238, 232, 255};
constexpr engine::graphics::Color porcelain{246, 250, 246, 242};
constexpr engine::graphics::Color porcelainSolid{246, 250, 246, 255};
constexpr engine::graphics::Color pearl{255, 255, 251, 255};
constexpr engine::graphics::Color coral{236, 84, 104, 255};
constexpr engine::graphics::Color coralSoft{252, 172, 154, 230};
constexpr engine::graphics::Color electricMint{56, 216, 196, 255};
constexpr engine::graphics::Color iris{111, 91, 206, 238};
constexpr engine::graphics::Color amber{245, 184, 76, 255};
constexpr engine::graphics::Color glassLine{168, 204, 196, 255};
constexpr engine::graphics::Color softShadow{13, 17, 26, 128};

engine::ui::ButtonStyle titleMenuStyle(engine::graphics::Color background) {
    return {background, pearl, 10};
}

engine::graphics::Rect newGameBounds() {
    return {72, 634, 236, 48};
}

engine::graphics::Rect loadBounds() {
    return {336, 634, 236, 48};
}

engine::graphics::Rect settingsBounds() {
    return {652, 634, 248, 48};
}

engine::graphics::Rect quitBounds() {
    return {984, 634, 206, 48};
}

engine::graphics::Rect backBounds() {
    return {80, 634, 232, 48};
}

engine::ui::Button newGameButton(const localization::GameText& text) {
    return {newGameBounds(),
            text.get(localization::TextId::NewGame),
            titleMenuStyle(coral)};
}

engine::ui::Button loadButton(const localization::GameText& text) {
    return {loadBounds(),
            text.get(localization::TextId::Load),
            titleMenuStyle(stormInk)};
}

engine::ui::Button settingsButton(const localization::GameText& text) {
    return {settingsBounds(),
            text.get(localization::TextId::Settings),
            titleMenuStyle(stormInk)};
}

engine::ui::Button quitButton(const localization::GameText& text) {
    return {quitBounds(),
            text.get(localization::TextId::Quit),
            titleMenuStyle(stormInk)};
}

engine::ui::Button backButton(const localization::GameText& text) {
    return {backBounds(),
            text.get(localization::TextId::Back),
            titleMenuStyle(stormInk)};
}

engine::graphics::Rect saveSlotBounds(std::size_t index) {
    return {224, 264 + (static_cast<int>(index) * 64), 640, 48};
}

std::optional<HomeAction> saveSlotAction(std::size_t index) {
    constexpr std::array<HomeAction, 4> actions{HomeAction::LoadSave0,
                                                HomeAction::LoadSave1,
                                                HomeAction::LoadSave2,
                                                HomeAction::LoadSave3};
    if (index >= actions.size()) {
        return std::nullopt;
    }
    return actions[index];
}

bool contains(engine::graphics::Rect bounds, engine::graphics::Point point) {
    return point.x >= bounds.x && point.y >= bounds.y &&
           point.x < bounds.x + bounds.width && point.y < bounds.y + bounds.height;
}

void drawTextBox(engine::graphics::RenderQueue& queue,
                 engine::graphics::Rect bounds,
                 const std::string& value,
                 engine::graphics::Color color,
                 int minPadding = 16,
                 int maxPadding = 56) {
    engine::ui::TextBoxStyle style;
    style.text = color;
    style.minHorizontalPadding = minPadding;
    style.maxHorizontalPadding = maxPadding;
    engine::ui::TextBox(bounds, value, style).render(queue);
}

void drawHeroCharacter(engine::graphics::RenderQueue& queue) {
    queue.fillRoundedRect({788, 86, 298, 492}, {244, 247, 244, 214}, 54);
    queue.strokeRect({788, 86, 298, 492}, {255, 255, 251, 210}, 3);
    queue.fillEllipse({852, 118, 170, 156}, {45, 52, 73, 245});
    queue.fillEllipse({808, 174, 76, 132}, {45, 52, 73, 235});
    queue.fillEllipse({968, 174, 92, 150}, {45, 52, 73, 235});
    queue.fillEllipse({870, 156, 132, 132}, {255, 229, 220, 255});
    queue.fillRoundedRect({880, 258, 112, 194}, {251, 252, 249, 255}, 28);
    queue.fillRoundedRect({826, 300, 94, 188}, {236, 84, 104, 238}, 34);
    queue.fillRoundedRect({962, 300, 94, 188}, {56, 216, 196, 238}, 34);
    queue.fillRoundedRect({894, 290, 72, 38}, {111, 91, 206, 238}, 18);
    queue.fillEllipse({910, 202, 18, 18}, stormInk);
    queue.fillEllipse({970, 202, 18, 18}, stormInk);
    queue.fillRoundedRect({918, 238, 56, 8}, coral, 4);
    queue.fillRoundedRect({852, 452, 224, 46}, {31, 37, 54, 246}, 18);
    queue.fillRoundedRect({876, 467, 134, 7}, electricMint, 4);
    queue.fillRoundedRect({876, 484, 74, 7}, coralSoft, 4);
}

void drawCoderStage(engine::graphics::RenderQueue& queue) {
    queue.fillRoundedRect({560, 108, 342, 296}, {28, 35, 51, 236}, 34);
    queue.strokeRect({560, 108, 342, 296}, {86, 113, 130, 230}, 2);
    queue.fillRoundedRect({592, 140, 278, 174}, {17, 23, 36, 255}, 22);
    queue.fillRoundedRect({622, 170, 180, 8}, electricMint, 4);
    queue.fillRoundedRect({622, 196, 112, 8}, coral, 4);
    queue.fillRoundedRect({622, 222, 220, 8}, iris, 4);
    queue.fillRoundedRect({622, 248, 148, 8}, amber, 4);
    queue.fillRoundedRect({612, 340, 236, 28}, {246, 250, 246, 238}, 14);
    queue.fillRoundedRect({666, 372, 94, 18}, {246, 250, 246, 214}, 9);
    queue.fillRoundedRect({532, 452, 392, 44}, {29, 36, 52, 220}, 18);
    queue.fillRoundedRect({560, 469, 190, 7}, electricMint, 4);
    queue.fillRoundedRect({780, 469, 74, 7}, coral, 4);
}

void drawSceneBackground(engine::graphics::RenderQueue& queue,
                         engine::graphics::Size surfaceSize) {
    const int width = std::max(surfaceSize.width, 1);
    const int height = std::max(surfaceSize.height, 1);
    queue.clear(nightInk);
    queue.fillVerticalGradient({0, 0, width, height}, nightInk, mist);

    queue.fillEllipse({-150, 78, 430, 430}, {56, 216, 196, 92});
    queue.fillEllipse({980, -90, 410, 410}, {236, 84, 104, 96});
    queue.fillEllipse({870, 420, 360, 260}, {245, 184, 76, 70});
    queue.fillRoundedRect({-24, 604, width + 48, 116}, {13, 17, 26, 226}, 0);
    queue.fillRoundedRect({0, 594, width, 2}, {255, 255, 251, 86}, 1);

    queue.fillRoundedRect({52, 48, 356, 86}, {246, 250, 246, 44}, 28);
    queue.fillRoundedRect({72, 78, 176, 8}, electricMint, 4);
    queue.fillRoundedRect({72, 100, 104, 8}, coral, 4);
    queue.fillRoundedRect({1068, 170, 172, 34}, {246, 250, 246, 60}, 17);
    queue.fillRoundedRect({1100, 184, 92, 5}, electricMint, 3);

    drawCoderStage(queue);
    drawHeroCharacter(queue);

    queue.fillRoundedRect({1022, 64, 218, 44}, {39, 47, 68, 210}, 18);
    queue.strokeRect({1022, 64, 218, 44}, glassLine, 2);
    drawTextBox(queue, {1034, 72, 194, 28}, "PATCH 0.0.1", pearl, 12, 24);
}

void renderTitleMark(engine::graphics::RenderQueue& queue,
                     const localization::GameText& text) {
    queue.fillRoundedRect({62, 512, 660, 8}, electricMint, 4);
    queue.fillRoundedRect({62, 528, 390, 8}, coral, 4);
    queue.drawText({54, 540, 600, 58}, text.get(localization::TextId::GameTitle), pearl);
    queue.fillRoundedRect({70, 604, 146, 6}, amber, 3);
    queue.fillRoundedRect({230, 604, 82, 6}, electricMint, 3);
    queue.fillRoundedRect({326, 604, 114, 6}, coral, 3);
}

void renderTitleMenuButton(engine::graphics::RenderQueue& queue,
                           const engine::ui::Button& button,
                           bool primary) {
    const auto& bounds = button.bounds();
    queue.fillRoundedRect({bounds.x - 8, bounds.y + 42, bounds.width + 16, 3},
                          primary ? coral : glassLine,
                          2);
    queue.fillRoundedRect({bounds.x + 8, bounds.y + 8, 36, 6},
                          primary ? coral : electricMint,
                          3);
    queue.fillEllipse({bounds.x + bounds.width - 32, bounds.y + 16, 14, 14},
                      primary ? electricMint : coral);
    if (primary) {
        queue.fillRoundedRect({bounds.x - 2, bounds.y + 20, 22, 3}, pearl, 2);
        queue.fillRoundedRect({bounds.x - 2, bounds.y + 28, 46, 3}, coralSoft, 2);
    }
    drawTextBox(queue,
                {bounds.x + 24, bounds.y + 12, bounds.width - 48, 28},
                button.label(),
                pearl,
                10,
                32);
}

void renderBottomMenu(engine::graphics::RenderQueue& queue,
                      const localization::GameText& text) {
    queue.fillRoundedRect({58, 624, 1146, 1}, stormInk, 1);
    renderTitleMenuButton(queue, newGameButton(text), true);
    renderTitleMenuButton(queue, loadButton(text), false);
    renderTitleMenuButton(queue, settingsButton(text), false);
    renderTitleMenuButton(queue, quitButton(text), false);
}

void renderPatchBoard(engine::graphics::RenderQueue& queue,
                      const localization::GameText& text) {
    queue.fillRoundedRect({72, 146, 486, 112}, {246, 250, 246, 192}, 28);
    queue.strokeRect({72, 146, 486, 112}, {255, 255, 251, 188}, 2);
    queue.fillRoundedRect({96, 174, 132, 8}, coral, 4);
    queue.fillRoundedRect({96, 196, 86, 8}, electricMint, 4);
    queue.fillRoundedRect({366, 170, 156, 18}, stormInk, 9);
    queue.fillRoundedRect({386, 177, 92, 4}, electricMint, 2);
    drawTextBox(queue,
                {206, 154, 176, 34},
                text.get(localization::TextId::HomeBoardTitle),
                nightInk,
                12,
                32);
    drawTextBox(queue,
                {206, 194, 164, 28},
                text.get(localization::TextId::HomeBoardPatch),
                slate,
                12,
                28);
    drawTextBox(queue,
                {370, 194, 160, 28},
                text.get(localization::TextId::HomeBoardCompile),
                slate,
                12,
                28);
    drawTextBox(queue,
                {96, 224, 420, 24},
                text.get(localization::TextId::HomeHarufushiStatus),
                slate,
                10,
                24);
}

void renderSaveSlot(engine::graphics::RenderQueue& queue,
                    engine::graphics::Rect bounds,
                    const std::string& summary,
                    std::size_t index) {
    const auto marker = index == 0 ? coral : electricMint;
    queue.fillRoundedRect({bounds.x + 7, bounds.y + 7, bounds.width, bounds.height},
                          softShadow,
                          18);
    queue.fillRoundedRect(bounds, porcelainSolid, 18);
    queue.strokeRect(bounds, glassLine, 2);
    queue.fillRoundedRect({bounds.x + 18, bounds.y + 14, 12, bounds.height - 28},
                          marker,
                          6);
    drawTextBox(queue,
                {bounds.x + 44, bounds.y + 10, bounds.width - 64, bounds.height - 20},
                summary,
                nightInk,
                14,
                32);
}

void renderSavesPanel(engine::graphics::RenderQueue& queue,
                      const localization::GameText& text,
                      const std::vector<std::string>& saveSummaries) {
    queue.fillRoundedRect({112, 128, 916, 468}, {18, 22, 32, 188}, 34);
    queue.fillRoundedRect({146, 154, 842, 408}, porcelain, 30);
    queue.strokeRect({146, 154, 842, 408}, {255, 255, 251, 210}, 2);
    queue.fillRoundedRect({180, 198, 206, 8}, coral, 4);
    queue.fillRoundedRect({180, 220, 132, 8}, electricMint, 4);
    drawTextBox(queue,
                {174, 164, 486, 42},
                text.get(localization::TextId::SaveFiles),
                nightInk,
                12,
                38);

    if (saveSummaries.empty()) {
        drawTextBox(queue,
                    {224, 286, 520, 36},
                    text.get(localization::TextId::NoSaveDataYet),
                    slate,
                    16,
                    42);
    } else {
        const std::size_t visibleCount = std::min<std::size_t>(saveSummaries.size(), 4);
        for (std::size_t index = 0; index < visibleCount; ++index) {
            renderSaveSlot(queue, saveSlotBounds(index), saveSummaries[index], index);
        }
    }

    renderTitleMenuButton(queue, backButton(text), false);
}

} // namespace

HomeScene::HomeScene(localization::GameText text) : text_(std::move(text)) {}

void HomeScene::render(engine::graphics::RenderQueue& queue,
                       engine::graphics::Size surfaceSize,
                       HomePanel panel,
                       const std::vector<std::string>& saveSummaries) const {
    drawSceneBackground(queue, surfaceSize);
    renderTitleMark(queue, text_);

    if (panel == HomePanel::Saves) {
        renderSavesPanel(queue, text_, saveSummaries);
        return;
    }

    renderPatchBoard(queue, text_);
    renderBottomMenu(queue, text_);
}

std::optional<HomeAction> HomeScene::actionAt(engine::graphics::Point point,
                                              engine::graphics::Size surfaceSize,
                                              HomePanel panel,
                                              std::size_t saveCount) const {
    if (surfaceSize.width <= 0 || surfaceSize.height <= 0) {
        return std::nullopt;
    }

    if (panel != HomePanel::Main && backButton(text_).contains(point)) {
        return HomeAction::Back;
    }

    if (panel == HomePanel::Saves) {
        const std::size_t visibleCount = std::min<std::size_t>(saveCount, 4);
        for (std::size_t index = 0; index < visibleCount; ++index) {
            if (contains(saveSlotBounds(index), point)) {
                return saveSlotAction(index);
            }
        }
        return std::nullopt;
    }

    if (newGameButton(text_).contains(point)) {
        return HomeAction::NewGame;
    }
    if (loadButton(text_).contains(point)) {
        return HomeAction::OpenSaves;
    }
    if (settingsButton(text_).contains(point)) {
        return HomeAction::OpenSettings;
    }
    if (quitButton(text_).contains(point)) {
        return HomeAction::Quit;
    }

    return std::nullopt;
}

} // namespace haru::game::scenes
