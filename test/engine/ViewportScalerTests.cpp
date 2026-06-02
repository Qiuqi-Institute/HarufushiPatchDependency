#include "support/TestHarness.hpp"

#include "engine/graphics/ViewportScaler.hpp"

HARU_TEST(viewport_scaler_letterboxes_design_resolution_into_target_size) {
    haru::engine::graphics::ViewportScaler scaler({1280, 720});

    const auto rect = scaler.presentationRect({1600, 1200});
    HARU_EXPECT_EQ(rect.x, 0);
    HARU_EXPECT_EQ(rect.y, 150);
    HARU_EXPECT_EQ(rect.width, 1600);
    HARU_EXPECT_EQ(rect.height, 900);

    const auto center = scaler.mapPointToDesign({800, 600}, {1600, 1200});
    HARU_EXPECT_TRUE(center.has_value());
    HARU_EXPECT_EQ(center->x, 640);
    HARU_EXPECT_EQ(center->y, 360);
    HARU_EXPECT_FALSE(scaler.mapPointToDesign({40, 40}, {1600, 1200}).has_value());
}

HARU_TEST(viewport_scaler_applies_resolution_scale_percent_inside_target_size) {
    haru::engine::graphics::ViewportScaler scaler({1280, 720});

    const auto rect = scaler.presentationRect({1280, 720}, 50);
    HARU_EXPECT_EQ(rect.x, 320);
    HARU_EXPECT_EQ(rect.y, 180);
    HARU_EXPECT_EQ(rect.width, 640);
    HARU_EXPECT_EQ(rect.height, 360);

    const auto center = scaler.mapPointToDesign({640, 360}, {1280, 720}, 50);
    HARU_EXPECT_TRUE(center.has_value());
    HARU_EXPECT_EQ(center->x, 640);
    HARU_EXPECT_EQ(center->y, 360);
    HARU_EXPECT_FALSE(scaler.mapPointToDesign({100, 100}, {1280, 720}, 50).has_value());
}
