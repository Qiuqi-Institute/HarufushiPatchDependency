#pragma once

#include <HaruFrame>

namespace haru::game::scenes {

class TitleScene {
public:
    void render(engine::graphics::RenderQueue& queue,
                engine::graphics::Size surfaceSize) const;
};

} // namespace haru::game::scenes
