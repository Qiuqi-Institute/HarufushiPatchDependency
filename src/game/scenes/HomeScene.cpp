#include "HomeScene.hpp"

#include <algorithm>

namespace haru::game::scenes {

namespace {

engine::ui::ButtonStyle primaryButtonStyle() {
    return {{202, 82, 132, 255}, {255, 246, 240, 255}, 12};
}

engine::ui::ButtonStyle secondaryButtonStyle() {
    return {{76, 90, 116, 255}, {244, 240, 236, 255}, 12};
}

engine::ui::Button newGameButton() {
    return {{96, 252, 292, 48}, "New Game", primaryButtonStyle()};
}

engine::ui::Button loadButton() {
    return {{96, 316, 292, 48}, "Load", secondaryButtonStyle()};
}

engine::ui::Button settingsButton() {
    return {{96, 380, 292, 48}, "Settings", secondaryButtonStyle()};
}

engine::ui::Button quitButton() {
    return {{96, 444, 292, 48}, "Quit", secondaryButtonStyle()};
}

engine::ui::Button backButton() {
    return {{96, 508, 292, 44}, "Back", secondaryButtonStyle()};
}

void renderShell(engine::graphics::RenderQueue& queue,
                 engine::graphics::Size surfaceSize) {
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
    root.setText("Harufushi Patch Dependency", {245, 235, 228, 255});
    root.render(queue);

    queue.drawText({96, 108, 460, 34}, "Harufushi Patch Dependency", {255, 246, 240, 255});
}

} // namespace

void HomeScene::render(engine::graphics::RenderQueue& queue,
                       engine::graphics::Size surfaceSize,
                       HomePanel panel) const {
    queue.clear({18, 18, 24, 255});
    renderShell(queue, surfaceSize);

    newGameButton().render(queue);
    loadButton().render(queue);
    settingsButton().render(queue);
    quitButton().render(queue);

    const int width = std::max(surfaceSize.width, 1);
    const int panelWidth = std::max(width - 560, 1);

    if (panel == HomePanel::Saves) {
        queue.drawText({520, 250, panelWidth, 32}, "Save Files", {255, 246, 240, 255});
        queue.drawText({520, 300, panelWidth, 28}, "No save data yet", {226, 218, 232, 255});
        backButton().render(queue);
        return;
    }

    if (panel == HomePanel::Settings) {
        queue.drawText({520, 250, panelWidth, 32}, "Settings", {255, 246, 240, 255});
        queue.drawText({520, 300, panelWidth, 28}, "Audio 80", {226, 218, 232, 255});
        queue.drawText({520, 336, panelWidth, 28}, "Visual 100", {226, 218, 232, 255});
        queue.drawText({520, 372, panelWidth, 28}, "Text Speed Normal", {226, 218, 232, 255});
        backButton().render(queue);
        return;
    }

    queue.drawText({520, 250, panelWidth, 32}, "Home", {255, 246, 240, 255});
    queue.drawText({520, 300, panelWidth, 28},
                   "Start Akioki and Harufushi's modding days.",
                   {226, 218, 232, 255});
}

std::optional<HomeAction> HomeScene::actionAt(engine::graphics::Point point,
                                              engine::graphics::Size surfaceSize,
                                              HomePanel panel) const {
    if (surfaceSize.width <= 0 || surfaceSize.height <= 0) {
        return std::nullopt;
    }

    if (panel != HomePanel::Main && backButton().contains(point)) {
        return HomeAction::Back;
    }

    if (panel == HomePanel::Main) {
        if (newGameButton().contains(point)) {
            return HomeAction::NewGame;
        }
        if (loadButton().contains(point)) {
            return HomeAction::OpenSaves;
        }
        if (settingsButton().contains(point)) {
            return HomeAction::OpenSettings;
        }
        if (quitButton().contains(point)) {
            return HomeAction::Quit;
        }
    }

    return std::nullopt;
}

} // namespace haru::game::scenes
