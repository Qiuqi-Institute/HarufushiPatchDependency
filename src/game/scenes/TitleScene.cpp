#include "TitleScene.hpp"

#include <algorithm>

namespace haru::game::scenes {

void TitleScene::render(engine::graphics::RenderQueue& queue,
                        engine::graphics::Size surfaceSize) const {
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

    const engine::ui::ButtonStyle primaryButton{{206, 86, 132, 255},
                                                {255, 246, 240, 255},
                                                12};
    const engine::ui::ButtonStyle secondaryButton{{74, 88, 112, 255},
                                                  {240, 236, 230, 255},
                                                  12};
    engine::ui::Button studyButton({72, 232, 284, 48}, "Study", secondaryButton);
    engine::ui::Button moddingButton({72, 296, 284, 48}, "Modding", primaryButton);
    engine::ui::Button harufushiButton({72, 360, 284, 48}, "Harufushi", secondaryButton);

    studyButton.render(queue);
    moddingButton.render(queue);
    harufushiButton.render(queue);
}

} // namespace haru::game::scenes
