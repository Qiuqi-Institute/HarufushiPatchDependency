#include "HomeScene.hpp"

#include <algorithm>
#include <utility>

namespace haru::game::scenes {

namespace {

constexpr engine::graphics::Color bgCream{251, 248, 241, 255};
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
constexpr engine::graphics::Color softLilac{226, 221, 249, 255};
constexpr engine::graphics::Color warmWhite{255, 250, 246, 255};

engine::ui::ButtonStyle primaryButtonStyle() {
    return {sakuraStrong, deepInk, 12};
}

engine::ui::ButtonStyle secondaryButtonStyle() {
    return {sky, deepInk, 12};
}

engine::ui::Button newGameButton(const localization::GameText& text) {
    return {{96, 252, 292, 48}, text.get(localization::TextId::NewGame), primaryButtonStyle()};
}

engine::ui::Button loadButton(const localization::GameText& text) {
    return {{96, 316, 292, 48}, text.get(localization::TextId::Load), secondaryButtonStyle()};
}

engine::ui::Button settingsButton(const localization::GameText& text) {
    return {{96, 380, 292, 48}, text.get(localization::TextId::Settings), secondaryButtonStyle()};
}

engine::ui::Button quitButton(const localization::GameText& text) {
    return {{96, 444, 292, 48}, text.get(localization::TextId::Quit), secondaryButtonStyle()};
}

engine::ui::Button backButton(const localization::GameText& text) {
    return {{96, 508, 292, 44}, text.get(localization::TextId::Back), secondaryButtonStyle()};
}

engine::ui::Button englishButton(const localization::GameText& text) {
    return {{520, 344, 236, 44},
            text.get(localization::TextId::LanguageEnglish),
            secondaryButtonStyle()};
}

engine::ui::Button chineseButton(const localization::GameText& text) {
    return {{520, 404, 236, 44},
            text.get(localization::TextId::LanguageSimplifiedChinese),
            secondaryButtonStyle()};
}

engine::ui::Button japaneseButton(const localization::GameText& text) {
    return {{520, 464, 236, 44},
            text.get(localization::TextId::LanguageJapanese),
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

void drawPetals(engine::graphics::RenderQueue& queue,
                engine::graphics::Size surfaceSize) {
    const int width = std::max(surfaceSize.width, 1);
    const int height = std::max(surfaceSize.height, 1);
    queue.fillRoundedRect({84, 116, 24, 10}, sakuraStrong, 5);
    queue.fillRoundedRect({148, 86, 18, 8}, sakuraStrong, 4);
    queue.fillRoundedRect({width - 186, 112, 26, 10}, sakuraStrong, 5);
    queue.fillRoundedRect({width - 126, 84, 16, 8}, sakuraStrong, 4);
    queue.fillRoundedRect({width - 248, height - 126, 24, 10}, sakuraStrong, 5);
    queue.fillRoundedRect({118, height - 104, 16, 8}, sakuraStrong, 4);
}

void renderButton(engine::graphics::RenderQueue& queue, const engine::ui::Button& button) {
    const auto& bounds = button.bounds();
    queue.fillRoundedRect({bounds.x + 8, bounds.y + 8, bounds.width, bounds.height},
                          shadow,
                          18);
    queue.fillRoundedRect(bounds, button.bounds().y == 252 ? sakuraStrong : sky, 18);
    queue.strokeRect(bounds, warmWhite, 2);
    queue.fillRoundedRect({bounds.x + 16, bounds.y + 8, 48, 8}, warmWhite, 4);
    queue.fillEllipse({bounds.x + bounds.width - 34, bounds.y + 14, 14, 14}, warmWhite);
    queue.drawText({bounds.x + 28, bounds.y + 11, bounds.width - 56, bounds.height - 18},
                   button.label(),
                   deepInk);
}

void renderShell(engine::graphics::RenderQueue& queue,
                 engine::graphics::Size surfaceSize,
                 const localization::GameText& text) {
    const int width = std::max(surfaceSize.width, 1);
    const int height = std::max(surfaceSize.height, 1);
    queue.fillVerticalGradient({0, 0, width, height}, {255, 247, 250, 255}, {232, 249, 250, 255});
    queue.fillRoundedRect({-40, 28, width + 80, 96}, sky, 42);
    queue.fillRoundedRect({96, 72, 640, 82}, warmWhite, 26);
    queue.strokeRect({96, 72, 640, 82}, sakuraStrong, 3);
    queue.fillRoundedRect({118, 126, 122, 8}, gold, 4);
    queue.fillRoundedRect({256, 126, 56, 8}, sakuraStrong, 4);
    drawPetals(queue, surfaceSize);

    drawPaper(queue, {56, 158, 384, 454}, paperBlue, sakura);
    drawPaper(queue, {472, 158, std::max(width - 528, 1), 454}, paper, sky);
    queue.fillRoundedRect({940, 206, 180, 180}, {245, 239, 255, 255}, 46);
    queue.strokeRect({940, 206, 180, 180}, softLilac, 2);
    queue.fillEllipse({988, 238, 84, 84}, sakura);
    queue.fillEllipse({1012, 264, 38, 38}, warmWhite);
    queue.fillRoundedRect({978, 338, 104, 14}, sky, 7);

    queue.drawText({116, 88, 600, 52},
                   text.get(localization::TextId::GameTitle),
                   ink);
}

} // namespace

HomeScene::HomeScene(localization::GameText text) : text_(std::move(text)) {}

void HomeScene::render(engine::graphics::RenderQueue& queue,
                       engine::graphics::Size surfaceSize,
    HomePanel panel) const {
    queue.clear(bgCream);
    renderShell(queue, surfaceSize, text_);

    renderButton(queue, newGameButton(text_));
    renderButton(queue, loadButton(text_));
    renderButton(queue, settingsButton(text_));
    renderButton(queue, quitButton(text_));

    const int width = std::max(surfaceSize.width, 1);
    const int panelWidth = std::max(width - 560, 1);

    if (panel == HomePanel::Saves) {
        queue.drawText({520, 250, panelWidth, 32},
                       text_.get(localization::TextId::SaveFiles),
                       ink);
        queue.drawText({520, 300, panelWidth, 28},
                       text_.get(localization::TextId::NoSaveDataYet),
                       deepInk);
        renderButton(queue, backButton(text_));
        return;
    }

    if (panel == HomePanel::Settings) {
        queue.drawText({520, 250, panelWidth, 32},
                       text_.get(localization::TextId::Settings),
                       ink);
        queue.drawText({520, 300, panelWidth, 28},
                       text_.get(localization::TextId::Language),
                       deepInk);
        renderButton(queue, englishButton(text_));
        renderButton(queue, chineseButton(text_));
        renderButton(queue, japaneseButton(text_));
        queue.drawText({520, 456, panelWidth, 28},
                       text_.get(localization::TextId::Audio80),
                       deepInk);
        queue.drawText({520, 494, panelWidth, 28},
                       text_.get(localization::TextId::Visual100),
                       deepInk);
        queue.drawText({520, 532, panelWidth, 28},
                       text_.get(localization::TextId::TextSpeedNormal),
                       deepInk);
        renderButton(queue, backButton(text_));
        return;
    }

    queue.fillRoundedRect({520, 340, std::max(panelWidth - 40, 1), 6}, sakuraStrong, 3);
    queue.fillRoundedRect({520, 360, std::max(panelWidth - 112, 1), 6}, mint, 3);
    queue.fillRoundedRect({520, 380, std::max(panelWidth - 196, 1), 6}, gold, 3);
    queue.drawText({520, 250, panelWidth, 32},
                   text_.get(localization::TextId::Home),
                   ink);
    queue.drawText({520, 300, panelWidth, 28},
                   text_.get(localization::TextId::HomePrompt),
                   deepInk);
    queue.fillRoundedRect({520, 414, 560, 134}, {245, 239, 255, 255}, 24);
    queue.strokeRect({520, 414, 560, 134}, softLilac, 2);
    queue.fillRoundedRect({548, 452, 132, 10}, sakuraStrong, 5);
    queue.fillRoundedRect({548, 486, 202, 10}, sky, 5);
    queue.drawText({548, 424, 260, 30},
                   text_.get(localization::TextId::HomeBoardTitle),
                   ink);
    queue.drawText({700, 444, 332, 28},
                   text_.get(localization::TextId::HomeBoardPatch),
                   deepInk);
    queue.drawText({770, 478, 262, 28},
                   text_.get(localization::TextId::HomeBoardCompile),
                   deepInk);
    queue.drawText({548, 514, 488, 28},
                   text_.get(localization::TextId::HomeHarufushiStatus),
                   deepInk);
}

std::optional<HomeAction> HomeScene::actionAt(engine::graphics::Point point,
                                              engine::graphics::Size surfaceSize,
                                              HomePanel panel) const {
    if (surfaceSize.width <= 0 || surfaceSize.height <= 0) {
        return std::nullopt;
    }

    if (panel != HomePanel::Main && backButton(text_).contains(point)) {
        return HomeAction::Back;
    }

    if (panel == HomePanel::Settings) {
        if (englishButton(text_).contains(point)) {
            return HomeAction::SetLocaleEnglish;
        }
        if (chineseButton(text_).contains(point)) {
            return HomeAction::SetLocaleSimplifiedChinese;
        }
        if (japaneseButton(text_).contains(point)) {
            return HomeAction::SetLocaleJapanese;
        }
    }

    if (panel == HomePanel::Main) {
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
    }

    return std::nullopt;
}

} // namespace haru::game::scenes
