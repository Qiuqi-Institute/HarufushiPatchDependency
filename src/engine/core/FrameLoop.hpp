#pragma once

#include <cstdint>
#include <functional>

namespace haru::engine::core {

enum class LoopDecision {
    Continue,
    Stop,
};

struct FrameContext {
    std::uint64_t frameIndex;
    double deltaSeconds;
};

struct FrameLoopResult {
    std::uint64_t framesRun;
};

using FrameCallback = std::function<LoopDecision(const FrameContext&)>;

class FrameLoop {
public:
    explicit FrameLoop(double fixedDeltaSeconds = 1.0 / 60.0);

    FrameLoopResult run(const FrameCallback& onFrame) const;
    FrameLoopResult runForFrames(std::uint64_t frameLimit, const FrameCallback& onFrame) const;

private:
    double fixedDeltaSeconds_;
};

} // namespace haru::engine::core
