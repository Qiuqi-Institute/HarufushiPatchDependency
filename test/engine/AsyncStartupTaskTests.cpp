#include "support/TestHarness.hpp"

#include "engine/core/AsyncStartupTask.hpp"

#include <atomic>
#include <chrono>
#include <future>

HARU_TEST(async_startup_task_runs_work_without_blocking_caller) {
    haru::engine::core::AsyncStartupTask task;
    std::promise<void> entered;
    std::promise<void> release;
    auto enteredFuture = entered.get_future();
    auto releaseFuture = release.get_future();
    std::atomic_bool finished{false};

    const bool started = task.start([&]() {
        entered.set_value();
        releaseFuture.wait();
        finished = true;
    });

    HARU_EXPECT_TRUE(started);
    HARU_EXPECT_TRUE(task.started());
    HARU_EXPECT_TRUE(enteredFuture.wait_for(std::chrono::seconds(1)) ==
                     std::future_status::ready);
    HARU_EXPECT_FALSE(task.ready());
    HARU_EXPECT_FALSE(finished.load());

    release.set_value();
    task.wait();

    HARU_EXPECT_TRUE(task.ready());
    HARU_EXPECT_TRUE(finished.load());
}

HARU_TEST(async_startup_task_rejects_duplicate_work_after_start) {
    haru::engine::core::AsyncStartupTask task;
    std::promise<void> release;
    auto releaseFuture = release.get_future();
    std::atomic_int runCount{0};

    HARU_EXPECT_TRUE(task.start([&]() {
        ++runCount;
        releaseFuture.wait();
    }));
    HARU_EXPECT_FALSE(task.start([&]() {
        ++runCount;
    }));

    release.set_value();
    task.wait();

    HARU_EXPECT_EQ(runCount.load(), 1);
}
