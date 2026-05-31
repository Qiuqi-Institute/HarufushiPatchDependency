#pragma once

#include <HaruFrame>

namespace haru::game::scenes {

class StudioSplashScene {
public:
    explicit StudioSplashScene(double durationSeconds = 2.0);

    void update(double deltaSeconds);
    bool active() const;
    double progress() const;
    void render(engine::graphics::RenderQueue& queue,
                engine::graphics::Size surfaceSize) const;

private:
    double durationSeconds_;
    double elapsedSeconds_ = 0.0;
};

} // namespace haru::game::scenes
