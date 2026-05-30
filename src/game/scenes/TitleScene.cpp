#include "TitleScene.hpp"

#include <algorithm>
#include <sstream>

namespace haru::game::scenes {

namespace {

engine::ui::ButtonStyle primaryButtonStyle() {
    return {{206, 86, 132, 255}, {255, 246, 240, 255}, 12};
}

engine::ui::ButtonStyle secondaryButtonStyle() {
    return {{74, 88, 112, 255}, {240, 236, 230, 255}, 12};
}

engine::ui::Button studyButton() {
    return {{72, 232, 284, 48}, "Study", secondaryButtonStyle()};
}

engine::ui::Button moddingButton() {
    return {{72, 296, 284, 48}, "Modding", primaryButtonStyle()};
}

engine::ui::Button harufushiButton() {
    return {{72, 360, 284, 48}, "Harufushi", secondaryButtonStyle()};
}

std::string statsLine(const systems::DailyStats& stats) {
    std::ostringstream line;
    line << "Day " << stats.day << "  Energy " << stats.energy << "  Study "
         << stats.studyFocus << "  Mod " << stats.modProgress << "  Bond "
         << stats.harufushiBond << "  Dependence " << stats.dependence;
    return line.str();
}

} // namespace

void TitleScene::render(engine::graphics::RenderQueue& queue,
                        engine::graphics::Size surfaceSize,
                        const systems::DailyStats& stats) const {
    queue.clear({18, 18, 22, 255});

    const int width = std::max(surfaceSize.width, 1);
    const int height = std::max(surfaceSize.height, 1);
    const int margin = 40;
    const int contentWidth = std::max(width - (margin * 2), 1);
    const int mainTop = 188;
    const int mainHeight = std::max(height - mainTop - 112, 1);

    engine::ui::UiNode root({0, 0, width, height}, {22, 22, 28, 255});
    root.addChild({{margin, margin, contentWidth, 112}, {54, 38, 62, 255}});
    root.addChild({{margin, mainTop, 360, mainHeight}, {32, 42, 54, 255}});
    root.addChild({{432, mainTop, std::max(width - 472, 1), mainHeight}, {42, 35, 48, 255}});
    root.setText("Harufushi Patch Dependency", {245, 235, 228, 255});
    root.render(queue);

    studyButton().render(queue);
    moddingButton().render(queue);
    harufushiButton().render(queue);

    queue.drawText({464, mainTop + 40, std::max(width - 528, 1), 32},
                   statsLine(stats),
                   {245, 235, 228, 255});
}

std::optional<systems::DailyAction> TitleScene::actionAt(
    engine::graphics::Point point,
    engine::graphics::Size surfaceSize) const {
    if (surfaceSize.width <= 0 || surfaceSize.height <= 0) {
        return std::nullopt;
    }

    if (studyButton().contains(point)) {
        return systems::DailyAction::Study;
    }
    if (moddingButton().contains(point)) {
        return systems::DailyAction::Modding;
    }
    if (harufushiButton().contains(point)) {
        return systems::DailyAction::SpendTimeWithHarufushi;
    }

    return std::nullopt;
}

} // namespace haru::game::scenes
