#include "engine/core/EngineOpeningGate.hpp"

namespace haru::engine::core {

EngineOpeningGate::EngineOpeningGate(double openingSeconds) : frame_(openingSeconds) {}

bool EngineOpeningGate::compose(graphics::RenderQueue& presentedQueue,
                                graphics::Size surfaceSize,
                                double deltaSeconds,
                                const graphics::RenderQueue& callerQueue) {
    presentedQueue.reset();
    const bool opening =
        frame_.render(presentedQueue, surfaceSize, deltaSeconds, [](graphics::RenderQueue&) {});
    if (!opening) {
        presentedQueue = callerQueue;
    }
    return opening;
}

bool EngineOpeningGate::openingActive() const {
    return frame_.openingActive();
}

} // namespace haru::engine::core
