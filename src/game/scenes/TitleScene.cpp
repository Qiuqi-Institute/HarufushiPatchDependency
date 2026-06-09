#include "TitleScene.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace haru::game::scenes {

namespace {

constexpr engine::graphics::Color bgCream{250, 248, 242, 255};
constexpr engine::graphics::Color paper{255, 252, 248, 255};
constexpr engine::graphics::Color paperBlue{239, 249, 250, 255};
constexpr engine::graphics::Color ink{67, 76, 104, 255};
constexpr engine::graphics::Color deepInk{86, 78, 122, 255};
constexpr engine::graphics::Color sky{185, 226, 232, 255};
constexpr engine::graphics::Color sakura{255, 210, 222, 255};
constexpr engine::graphics::Color sakuraStrong{255, 183, 205, 255};
constexpr engine::graphics::Color mint{202, 234, 219, 255};
constexpr engine::graphics::Color gold{230, 198, 128, 255};
constexpr engine::graphics::Color shadow{224, 216, 210, 255};
constexpr engine::graphics::Color warmWhite{255, 250, 246, 255};
constexpr engine::graphics::Color softLilac{226, 221, 249, 255};
constexpr engine::graphics::Color disabledFill{226, 224, 220, 255};
constexpr engine::graphics::Color disabledText{132, 132, 140, 255};

engine::ui::ButtonStyle primaryButtonStyle() {
    return {sakuraStrong, deepInk, 12};
}

engine::ui::ButtonStyle secondaryButtonStyle() {
    return {sky, deepInk, 12};
}

engine::ui::Button studyButton(const localization::GameText& text) {
    return {{72, 232, 284, 48}, text.get(localization::TextId::Study), secondaryButtonStyle()};
}

engine::ui::Button moddingButton(const localization::GameText& text) {
    return {{72, 296, 284, 48}, text.get(localization::TextId::Modding), primaryButtonStyle()};
}

engine::ui::Button harufushiButton(const localization::GameText& text) {
    return {{72, 360, 284, 48},
            text.get(localization::TextId::Harufushi),
            secondaryButtonStyle()};
}

engine::ui::Button restButton(const localization::GameText& text) {
    return {{72, 424, 284, 48}, text.get(localization::TextId::Rest), secondaryButtonStyle()};
}

engine::ui::Button homeButton(const localization::GameText& text, int width) {
    return {{std::max(width - 224, 1), 72, 160, 48},
            text.get(localization::TextId::ReturnHome),
            secondaryButtonStyle()};
}

void drawPaper(engine::graphics::RenderQueue& queue,
               engine::graphics::Rect rect,
               engine::graphics::Color fill,
               engine::graphics::Color accent) {
    queue.fillRoundedRect({rect.x + 12, rect.y + 14, rect.width, rect.height}, shadow, 28);
    queue.fillRoundedRect(rect, fill, 28);
    queue.strokeRect(rect, accent, 3);
    queue.fillRoundedRect({rect.x + 22, rect.y + 18, rect.width - 44, 10}, mint, 5);
    queue.fillRoundedRect({rect.x + 32, rect.y + rect.height - 30, rect.width - 64, 8},
                          accent,
                          4);
}

void renderButton(engine::graphics::RenderQueue& queue,
                  const engine::ui::Button& button,
                  engine::graphics::Color fill,
                  bool available = true) {
    const auto& bounds = button.bounds();
    queue.fillRoundedRect({bounds.x + 8, bounds.y + 8, bounds.width, bounds.height},
                          shadow,
                          18);
    queue.fillRoundedRect(bounds, available ? fill : disabledFill, 18);
    queue.strokeRect(bounds, warmWhite, 2);
    queue.fillRoundedRect({bounds.x + 16, bounds.y + 8, 48, 8},
                          available ? warmWhite : shadow,
                          4);
    queue.fillEllipse({bounds.x + bounds.width - 34, bounds.y + 14, 14, 14},
                      available ? warmWhite : shadow);
    engine::ui::TextBoxStyle textStyle;
    textStyle.text = available ? deepInk : disabledText;
    textStyle.minHorizontalPadding = 18;
    textStyle.maxHorizontalPadding = 42;
    engine::ui::TextBox({bounds.x + 28,
                         bounds.y + 11,
                         bounds.width - 56,
                         bounds.height - 18},
                        button.label(),
                        textStyle)
        .render(queue);
}

void drawProgressBar(engine::graphics::RenderQueue& queue,
                     engine::graphics::Rect bounds,
                     int value,
                     engine::graphics::Color fill) {
    queue.fillRoundedRect(bounds, {235, 230, 224, 255}, bounds.height / 2);
    const int fillWidth = std::max((bounds.width * std::clamp(value, 0, 100)) / 100, 1);
    queue.fillRoundedRect({bounds.x, bounds.y, fillWidth, bounds.height},
                          fill,
                           bounds.height / 2);
}

void renderStatusPanel(engine::graphics::RenderQueue& queue,
                       const localization::GameText& text,
                       int width,
                       int height,
                       const systems::DailyStats& stats) {
    queue.fillRoundedRect({92, std::max(height - 150, 1), 168, 38}, sakuraStrong, 18);
    queue.strokeRect({92, std::max(height - 150, 1), 168, 38}, warmWhite, 2);
    queue.drawText({108, std::max(height - 143, 1), 136, 24},
                   text.get(localization::TextId::Harufushi),
                   deepInk);
    queue.drawText({286, std::max(height - 126, 1), std::max(width - 372, 1), 32},
                   text.formatDailyStats(stats),
                   ink);
}

void renderDialoguePanel(engine::graphics::RenderQueue& queue,
                         int width,
                         int height,
                         const systems::DailyDialogueEntry& dialogue,
                         std::size_t dialoguePage) {
    const int panelY = std::max(height - 150, 1);
    queue.fillRoundedRect({92, panelY, 208, 38}, sakuraStrong, 18);
    queue.strokeRect({92, panelY, 208, 38}, warmWhite, 2);
    queue.drawText({112, panelY + 7, 168, 24}, dialogue.speaker, deepInk);

    const int textX = 326;
    const int textWidth = std::max(width - textX - 112, 1);
    const std::size_t firstLine = dialoguePage * 3U;
    if (firstLine < dialogue.lines.size()) {
        queue.drawText({textX, panelY + 8, textWidth, 26},
                       dialogue.lines[firstLine],
                       ink);
    }
    if (firstLine + 1U < dialogue.lines.size()) {
        queue.drawText({textX, panelY + 38, textWidth, 26},
                       dialogue.lines[firstLine + 1U],
                       deepInk);
    }
    if (firstLine + 2U < dialogue.lines.size()) {
        queue.drawText({textX, panelY + 68, textWidth, 26},
                       dialogue.lines[firstLine + 2U],
                       ink);
    }
    const std::size_t pageCount = std::max<std::size_t>((dialogue.lines.size() + 2U) / 3U, 1U);
    const std::string pageText = std::to_string(std::min(dialoguePage + 1U, pageCount)) +
                                 "/" + std::to_string(pageCount);
    queue.drawText({std::max(width - 180, 1), panelY + 66, 64, 24}, pageText, disabledText);
    queue.fillRoundedRect({std::max(width - 190, 1), panelY + 96, 76, 8}, sky, 4);
    queue.fillEllipse({std::max(width - 98, 1), panelY + 90, 14, 14}, sakuraStrong);
}

} // namespace

TitleScene::TitleScene(localization::GameText text) : text_(std::move(text)) {}

void TitleScene::render(engine::graphics::RenderQueue& queue,
                        engine::graphics::Size surfaceSize,
                        const systems::DailyStats& stats,
                        const systems::DailyDialogueEntry* activeDialogue,
                        std::size_t dialoguePage) const {
    queue.clear(bgCream);

    const int width = std::max(surfaceSize.width, 1);
    const int height = std::max(surfaceSize.height, 1);
    const int stageWidth = std::max(width - 488, 1);

    queue.fillVerticalGradient({0, 0, width, height}, {255, 247, 250, 255}, {232, 249, 250, 255});
    queue.fillRoundedRect({-40, 28, width + 80, 96}, sky, 42);

    drawPaper(queue, {48, 170, 340, 306}, paperBlue, sakura);
    drawPaper(queue, {424, 148, stageWidth, 350}, paper, sky);
    drawPaper(queue, {64, std::max(height - 168, 1), std::max(width - 128, 1), 118},
              paper,
              sakura);

    queue.fillRoundedRect({96, 72, 640, 82}, warmWhite, 26);
    queue.strokeRect({96, 72, 640, 82}, sakuraStrong, 3);
    queue.fillRoundedRect({118, 126, 122, 8}, gold, 4);
    queue.fillRoundedRect({256, 126, 56, 8}, sakuraStrong, 4);
    engine::ui::TextBoxStyle titleTextStyle;
    titleTextStyle.text = ink;
    titleTextStyle.minHorizontalPadding = 18;
    titleTextStyle.maxHorizontalPadding = 72;
    engine::ui::TextBox({116, 88, 600, 52},
                        text_.get(localization::TextId::GameTitle),
                        titleTextStyle)
        .render(queue);
    renderButton(queue, homeButton(text_, width), sky);

    queue.fillRoundedRect({464, 194, std::max(stageWidth - 92, 1), 52},
                          {231, 243, 250, 255},
                          20);
    queue.fillRoundedRect({width - 236, 184, 112, 126}, softLilac, 34);
    queue.strokeRect({width - 236, 184, 112, 126}, sakura, 2);
    queue.fillEllipse({width - 216, 206, 72, 72}, sakura);
    queue.fillEllipse({width - 194, 230, 28, 28}, warmWhite);
    queue.fillRoundedRect({width - 218, 284, 76, 10}, sky, 5);

    renderButton(queue,
                 studyButton(text_),
                 sky,
                 systems::DailyLoopState::canApply(systems::DailyAction::Study, stats));
    renderButton(queue,
                 moddingButton(text_),
                 sakuraStrong,
                 systems::DailyLoopState::canApply(systems::DailyAction::Modding, stats));
    renderButton(queue,
                 harufushiButton(text_),
                 sky,
                 systems::DailyLoopState::canApply(systems::DailyAction::SpendTimeWithHarufushi,
                                                   stats));
    renderButton(queue,
                 restButton(text_),
                 sky,
                 systems::DailyLoopState::canApply(systems::DailyAction::Rest, stats));

    queue.drawText({492, 208, 360, 34},
                   text_.get(localization::TextId::DailyBoardTitle),
                   ink);
    queue.drawText({492, 244, std::max(stageWidth - 200, 1), 30},
                   text_.get(localization::TextId::DailyEventPreview),
                   deepInk);
    queue.drawText({520, 294, 130, 28}, text_.get(localization::TextId::Energy), ink);
    drawProgressBar(queue, {650, 304, 360, 12}, stats.energy, sky);
    queue.drawText({520, 334, 130, 28}, text_.get(localization::TextId::ModStat), ink);
    drawProgressBar(queue, {650, 344, 360, 12}, stats.modProgress, sakuraStrong);
    queue.drawText({520, 374, 130, 28}, text_.get(localization::TextId::Bond), ink);
    drawProgressBar(queue, {650, 384, 360, 12}, stats.harufushiBond, mint);

    if (activeDialogue != nullptr) {
        renderDialoguePanel(queue, width, height, *activeDialogue, dialoguePage);
    } else {
        renderStatusPanel(queue, text_, width, height, stats);
    }
}

std::optional<systems::DailyAction> TitleScene::actionAt(
    engine::graphics::Point point,
    engine::graphics::Size surfaceSize) const {
    return actionAt(point, surfaceSize, systems::DailyStats{});
}

std::optional<systems::DailyAction> TitleScene::actionAt(
    engine::graphics::Point point,
    engine::graphics::Size surfaceSize,
    const systems::DailyStats& stats) const {
    if (surfaceSize.width <= 0 || surfaceSize.height <= 0) {
        return std::nullopt;
    }

    if (studyButton(text_).contains(point) &&
        systems::DailyLoopState::canApply(systems::DailyAction::Study, stats)) {
        return systems::DailyAction::Study;
    }
    if (moddingButton(text_).contains(point) &&
        systems::DailyLoopState::canApply(systems::DailyAction::Modding, stats)) {
        return systems::DailyAction::Modding;
    }
    if (harufushiButton(text_).contains(point) &&
        systems::DailyLoopState::canApply(systems::DailyAction::SpendTimeWithHarufushi,
                                          stats)) {
        return systems::DailyAction::SpendTimeWithHarufushi;
    }
    if (restButton(text_).contains(point) &&
        systems::DailyLoopState::canApply(systems::DailyAction::Rest, stats)) {
        return systems::DailyAction::Rest;
    }

    return std::nullopt;
}

std::optional<TitleNavigationAction> TitleScene::navigationActionAt(
    engine::graphics::Point point,
    engine::graphics::Size surfaceSize) const {
    if (surfaceSize.width <= 0 || surfaceSize.height <= 0) {
        return std::nullopt;
    }

    if (homeButton(text_, surfaceSize.width).contains(point)) {
        return TitleNavigationAction::ReturnHome;
    }

    return std::nullopt;
}

bool TitleScene::dialogueAdvanceAt(engine::graphics::Point point,
                                   engine::graphics::Size surfaceSize) const {
    if (surfaceSize.width <= 0 || surfaceSize.height <= 0) {
        return false;
    }

    return point.x >= 64 && point.x <= surfaceSize.width - 64 &&
           point.y >= std::max(surfaceSize.height - 168, 1) &&
           point.y <= surfaceSize.height - 50;
}

} // namespace haru::game::scenes
