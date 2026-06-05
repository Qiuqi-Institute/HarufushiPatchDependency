#include "support/TestHarness.hpp"

#include "engine/core/FrameLoop.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

HARU_TEST(frame_loop_stops_when_runtime_requests_stop) {
    haru::engine::core::FrameLoop loop;
    int ticks = 0;

    const auto result = loop.run([&](const haru::engine::core::FrameContext& context) {
        ++ticks;
        HARU_EXPECT_EQ(context.frameIndex, static_cast<std::uint64_t>(ticks - 1));
        if (ticks == 3) {
            return haru::engine::core::LoopDecision::Stop;
        }
        return haru::engine::core::LoopDecision::Continue;
    });

    HARU_EXPECT_EQ(result.framesRun, static_cast<std::uint64_t>(3));
    HARU_EXPECT_EQ(ticks, 3);
}

HARU_TEST(frame_loop_can_be_limited_for_headless_tests) {
    haru::engine::core::FrameLoop loop;

    const auto result = loop.runForFrames(5, [](const haru::engine::core::FrameContext&) {
        return haru::engine::core::LoopDecision::Continue;
    });

    HARU_EXPECT_EQ(result.framesRun, static_cast<std::uint64_t>(5));
}

HARU_TEST(frame_loop_keeps_runtime_delta_fixed_for_opening_animation_cadence) {
    haru::engine::core::FrameLoop loop;
    std::vector<double> deltas;

    loop.run([&](const haru::engine::core::FrameContext& context) {
        deltas.push_back(context.deltaSeconds);
        return deltas.size() == 3U ? haru::engine::core::LoopDecision::Stop
                                   : haru::engine::core::LoopDecision::Continue;
    });

    HARU_EXPECT_EQ(deltas.size(), static_cast<std::size_t>(3));
    for (const double delta : deltas) {
        HARU_EXPECT_TRUE(std::abs(delta - (1.0 / 60.0)) < 0.0001);
    }
}
