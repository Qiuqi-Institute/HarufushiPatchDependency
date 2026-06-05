#include "SettingsScene.hpp"

#include <algorithm>
#include <string>
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
constexpr engine::graphics::Color slate{49, 59, 82, 255};
constexpr engine::graphics::Color line{215, 224, 232, 255};

engine::ui::ButtonStyle tabButtonStyle() {
    return {panelBlue, deepInk, 12};
}

engine::ui::ButtonStyle backButtonStyle() {
    return {warmWhite, deepInk, 12};
}

engine::ui::Button englishButton(const localization::GameText& text) {
    return {{468, 228, 132, 40},
            text.get(localization::TextId::LanguageEnglish),
            tabButtonStyle()};
}

engine::ui::Button chineseButton(const localization::GameText& text) {
    return {{612, 228, 132, 40},
            text.get(localization::TextId::LanguageSimplifiedChinese),
            tabButtonStyle()};
}

engine::ui::Button japaneseButton(const localization::GameText& text) {
    return {{756, 228, 132, 40},
            text.get(localization::TextId::LanguageJapanese),
            tabButtonStyle()};
}

engine::ui::Button backButton(const localization::GameText& text) {
    return {{72, 620, 224, 46}, text.get(localization::TextId::Back), backButtonStyle()};
}

void renderAdaptiveText(engine::graphics::RenderQueue& queue,
                        engine::graphics::Rect bounds,
                        const std::string& value,
                        engine::graphics::Color color,
                        int maxPadding = 72,
                        engine::graphics::TextRole textRole =
                            engine::graphics::TextRole::ZenMaruBold) {
    engine::ui::TextBoxStyle style;
    style.text = color;
    style.textRole = textRole;
    style.minHorizontalPadding = 18;
    style.maxHorizontalPadding = maxPadding;
    engine::ui::TextBox(bounds, value, style).render(queue);
}

void renderButton(engine::graphics::RenderQueue& queue,
                  const engine::ui::Button& button,
                  engine::graphics::Color fill,
                  bool selected = false,
                  engine::graphics::TextRole textRole =
                      engine::graphics::TextRole::ZenMaruBold) {
    const auto& bounds = button.bounds();
    queue.fillRoundedRect({bounds.x + 4, bounds.y + 5, bounds.width, bounds.height},
                          {203, 210, 222, 150},
                          12);
    queue.fillRoundedRect(bounds, fill, 12);
    queue.strokeRect(bounds, selected ? sakuraStrong : line, selected ? 3 : 1);
    renderAdaptiveText(queue,
                       {bounds.x + 14, bounds.y + 9, bounds.width - 28, bounds.height - 16},
                       button.label(),
                       selected ? slate : deepInk,
                       28,
                       textRole);
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

engine::graphics::Rect tabRect(SettingsTab tab) {
    switch (tab) {
    case SettingsTab::Game:
        return {430, 82, 132, 44};
    case SettingsTab::Audio:
        return {586, 82, 132, 44};
    case SettingsTab::Display:
        return {742, 82, 132, 44};
    }
    return {430, 82, 132, 44};
}

engine::ui::Button tabButton(const localization::GameText& text, SettingsTab tab) {
    localization::TextId textId = localization::TextId::SettingsTabGame;
    if (tab == SettingsTab::Audio) {
        textId = localization::TextId::SettingsTabAudio;
    } else if (tab == SettingsTab::Display) {
        textId = localization::TextId::SettingsTabDisplay;
    }
    return {tabRect(tab), text.get(textId), tabButtonStyle()};
}

engine::graphics::Rect audioDecreaseRect() {
    return {472, 330, 48, 40};
}

engine::graphics::Rect audioIncreaseRect() {
    return {1004, 330, 48, 40};
}

engine::graphics::Rect scaleDecreaseRect() {
    return {472, 430, 48, 40};
}

engine::graphics::Rect scaleIncreaseRect() {
    return {1004, 430, 48, 40};
}

engine::graphics::Rect speedDecreaseRect() {
    return {472, 530, 48, 40};
}

engine::graphics::Rect speedIncreaseRect() {
    return {1004, 330, 48, 40};
}

engine::graphics::Rect bgmDecreaseRect() {
    return {472, 430, 48, 40};
}

engine::graphics::Rect bgmIncreaseRect() {
    return {1004, 430, 48, 40};
}

engine::graphics::Rect seDecreaseRect() {
    return {472, 530, 48, 40};
}

engine::graphics::Rect seIncreaseRect() {
    return {1004, 530, 48, 40};
}

bool contains(engine::graphics::Rect rect, engine::graphics::Point point) {
    return point.x >= rect.x && point.x < rect.x + rect.width && point.y >= rect.y &&
           point.y < rect.y + rect.height;
}

void renderStepButton(engine::graphics::RenderQueue& queue,
                      engine::graphics::Rect rect,
                      const std::string& label) {
    queue.fillRoundedRect(rect, warmWhite, 10);
    queue.strokeRect(rect, line, 2);
    queue.drawText({rect.x, rect.y + 7, rect.width, rect.height - 14},
                   label,
                   slate,
                   engine::graphics::TextRole::ZenMaruBold);
}

void renderValueRow(engine::graphics::RenderQueue& queue,
                    engine::graphics::Rect labelRect,
                    const std::string& label,
                    int value,
                    engine::graphics::Color accent,
                    engine::graphics::Rect downRect,
                    engine::graphics::Rect upRect) {
    queue.drawText(labelRect, label, ink, engine::graphics::TextRole::ZenMaruBold);
    renderStepButton(queue, downRect, "-");
    drawSettingRail(queue, {548, downRect.y + 15, 420, 10}, accent, value);
    renderStepButton(queue, upRect, "+");
    renderAdaptiveText(queue,
                       {912, downRect.y + 4, 72, 32},
                       std::to_string(value),
                       slate,
                       18,
                       engine::graphics::TextRole::ZenMaruBold);
}

void renderSettingPanel(engine::graphics::RenderQueue& queue,
                        engine::graphics::Rect rect,
                        engine::graphics::Color fill = {255, 252, 248, 255}) {
    queue.fillRoundedRect(rect, fill, 18);
    queue.strokeRect(rect, line, 1);
}

} // namespace

SettingsScene::SettingsScene(localization::GameText text, SettingsState state)
    : text_(std::move(text)), state_(state) {}

void SettingsScene::render(engine::graphics::RenderQueue& queue,
                           engine::graphics::Size surfaceSize) const {
    const int width = std::max(surfaceSize.width, 1);
    const int height = std::max(surfaceSize.height, 1);

    queue.clear(bgCream);
    queue.fillVerticalGradient({0, 0, width, height}, {255, 247, 250, 255}, {232, 249, 250, 255});
    queue.fillRoundedRect({0, 0, width, 146}, {255, 210, 222, 255}, 0);
    queue.fillRoundedRect({0, std::max(height - 92, 1), width, 92}, {232, 249, 250, 255}, 0);
    queue.fillRoundedRect({24, 24, 72, 72}, {185, 226, 232, 255}, 28);
    queue.fillEllipse({44, 44, 32, 32}, sakuraStrong);
    queue.fillRoundedRect({48, 150, std::max(width - 96, 1), std::max(height - 242, 1)},
                          {255, 255, 255, 232},
                          18);
    queue.strokeRect({48, 150, std::max(width - 96, 1), std::max(height - 242, 1)}, line, 2);
    renderAdaptiveText(queue,
                       {98, 52, 300, 54},
                       "System Setting",
                       ink,
                       54,
                       engine::graphics::TextRole::ZenMaruBlack);
    renderAdaptiveText(queue,
                       {900, 82, 280, 38},
                       text_.get(localization::TextId::GameTitle),
                       ink,
                       72,
                       engine::graphics::TextRole::ZenMaruBlack);

    renderButton(queue,
                 tabButton(text_, SettingsTab::Game),
                 state_.activeTab == SettingsTab::Game ? sakura : panelBlue,
                 state_.activeTab == SettingsTab::Game,
                 engine::graphics::TextRole::ZenMaruBlack);
    renderButton(queue,
                 tabButton(text_, SettingsTab::Audio),
                 state_.activeTab == SettingsTab::Audio ? sakura : panelBlue,
                 state_.activeTab == SettingsTab::Audio,
                 engine::graphics::TextRole::ZenMaruBlack);
    renderButton(queue,
                 tabButton(text_, SettingsTab::Display),
                 state_.activeTab == SettingsTab::Display ? sakura : panelBlue,
                 state_.activeTab == SettingsTab::Display,
                 engine::graphics::TextRole::ZenMaruBlack);

    if (state_.activeTab == SettingsTab::Game) {
        renderSettingPanel(queue, {356, 158, std::max(width - 428, 1), 126}, {246, 251, 252, 255});
        queue.drawText({392, 180, 220, 30},
                       text_.get(localization::TextId::Language),
                       deepInk,
                       engine::graphics::TextRole::ZenMaruBold);

        renderButton(queue, englishButton(text_), sky, text_.activeLocale() == "en-US");
        renderButton(queue,
                     chineseButton(text_),
                     sky,
                     text_.activeLocale() == "zh-CN");
        renderButton(queue, japaneseButton(text_), sky, text_.activeLocale() == "ja-JP");

        renderSettingPanel(queue, {356, 308, std::max(width - 428, 1), 88});
        renderValueRow(queue,
                       {392, 310, 220, 22},
                       text_.get(localization::TextId::SettingsTextSpeed),
                       state_.textSpeed,
                       sky,
                       audioDecreaseRect(),
                       audioIncreaseRect());

        renderSettingPanel(queue, {356, 408, std::max(width - 428, 1), 88});
        queue.drawText({392, 416, 220, 24},
                       text_.get(localization::TextId::SettingsSkipMode),
                       ink,
                       engine::graphics::TextRole::ZenMaruBold);
        renderButton(queue,
                     {{536, 432, 180, 40}, text_.get(localization::TextId::SettingsReadOnly),
                      tabButtonStyle()},
                     sky,
                     true);
        renderButton(queue,
                     {{744, 432, 180, 40}, text_.get(localization::TextId::SettingsAllText),
                      tabButtonStyle()},
                     panelBlue);

        renderSettingPanel(queue, {356, 508, std::max(width - 428, 1), 88});
        queue.drawText({392, 516, 220, 24},
                       text_.get(localization::TextId::SettingsAutoMode),
                       ink,
                       engine::graphics::TextRole::ZenMaruBold);
        renderButton(queue,
                     {{536, 532, 180, 40}, "Normal", tabButtonStyle()},
                     sky,
                     true);
        renderButton(queue,
                     {{744, 532, 180, 40}, "Fast", tabButtonStyle()},
                     panelBlue);
    } else if (state_.activeTab == SettingsTab::Audio) {
        renderSettingPanel(queue, {356, 308, std::max(width - 428, 1), 88});
        renderValueRow(queue,
                       {392, 310, 220, 22},
                       text_.get(localization::TextId::SettingsMasterVolume),
                       state_.masterVolume,
                       sakuraStrong,
                       audioDecreaseRect(),
                       audioIncreaseRect());

        renderSettingPanel(queue, {356, 408, std::max(width - 428, 1), 88});
        renderValueRow(queue,
                       {392, 410, 220, 22},
                       text_.get(localization::TextId::SettingsBgmVolume),
                       state_.bgmVolume,
                       mint,
                       bgmDecreaseRect(),
                       bgmIncreaseRect());

        renderSettingPanel(queue, {356, 508, std::max(width - 428, 1), 88});
        renderValueRow(queue,
                       {392, 510, 220, 22},
                       text_.get(localization::TextId::SettingsSeVolume),
                       state_.seVolume,
                       sky,
                       seDecreaseRect(),
                       seIncreaseRect());
    } else if (state_.activeTab == SettingsTab::Display) {
        renderSettingPanel(queue, {356, 158, std::max(width - 428, 1), 126}, {246, 251, 252, 255});
        queue.drawText({392, 180, 220, 30},
                       text_.get(localization::TextId::SettingsAspectRatio),
                       deepInk,
                       engine::graphics::TextRole::ZenMaruBold);
        renderButton(queue,
                     {{468, 228, 132, 40}, "16:9", tabButtonStyle()},
                     sky,
                     true);
        renderButton(queue,
                     {{612, 228, 132, 40}, "4:3", tabButtonStyle()},
                     panelBlue);

        renderSettingPanel(queue, {356, 308, std::max(width - 428, 1), 88});
        renderValueRow(queue,
                       {392, 310, 220, 22},
                       text_.get(localization::TextId::SettingsWindowScale),
                       state_.windowScale,
                       mint,
                       audioDecreaseRect(),
                       audioIncreaseRect());

        renderSettingPanel(queue, {356, 408, std::max(width - 428, 1), 88});
        queue.drawText({392, 416, 220, 24},
                       text_.get(localization::TextId::SettingsDisplayMode),
                       ink,
                       engine::graphics::TextRole::ZenMaruBold);
        renderButton(queue,
                     {{536, 432, 180, 40}, text_.get(localization::TextId::SettingsWindowed),
                      tabButtonStyle()},
                     sky,
                     true);
        renderButton(queue,
                     {{744, 432, 180, 40}, text_.get(localization::TextId::SettingsFullscreen),
                      tabButtonStyle()},
                     panelBlue);
    }

    queue.fillEllipse({width - 220, 88, 92, 92}, softLilac);
    queue.fillEllipse({width - 196, 114, 42, 42}, sakura);
    queue.fillRoundedRect({width - 214, 188, 122, 10}, sky, 5);
    renderButton(queue, backButton(text_), sakura);
}

std::optional<SettingsAction> SettingsScene::actionAt(engine::graphics::Point point,
                                                      engine::graphics::Size surfaceSize) const {
    if (surfaceSize.width <= 0 || surfaceSize.height <= 0) {
        return std::nullopt;
    }

    if (tabButton(text_, SettingsTab::Game).contains(point)) {
        return SettingsAction::SelectGameTab;
    }
    if (tabButton(text_, SettingsTab::Audio).contains(point)) {
        return SettingsAction::SelectAudioTab;
    }
    if (tabButton(text_, SettingsTab::Display).contains(point)) {
        return SettingsAction::SelectDisplayTab;
    }
    if (state_.activeTab == SettingsTab::Game) {
        if (englishButton(text_).contains(point)) {
            return SettingsAction::SetLocaleEnglish;
        }
        if (chineseButton(text_).contains(point)) {
            return SettingsAction::SetLocaleSimplifiedChinese;
        }
        if (japaneseButton(text_).contains(point)) {
            return SettingsAction::SetLocaleJapanese;
        }
        if (contains(audioDecreaseRect(), point)) {
            return SettingsAction::DecreaseTextSpeed;
        }
        if (contains(audioIncreaseRect(), point)) {
            return SettingsAction::IncreaseTextSpeed;
        }
    } else if (state_.activeTab == SettingsTab::Audio) {
        if (contains(audioDecreaseRect(), point)) {
            return SettingsAction::DecreaseMasterVolume;
        }
        if (contains(audioIncreaseRect(), point)) {
            return SettingsAction::IncreaseMasterVolume;
        }
        if (contains(bgmDecreaseRect(), point)) {
            return SettingsAction::DecreaseBgmVolume;
        }
        if (contains(bgmIncreaseRect(), point)) {
            return SettingsAction::IncreaseBgmVolume;
        }
        if (contains(seDecreaseRect(), point)) {
            return SettingsAction::DecreaseSeVolume;
        }
        if (contains(seIncreaseRect(), point)) {
            return SettingsAction::IncreaseSeVolume;
        }
    } else if (state_.activeTab == SettingsTab::Display) {
        if (contains(audioDecreaseRect(), point)) {
            return SettingsAction::DecreaseWindowScale;
        }
        if (contains(audioIncreaseRect(), point)) {
            return SettingsAction::IncreaseWindowScale;
        }
    }
    if (backButton(text_).contains(point)) {
        return SettingsAction::Back;
    }

    return std::nullopt;
}

} // namespace haru::game::scenes
