#include "support/TestHarness.hpp"

#include "engine/core/EngineOpeningGate.hpp"

#include <cstddef>

namespace {

bool hasText(const haru::engine::graphics::RenderQueue& queue, const std::string& text) {
    for (const auto& command : queue.commands()) {
        if (command.kind == haru::engine::graphics::DrawCommandKind::Text &&
            command.text == text) {
            return true;
        }
    }
    return false;
}

} // namespace

HARU_TEST(engine_opening_gate_forces_splash_over_caller_visual_queue) {
    haru::engine::core::EngineOpeningGate gate(1.0);
    haru::engine::graphics::RenderQueue caller;
    caller.clear({1, 2, 3, 255});
    caller.drawText({0, 0, 160, 32}, "Caller Content", {4, 5, 6, 255});

    haru::engine::graphics::RenderQueue presented;
    const bool opening = gate.compose(presented, {1280, 720}, 0.10, caller);

    HARU_EXPECT_TRUE(opening);
    HARU_EXPECT_TRUE(gate.openingActive());
    HARU_EXPECT_TRUE(hasText(presented, "H"));
    HARU_EXPECT_FALSE(hasText(presented, "Caller Content"));
    HARU_EXPECT_EQ(presented.commands()[0].color,
                   (haru::engine::graphics::Color{255, 255, 255, 255}));
}

HARU_TEST(engine_opening_gate_releases_caller_visual_queue_after_splash) {
    haru::engine::core::EngineOpeningGate gate(0.25);
    haru::engine::graphics::RenderQueue caller;
    caller.clear({1, 2, 3, 255});
    caller.drawText({0, 0, 160, 32}, "Caller Content", {4, 5, 6, 255});

    haru::engine::graphics::RenderQueue first;
    haru::engine::graphics::RenderQueue second;
    gate.compose(first, {1280, 720}, 0.25, caller);
    const bool opening = gate.compose(second, {1280, 720}, 0.016, caller);

    HARU_EXPECT_FALSE(opening);
    HARU_EXPECT_FALSE(gate.openingActive());
    HARU_EXPECT_TRUE(hasText(second, "Caller Content"));
    HARU_EXPECT_EQ(second.commands().size(), caller.commands().size());
}
