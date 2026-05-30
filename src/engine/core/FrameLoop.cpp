#include "engine/core/FrameLoop.hpp"

namespace haru::engine::core {

FrameLoop::FrameLoop(double fixedDeltaSeconds) : fixedDeltaSeconds_(fixedDeltaSeconds) {}

FrameLoopResult FrameLoop::run(const FrameCallback& onFrame) const {
    std::uint64_t frameIndex = 0;

    while (true) {
        const FrameContext context{frameIndex, fixedDeltaSeconds_};
        ++frameIndex;

        if (onFrame(context) == LoopDecision::Stop) {
            return {frameIndex};
        }
    }
}

FrameLoopResult FrameLoop::runForFrames(std::uint64_t frameLimit,
                                        const FrameCallback& onFrame) const {
    std::uint64_t framesRun = 0;

    for (; framesRun < frameLimit; ++framesRun) {
        const FrameContext context{framesRun, fixedDeltaSeconds_};
        if (onFrame(context) == LoopDecision::Stop) {
            ++framesRun;
            break;
        }
    }

    return {framesRun};
}

} // namespace haru::engine::core
