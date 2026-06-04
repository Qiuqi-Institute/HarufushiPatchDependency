#include "HomeScene.hpp"

#include <algorithm>
#include <array>
#include <utility>
#include <vector>

namespace haru::game::scenes {

namespace {

constexpr const char* homeBackgroundPath = "resources/images/backgrounds/home_chunfu.png";
constexpr engine::graphics::Color nightInk{49, 59, 82, 255};
constexpr engine::graphics::Color slate{67, 76, 104, 255};
constexpr engine::graphics::Color porcelain{246, 250, 246, 242};
constexpr engine::graphics::Color porcelainSolid{246, 250, 246, 255};
constexpr engine::graphics::Color pearl{255, 255, 251, 255};
constexpr engine::graphics::Color logoBlue{11, 119, 155, 230};
constexpr engine::graphics::Color logoBlueSoft{185, 226, 232, 220};
constexpr engine::graphics::Color logoPink{255, 183, 205, 230};
constexpr engine::graphics::Color logoPinkSoft{255, 210, 222, 220};
constexpr engine::graphics::Color glassLine{185, 226, 232, 210};
constexpr engine::graphics::Color softShadow{67, 76, 104, 128};

engine::ui::ButtonStyle titleMenuStyle(engine::graphics::Color background) {
    return {background, pearl, 10};
}

engine::graphics::Rect newGameBounds() {
    return {74, 666, 226, 38};
}

engine::graphics::Rect loadBounds() {
    return {340, 666, 226, 38};
}

engine::graphics::Rect settingsBounds() {
    return {662, 666, 226, 38};
}

engine::graphics::Rect quitBounds() {
    return {1000, 666, 202, 38};
}

engine::graphics::Rect backBounds() {
    return {80, 634, 232, 48};
}

engine::ui::Button newGameButton(const localization::GameText& text) {
    return {newGameBounds(),
            text.get(localization::TextId::NewGame),
            titleMenuStyle({0, 0, 0, 0})};
}

engine::ui::Button loadButton(const localization::GameText& text) {
    return {loadBounds(),
            text.get(localization::TextId::Load),
            titleMenuStyle({0, 0, 0, 0})};
}

engine::ui::Button settingsButton(const localization::GameText& text) {
    return {settingsBounds(),
            text.get(localization::TextId::Settings),
            titleMenuStyle({0, 0, 0, 0})};
}

engine::ui::Button quitButton(const localization::GameText& text) {
    return {quitBounds(),
            text.get(localization::TextId::Quit),
            titleMenuStyle({0, 0, 0, 0})};
}

engine::ui::Button backButton(const localization::GameText& text) {
    return {backBounds(),
            text.get(localization::TextId::Back),
            titleMenuStyle({0, 0, 0, 0})};
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

void drawSceneBackground(engine::graphics::RenderQueue& queue,
                         engine::graphics::Size surfaceSize) {
    const int width = std::max(surfaceSize.width, 1);
    const int height = std::max(surfaceSize.height, 1);
    queue.clear({0, 0, 0, 0});
    queue.drawImage({0, 0, width, height}, homeBackgroundPath);
}

void drawLogoStroke(engine::graphics::RenderQueue& queue,
                    engine::graphics::Rect rect,
                    engine::graphics::Color color) {
    queue.fillRoundedRect({rect.x + 5, rect.y + 6, rect.width, rect.height},
                          {255, 255, 255, 212},
                          std::max(rect.height / 3, 2));
    queue.fillRoundedRect(rect, color, std::max(rect.height / 3, 2));
}

std::vector<engine::graphics::Point> shiftedPolygon(
    const std::vector<engine::graphics::Point>& points,
    int dx,
    int dy) {
    std::vector<engine::graphics::Point> shifted;
    shifted.reserve(points.size());
    for (const auto point : points) {
        shifted.push_back({point.x + dx, point.y + dy});
    }
    return shifted;
}

engine::graphics::Rect shiftedRect(engine::graphics::Rect rect, int dx, int dy) {
    rect.x += dx;
    rect.y += dy;
    return rect;
}

void drawLogoPolygon(engine::graphics::RenderQueue& queue,
                     std::vector<engine::graphics::Point> points,
                     engine::graphics::Color color) {
    queue.fillPolygon(shiftedPolygon(points, 5, 6), {255, 255, 255, 205});
    queue.fillPolygon(std::move(points), color);
}

void renderArtTitleMark(engine::graphics::RenderQueue& queue) {
    constexpr int logoOffsetX = -92;
    constexpr int logoOffsetY = 2;
    constexpr int subtitleOffsetY = 36;
    const auto artRect = [=](engine::graphics::Rect rect) {
        return shiftedRect(rect, logoOffsetX, logoOffsetY);
    };
    const auto artPolygon =
        [&](std::vector<engine::graphics::Point> points, engine::graphics::Color color) {
            drawLogoPolygon(queue, shiftedPolygon(points, logoOffsetX, logoOffsetY), color);
        };
    const auto subtitleRect = [=](engine::graphics::Rect rect) {
        return shiftedRect(rect, logoOffsetX, logoOffsetY + subtitleOffsetY);
    };
    const auto subtitlePolygon =
        [&](std::vector<engine::graphics::Point> points, engine::graphics::Color color) {
            drawLogoPolygon(
                queue,
                shiftedPolygon(points, logoOffsetX, logoOffsetY + subtitleOffsetY),
                color);
        };
    const auto subtitleStroke = [&](engine::graphics::Rect rect, engine::graphics::Color color) {
        drawLogoStroke(queue, subtitleRect(rect), color);
    };
    const auto artStroke = [&](engine::graphics::Rect rect, engine::graphics::Color color) {
        drawLogoStroke(queue, artRect(rect), color);
    };

    queue.fillEllipse(artRect({890, 432, 342, 142}), {255, 255, 255, 66});
    queue.fillEllipse(artRect({966, 462, 230, 92}), {185, 226, 232, 56});
    queue.fillEllipse(artRect({916, 520, 246, 82}), {255, 210, 222, 46});

    // ハ
    artPolygon({{936, 452}, {956, 458}, {928, 534}, {908, 528}}, logoBlue);
    artPolygon({{976, 452}, {996, 456}, {1020, 532}, {998, 536}}, logoBlue);

    // ル
    artStroke({1030, 454, 15, 66}, logoPink);
    artPolygon({{1064, 456}, {1082, 459}, {1074, 528}, {1056, 524}}, logoPink);
    artPolygon({{1028, 512}, {1088, 498}, {1092, 516}, {1034, 536}}, logoPink);

    // フ
    artStroke({1100, 456, 68, 13}, logoBlue);
    artPolygon({{1150, 466}, {1168, 474}, {1124, 536}, {1106, 526}}, logoBlue);

    // シ
    artPolygon({{1190, 462}, {1220, 468}, {1218, 480}, {1188, 474}}, logoPink);
    artPolygon({{1182, 488}, {1214, 494}, {1212, 506}, {1180, 500}}, logoPink);
    artPolygon({{1174, 532}, {1222, 498}, {1228, 512}, {1182, 546}}, logoPink);

    // パ
    subtitlePolygon({{940, 532}, {958, 536}, {938, 594}, {920, 590}}, logoPink);
    subtitlePolygon({{972, 530}, {990, 534}, {1010, 592}, {990, 596}}, logoPink);
    queue.fillEllipse(subtitleRect({1004, 526, 12, 12}), {255, 255, 255, 205});
    queue.fillEllipse(subtitleRect({1000, 522, 12, 12}), logoPink);
    queue.fillEllipse(subtitleRect({1018, 526, 10, 10}), {255, 255, 255, 205});
    queue.fillEllipse(subtitleRect({1015, 522, 10, 10}), logoPink);

    // ッ
    subtitlePolygon({{1044, 548}, {1062, 552}, {1058, 568}, {1040, 564}}, logoBlue);
    subtitlePolygon({{1070, 542}, {1088, 546}, {1082, 568}, {1064, 564}}, logoBlue);
    subtitlePolygon({{1090, 546}, {1108, 554}, {1078, 596}, {1060, 588}}, logoBlue);

    // チ
    subtitleStroke({1128, 530, 74, 12}, logoPink);
    subtitleStroke({1118, 556, 84, 12}, logoPink);
    subtitlePolygon({{1160, 538}, {1178, 541}, {1166, 604}, {1148, 600}}, logoPink);

}

void renderTitleMenuButton(engine::graphics::RenderQueue& queue,
                           const engine::ui::Button& button) {
    const auto& bounds = button.bounds();
    drawTextBox(queue,
                {bounds.x + 24, bounds.y + 3, bounds.width - 48, 28},
                button.label(),
                pearl,
                10,
                32);
}

void renderBottomMenu(engine::graphics::RenderQueue& queue,
                      const localization::GameText& text) {
    queue.fillVerticalGradient({0, 520, 1280, 200},
                               {33, 47, 75, 0},
                               {33, 47, 75, 176});
    renderTitleMenuButton(queue, newGameButton(text));
    renderTitleMenuButton(queue, loadButton(text));
    renderTitleMenuButton(queue, settingsButton(text));
    renderTitleMenuButton(queue, quitButton(text));
}

void renderSaveSlot(engine::graphics::RenderQueue& queue,
                    engine::graphics::Rect bounds,
                    const std::string& summary,
                    std::size_t index) {
    const auto marker = index == 0 ? logoPink : logoBlue;
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
    queue.fillRoundedRect({180, 198, 206, 8}, logoPink, 4);
    queue.fillRoundedRect({180, 220, 132, 8}, logoBlueSoft, 4);
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

    renderTitleMenuButton(queue, backButton(text));
}

} // namespace

HomeScene::HomeScene(localization::GameText text) : text_(std::move(text)) {}

void HomeScene::render(engine::graphics::RenderQueue& queue,
                       engine::graphics::Size surfaceSize,
                       HomePanel panel,
                       const std::vector<std::string>& saveSummaries) const {
    drawSceneBackground(queue, surfaceSize);

    if (panel == HomePanel::Saves) {
        renderArtTitleMark(queue);
        renderSavesPanel(queue, text_, saveSummaries);
        return;
    }

    renderBottomMenu(queue, text_);
    renderArtTitleMark(queue);
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
