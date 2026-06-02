#include "support/TestHarness.hpp"

#include <HaruResult>
#include <HaruString>

HARU_TEST(haru_string_wraps_utf8_and_counts_code_points) {
    haru::engine::foundation::HaruString value("春伏 Patch");

    HARU_EXPECT_EQ(value.toUtf8(), "春伏 Patch");
    HARU_EXPECT_EQ(value.codePointCount(), static_cast<std::size_t>(8));
    HARU_EXPECT_TRUE(value.startsWith(haru::engine::foundation::HaruString("春伏")));
    HARU_EXPECT_FALSE(value.empty());
}

HARU_TEST(haru_result_carries_value_or_error_without_exceptions) {
    auto ok = haru::engine::foundation::HaruResult<int>::ok(42);
    auto failed =
        haru::engine::foundation::HaruResult<int>::error("registry write failed");

    HARU_EXPECT_TRUE(ok.hasValue());
    HARU_EXPECT_EQ(ok.value(), 42);
    HARU_EXPECT_FALSE(failed.hasValue());
    HARU_EXPECT_EQ(failed.error(), "registry write failed");
}
