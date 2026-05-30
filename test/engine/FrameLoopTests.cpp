#include "support/TestHarness.hpp"

#include "engine/core/FrameLoop.hpp"

#include <cstdint>

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
