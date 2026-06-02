#pragma once

#include "HaruFrame.hpp"
#include "../graphics/Geometry.hpp"
#include "../graphics/RenderQueue.hpp"

namespace haru::engine::core {

class EngineOpeningGate {
public:
    explicit EngineOpeningGate(double openingSeconds = 2.0);

    bool compose(graphics::RenderQueue& presentedQueue,
                 graphics::Size surfaceSize,
                 double deltaSeconds,
                 const graphics::RenderQueue& callerQueue);
    bool openingActive() const;

private:
    HaruFrame frame_;
};

} // namespace haru::engine::core
