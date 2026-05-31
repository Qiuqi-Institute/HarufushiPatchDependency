#include "HomeScene.hpp"

#include <algorithm>
#include <utility>

namespace haru::game::scenes {

namespace {

engine::ui::ButtonStyle primaryButtonStyle() {
    return {{202, 82, 132, 255}, {255, 246, 240, 255}, 12};
}

engine::ui::ButtonStyle secondaryButtonStyle() {
    return {{76, 90, 116, 255}, {244, 240, 236, 255}, 12};
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

void renderShell(engine::graphics::RenderQueue& queue,
                 engine::graphics::Size surfaceSize,
                 const localization::GameText& text) {
    const int width = std::max(surfaceSize.width, 1);
    const int height = std::max(surfaceSize.height, 1);
    const int margin = 40;
    const int contentWidth = std::max(width - (margin * 2), 1);
    const int mainTop = 188;
    const int mainHeight = std::max(height - mainTop - 112, 1);

    engine::ui::UiNode root({0, 0, width, height}, {18, 18, 24, 255});
    root.addChild({{margin, margin, contentWidth, 112}, {54, 38, 62, 255}});
    root.addChild({{margin, mainTop, 392, mainHeight}, {30, 42, 54, 255}});
    root.addChild({{472, mainTop, std::max(width - 512, 1), mainHeight}, {42, 35, 48, 255}});
    root.render(queue);

    queue.drawText({96, 108, 460, 34},
                   text.get(localization::TextId::GameTitle),
                   {255, 246, 240, 255});
}

} // namespace

HomeScene::HomeScene(localization::GameText text) : text_(std::move(text)) {}

void HomeScene::render(engine::graphics::RenderQueue& queue,
                       engine::graphics::Size surfaceSize,
                       HomePanel panel) const {
    queue.clear({18, 18, 24, 255});
    renderShell(queue, surfaceSize, text_);

    newGameButton(text_).render(queue);
    loadButton(text_).render(queue);
    settingsButton(text_).render(queue);
    quitButton(text_).render(queue);

    const int width = std::max(surfaceSize.width, 1);
    const int panelWidth = std::max(width - 560, 1);

    if (panel == HomePanel::Saves) {
        queue.drawText({520, 250, panelWidth, 32},
                       text_.get(localization::TextId::SaveFiles),
                       {255, 246, 240, 255});
        queue.drawText({520, 300, panelWidth, 28},
                       text_.get(localization::TextId::NoSaveDataYet),
                       {226, 218, 232, 255});
        backButton(text_).render(queue);
        return;
    }

    if (panel == HomePanel::Settings) {
        queue.drawText({520, 250, panelWidth, 32},
                       text_.get(localization::TextId::Settings),
                       {255, 246, 240, 255});
        queue.drawText({520, 300, panelWidth, 28},
                       text_.get(localization::TextId::Language),
                       {255, 246, 240, 255});
        englishButton(text_).render(queue);
        chineseButton(text_).render(queue);
        japaneseButton(text_).render(queue);
        queue.drawText({520, 532, panelWidth, 28},
                       text_.get(localization::TextId::Audio80),
                       {226, 218, 232, 255});
        queue.drawText({520, 568, panelWidth, 28},
                       text_.get(localization::TextId::Visual100),
                       {226, 218, 232, 255});
        queue.drawText({520, 604, panelWidth, 28},
                       text_.get(localization::TextId::TextSpeedNormal),
                       {226, 218, 232, 255});
        backButton(text_).render(queue);
        return;
    }

    queue.drawText({520, 250, panelWidth, 32},
                   text_.get(localization::TextId::Home),
                   {255, 246, 240, 255});
    queue.drawText({520, 300, panelWidth, 28},
                   text_.get(localization::TextId::HomePrompt),
                   {226, 218, 232, 255});
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
