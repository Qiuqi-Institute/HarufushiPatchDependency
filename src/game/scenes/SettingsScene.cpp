#include "SettingsScene.hpp"

#include <algorithm>
#include <utility>

namespace haru::game::scenes {

namespace {

constexpr engine::graphics::Color bgCream{251, 248, 241, 255};
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
constexpr engine::graphics::Color panelBlue{239, 249, 250, 255};

engine::ui::ButtonStyle languageButtonStyle() {
    return {sky, deepInk, 12};
}

engine::ui::ButtonStyle backButtonStyle() {
    return {sakuraStrong, deepInk, 12};
}

engine::ui::Button englishButton(const localization::GameText& text) {
    return {{456, 288, 368, 48},
            text.get(localization::TextId::LanguageEnglish),
            languageButtonStyle()};
}

engine::ui::Button chineseButton(const localization::GameText& text) {
    return {{456, 352, 368, 48},
            text.get(localization::TextId::LanguageSimplifiedChinese),
            languageButtonStyle()};
}

engine::ui::Button japaneseButton(const localization::GameText& text) {
    return {{456, 416, 368, 48},
            text.get(localization::TextId::LanguageJapanese),
            languageButtonStyle()};
}

engine::ui::Button backButton(const localization::GameText& text) {
    return {{72, 596, 224, 44}, text.get(localization::TextId::Back), backButtonStyle()};
}

void renderAdaptiveText(engine::graphics::RenderQueue& queue,
                        engine::graphics::Rect bounds,
                        const std::string& value,
                        engine::graphics::Color color,
                        int maxPadding = 72) {
    engine::ui::TextBoxStyle style;
    style.text = color;
    style.minHorizontalPadding = 18;
    style.maxHorizontalPadding = maxPadding;
    engine::ui::TextBox(bounds, value, style).render(queue);
}

void renderButton(engine::graphics::RenderQueue& queue,
                  const engine::ui::Button& button,
                  engine::graphics::Color fill) {
    const auto& bounds = button.bounds();
    queue.fillRoundedRect({bounds.x + 8, bounds.y + 8, bounds.width, bounds.height},
                          shadow,
                          18);
    queue.fillRoundedRect(bounds, fill, 18);
    queue.strokeRect(bounds, warmWhite, 2);
    queue.fillRoundedRect({bounds.x + 16, bounds.y + 8, 48, 8}, warmWhite, 4);
    queue.fillEllipse({bounds.x + bounds.width - 34, bounds.y + 14, 14, 14}, warmWhite);
    renderAdaptiveText(queue,
                       {bounds.x + 28, bounds.y + 11, bounds.width - 56, bounds.height - 18},
                       button.label(),
                       deepInk,
                       42);
}

void drawSettingRail(engine::graphics::RenderQueue& queue,
                     engine::graphics::Rect bounds,
                     engine::graphics::Color fill,
                     int value) {
    queue.fillRoundedRect(bounds, {235, 230, 224, 255}, bounds.height / 2);
    const int fillWidth = std::max((bounds.width * std::clamp(value, 0, 100)) / 100, 1);
    queue.fillRoundedRect({bounds.x, bounds.y, fillWidth, bounds.height},
                          fill,
                          bounds.height / 2);
    queue.fillEllipse({bounds.x + fillWidth - 10, bounds.y - 5, 22, 22}, warmWhite);
    queue.strokeRect({bounds.x + fillWidth - 10, bounds.y - 5, 22, 22}, fill, 2);
}

} // namespace

SettingsScene::SettingsScene(localization::GameText text) : text_(std::move(text)) {}

void SettingsScene::render(engine::graphics::RenderQueue& queue,
                           engine::graphics::Size surfaceSize) const {
    const int width = std::max(surfaceSize.width, 1);
    const int height = std::max(surfaceSize.height, 1);

    queue.clear(bgCream);
    queue.fillVerticalGradient({0, 0, width, height}, {255, 247, 250, 255}, {232, 249, 250, 255});
    queue.fillRoundedRect({-40, 28, width + 80, 96}, sky, 42);
    queue.fillRoundedRect({88, 72, 640, 82}, warmWhite, 26);
    queue.strokeRect({88, 72, 640, 82}, sakuraStrong, 3);
    queue.fillRoundedRect({110, 126, 122, 8}, gold, 4);
    queue.fillRoundedRect({248, 126, 56, 8}, sakuraStrong, 4);
    renderAdaptiveText(queue,
                       {108, 88, 600, 52},
                       text_.get(localization::TextId::GameTitle),
                       ink);

    queue.fillRoundedRect({136, 188, std::max(width - 272, 1), 376}, warmWhite, 34);
    queue.fillRoundedRect({154, 206, std::max(width - 308, 1), 340}, panelBlue, 28);
    queue.strokeRect({136, 188, std::max(width - 272, 1), 376}, sakura, 3);
    queue.fillRoundedRect({178, 226, 186, 10}, sakuraStrong, 5);
    queue.fillRoundedRect({178, 250, 128, 10}, mint, 5);
    queue.fillEllipse({width - 290, 224, 104, 104}, softLilac);
    queue.fillEllipse({width - 260, 250, 44, 44}, sakura);
    queue.fillRoundedRect({width - 272, 346, 132, 12}, sky, 6);

    renderAdaptiveText(queue,
                       {188, 258, 212, 36},
                       text_.get(localization::TextId::Settings),
                       ink,
                       48);
    renderAdaptiveText(queue,
                       {188, 318, 212, 30},
                       text_.get(localization::TextId::Language),
                       deepInk,
                       36);

    renderButton(queue, englishButton(text_), sky);
    renderButton(queue, chineseButton(text_), sky);
    renderButton(queue, japaneseButton(text_), sky);

    renderAdaptiveText(queue,
                       {884, 292, 220, 28},
                       text_.get(localization::TextId::Audio80),
                       deepInk,
                       32);
    drawSettingRail(queue, {884, 330, 236, 12}, sakuraStrong, 80);
    renderAdaptiveText(queue,
                       {884, 372, 220, 28},
                       text_.get(localization::TextId::Visual100),
                       deepInk,
                       32);
    drawSettingRail(queue, {884, 410, 236, 12}, mint, 100);
    renderAdaptiveText(queue,
                       {884, 452, 248, 28},
                       text_.get(localization::TextId::TextSpeedNormal),
                       deepInk,
                       32);

    renderButton(queue, backButton(text_), sakuraStrong);
}

std::optional<SettingsAction> SettingsScene::actionAt(engine::graphics::Point point,
                                                      engine::graphics::Size surfaceSize) const {
    if (surfaceSize.width <= 0 || surfaceSize.height <= 0) {
        return std::nullopt;
    }

    if (englishButton(text_).contains(point)) {
        return SettingsAction::SetLocaleEnglish;
    }
    if (chineseButton(text_).contains(point)) {
        return SettingsAction::SetLocaleSimplifiedChinese;
    }
    if (japaneseButton(text_).contains(point)) {
        return SettingsAction::SetLocaleJapanese;
    }
    if (backButton(text_).contains(point)) {
        return SettingsAction::Back;
    }

    return std::nullopt;
}

} // namespace haru::game::scenes
