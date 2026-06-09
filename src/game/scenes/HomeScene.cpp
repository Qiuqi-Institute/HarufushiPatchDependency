#include "HomeScene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>
#include <vector>

namespace haru::game::scenes {

namespace {

constexpr const char* homeBackgroundResourceId = "images.backgrounds.home_chunfu";
constexpr engine::graphics::Color nightInk{49, 59, 82, 255};
constexpr engine::graphics::Color slate{67, 76, 104, 255};
constexpr engine::graphics::Color porcelain{246, 250, 246, 242};
constexpr engine::graphics::Color porcelainSolid{246, 250, 246, 255};
constexpr engine::graphics::Color pearl{255, 255, 251, 255};
constexpr engine::graphics::Color logoBlue{11, 119, 155, 230};
constexpr engine::graphics::Color logoBlueSoft{185, 226, 232, 220};
constexpr engine::graphics::Color logoPink{255, 183, 205, 230};
constexpr engine::graphics::Color glassLine{185, 226, 232, 210};
constexpr engine::graphics::Color softShadow{67, 76, 104, 128};
constexpr engine::graphics::Size designSurface{1280, 720};

engine::ui::ButtonStyle titleMenuStyle(engine::graphics::Color background) {
    return {background, pearl, 10};
}

int scaledHorizontal(int value, engine::graphics::Size surfaceSize) {
    const int width = std::max(surfaceSize.width, 1);
    return static_cast<int>(std::lround(static_cast<double>(value) *
                                        static_cast<double>(width) /
                                        static_cast<double>(designSurface.width)));
}

int scaledVertical(int value, engine::graphics::Size surfaceSize) {
    const int height = std::max(surfaceSize.height, 1);
    return static_cast<int>(std::lround(static_cast<double>(value) *
                                        static_cast<double>(height) /
                                        static_cast<double>(designSurface.height)));
}

engine::graphics::Rect scaledRect(engine::graphics::Rect rect,
                                  engine::graphics::Size surfaceSize) {
    const int left = scaledHorizontal(rect.x, surfaceSize);
    const int top = scaledVertical(rect.y, surfaceSize);
    const int right = scaledHorizontal(rect.x + rect.width, surfaceSize);
    const int bottom = scaledVertical(rect.y + rect.height, surfaceSize);
    return {left, top, std::max(right - left, 1), std::max(bottom - top, 1)};
}

engine::graphics::Rect newGameBounds(engine::graphics::Size surfaceSize) {
    return scaledRect({72, 676, 236, 38}, surfaceSize);
}

engine::graphics::Rect loadBounds(engine::graphics::Size surfaceSize) {
    return scaledRect({372, 676, 236, 38}, surfaceSize);
}

engine::graphics::Rect settingsBounds(engine::graphics::Size surfaceSize) {
    return scaledRect({672, 676, 236, 38}, surfaceSize);
}

engine::graphics::Rect quitBounds(engine::graphics::Size surfaceSize) {
    return scaledRect({972, 676, 236, 38}, surfaceSize);
}

engine::graphics::Rect backBounds(engine::graphics::Size surfaceSize) {
    return scaledRect({80, 634, 232, 48}, surfaceSize);
}

engine::ui::Button newGameButton(const localization::GameText& text,
                                 engine::graphics::Size surfaceSize) {
    return {newGameBounds(surfaceSize),
            text.get(localization::TextId::NewGame),
            titleMenuStyle({0, 0, 0, 0})};
}

engine::ui::Button loadButton(const localization::GameText& text,
                              engine::graphics::Size surfaceSize) {
    return {loadBounds(surfaceSize),
            text.get(localization::TextId::Load),
            titleMenuStyle({0, 0, 0, 0})};
}

engine::ui::Button settingsButton(const localization::GameText& text,
                                  engine::graphics::Size surfaceSize) {
    return {settingsBounds(surfaceSize),
            text.get(localization::TextId::Settings),
            titleMenuStyle({0, 0, 0, 0})};
}

engine::ui::Button quitButton(const localization::GameText& text,
                              engine::graphics::Size surfaceSize) {
    return {quitBounds(surfaceSize),
            text.get(localization::TextId::Quit),
            titleMenuStyle({0, 0, 0, 0})};
}

engine::ui::Button backButton(const localization::GameText& text,
                              engine::graphics::Size surfaceSize) {
    return {backBounds(surfaceSize),
            text.get(localization::TextId::Back),
            titleMenuStyle({0, 0, 0, 0})};
}

engine::graphics::Rect saveSlotBounds(std::size_t index,
                                      engine::graphics::Size surfaceSize) {
    return scaledRect({224, 264 + (static_cast<int>(index) * 64), 640, 48},
                      surfaceSize);
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
                 engine::graphics::Size surfaceSize,
                 int minPadding = 16,
                 int maxPadding = 56) {
    engine::ui::TextBoxStyle style;
    style.text = color;
    style.minHorizontalPadding = scaledHorizontal(minPadding, surfaceSize);
    style.maxHorizontalPadding = scaledHorizontal(maxPadding, surfaceSize);
    engine::ui::TextBox(bounds, value, style).render(queue);
}

void drawSceneBackground(engine::graphics::RenderQueue& queue,
                         engine::graphics::Size surfaceSize) {
    const int width = std::max(surfaceSize.width, 1);
    const int height = std::max(surfaceSize.height, 1);
    queue.clear({0, 0, 0, 0});
    queue.drawImage({0, 0, width, height}, homeBackgroundResourceId);
}

void renderTitleMenuButton(engine::graphics::RenderQueue& queue,
                           const engine::ui::Button& button,
                           engine::graphics::Size surfaceSize) {
    const auto& bounds = button.bounds();
    const bool needsTallBounds =
        std::any_of(button.label().begin(), button.label().end(), [](unsigned char byte) {
            return byte >= 0x80;
        });
    const int textY = needsTallBounds ? bounds.y - scaledVertical(4, surfaceSize)
                                      : bounds.y + scaledVertical(8, surfaceSize);
    const int textHeight = std::max(1, scaledVertical(needsTallBounds ? 42 : 28,
                                                      surfaceSize));
    queue.drawText({bounds.x, textY, bounds.width, textHeight},
                   button.label(),
                   pearl,
                   engine::graphics::TextRole::ZenMaruBlack,
                   needsTallBounds ? 67 : 100);
}

void renderBottomMenu(engine::graphics::RenderQueue& queue,
                      const localization::GameText& text,
                      engine::graphics::Size surfaceSize) {
    queue.fillVerticalGradient(scaledRect({0, 520, 1280, 200}, surfaceSize),
                               {33, 47, 75, 0},
                               {33, 47, 75, 176});
    renderTitleMenuButton(queue, newGameButton(text, surfaceSize), surfaceSize);
    renderTitleMenuButton(queue, loadButton(text, surfaceSize), surfaceSize);
    renderTitleMenuButton(queue, settingsButton(text, surfaceSize), surfaceSize);
    renderTitleMenuButton(queue, quitButton(text, surfaceSize), surfaceSize);
}

void renderSaveSlot(engine::graphics::RenderQueue& queue,
                    engine::graphics::Rect bounds,
                    const std::string& summary,
                    std::size_t index,
                    engine::graphics::Size surfaceSize) {
    const auto marker = index == 0 ? logoPink : logoBlue;
    queue.fillRoundedRect({bounds.x + scaledHorizontal(7, surfaceSize),
                           bounds.y + scaledVertical(7, surfaceSize),
                           bounds.width,
                           bounds.height},
                          softShadow,
                          scaledVertical(18, surfaceSize));
    queue.fillRoundedRect(bounds, porcelainSolid, scaledVertical(18, surfaceSize));
    queue.strokeRect(bounds, glassLine, std::max(1, scaledHorizontal(2, surfaceSize)));
    queue.fillRoundedRect({bounds.x + scaledHorizontal(18, surfaceSize),
                           bounds.y + scaledVertical(14, surfaceSize),
                           std::max(1, scaledHorizontal(12, surfaceSize)),
                           std::max(1, bounds.height - scaledVertical(28, surfaceSize))},
                          marker,
                          scaledVertical(6, surfaceSize));
    drawTextBox(queue,
                {bounds.x + scaledHorizontal(44, surfaceSize),
                 bounds.y + scaledVertical(10, surfaceSize),
                 std::max(1, bounds.width - scaledHorizontal(64, surfaceSize)),
                 std::max(1, bounds.height - scaledVertical(20, surfaceSize))},
                summary,
                nightInk,
                surfaceSize,
                14,
                32);
}

void renderSavesPanel(engine::graphics::RenderQueue& queue,
                      const localization::GameText& text,
                      const std::vector<std::string>& saveSummaries,
                      engine::graphics::Size surfaceSize) {
    queue.fillRoundedRect(scaledRect({112, 128, 916, 468}, surfaceSize),
                          {18, 22, 32, 188},
                          scaledVertical(34, surfaceSize));
    queue.fillRoundedRect(scaledRect({146, 154, 842, 408}, surfaceSize),
                          porcelain,
                          scaledVertical(30, surfaceSize));
    queue.strokeRect(scaledRect({146, 154, 842, 408}, surfaceSize),
                     {255, 255, 251, 210},
                     std::max(1, scaledHorizontal(2, surfaceSize)));
    queue.fillRoundedRect(scaledRect({180, 198, 206, 8}, surfaceSize),
                          logoPink,
                          scaledVertical(4, surfaceSize));
    queue.fillRoundedRect(scaledRect({180, 220, 132, 8}, surfaceSize),
                          logoBlueSoft,
                          scaledVertical(4, surfaceSize));
    drawTextBox(queue,
                scaledRect({174, 164, 486, 42}, surfaceSize),
                text.get(localization::TextId::SaveFiles),
                nightInk,
                surfaceSize,
                12,
                38);

    if (saveSummaries.empty()) {
        drawTextBox(queue,
                    scaledRect({224, 286, 520, 36}, surfaceSize),
                    text.get(localization::TextId::NoSaveDataYet),
                    slate,
                    surfaceSize,
                    16,
                    42);
    } else {
        const std::size_t visibleCount = std::min<std::size_t>(saveSummaries.size(), 4);
        for (std::size_t index = 0; index < visibleCount; ++index) {
            renderSaveSlot(queue,
                           saveSlotBounds(index, surfaceSize),
                           saveSummaries[index],
                           index,
                           surfaceSize);
        }
    }

    renderTitleMenuButton(queue, backButton(text, surfaceSize), surfaceSize);
}

} // namespace

HomeScene::HomeScene(localization::GameText text) : text_(std::move(text)) {}

void HomeScene::render(engine::graphics::RenderQueue& queue,
                       engine::graphics::Size surfaceSize,
                       HomePanel panel,
                       const std::vector<std::string>& saveSummaries) const {
    drawSceneBackground(queue, surfaceSize);

    if (panel == HomePanel::Saves) {
        renderSavesPanel(queue, text_, saveSummaries, surfaceSize);
        return;
    }

    renderBottomMenu(queue, text_, surfaceSize);
}

std::optional<HomeAction> HomeScene::actionAt(engine::graphics::Point point,
                                              engine::graphics::Size surfaceSize,
                                              HomePanel panel,
                                              std::size_t saveCount) const {
    if (surfaceSize.width <= 0 || surfaceSize.height <= 0) {
        return std::nullopt;
    }

    if (panel != HomePanel::Main && backButton(text_, surfaceSize).contains(point)) {
        return HomeAction::Back;
    }

    if (panel == HomePanel::Saves) {
        const std::size_t visibleCount = std::min<std::size_t>(saveCount, 4);
        for (std::size_t index = 0; index < visibleCount; ++index) {
            if (contains(saveSlotBounds(index, surfaceSize), point)) {
                return saveSlotAction(index);
            }
        }
        return std::nullopt;
    }

    if (newGameButton(text_, surfaceSize).contains(point)) {
        return HomeAction::NewGame;
    }
    if (loadButton(text_, surfaceSize).contains(point)) {
        return HomeAction::OpenSaves;
    }
    if (settingsButton(text_, surfaceSize).contains(point)) {
        return HomeAction::OpenSettings;
    }
    if (quitButton(text_, surfaceSize).contains(point)) {
        return HomeAction::Quit;
    }

    return std::nullopt;
}

} // namespace haru::game::scenes
