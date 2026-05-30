#pragma once

#include "../graphics/Geometry.hpp"
#include "../graphics/RenderQueue.hpp"

#include <functional>

namespace haru::engine {

class HaruFrame {
public:
    using VisualContentRenderer = std::function<void(graphics::RenderQueue&)>;

    explicit HaruFrame(double openingSeconds = 2.0);

    bool render(graphics::RenderQueue& queue,
                graphics::Size surfaceSize,
                double deltaSeconds,
                const VisualContentRenderer& renderVisualContent);

    bool openingActive() const;
    double openingProgress() const;

private:
    void renderOpening(graphics::RenderQueue& queue, graphics::Size surfaceSize) const;
    void updateOpening(double deltaSeconds);

    double openingSeconds_;
    double openingElapsedSeconds_ = 0.0;
};

} // namespace haru::engine
