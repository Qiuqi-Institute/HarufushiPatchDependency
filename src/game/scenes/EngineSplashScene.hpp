#pragma once

#include <HaruGeometry>
#include <HaruRenderQueue>

namespace haru::game::scenes {

class EngineSplashScene {
public:
    explicit EngineSplashScene(double durationSeconds = 2.0);

    void update(double deltaSeconds);
    void render(engine::graphics::RenderQueue& queue, engine::graphics::Size surfaceSize) const;

    bool active() const;
    bool complete() const;
    double progress() const;

private:
    double durationSeconds_;
    double elapsedSeconds_ = 0.0;
};

} // namespace haru::game::scenes
