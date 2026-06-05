#pragma once

#include <atomic>
#include <functional>
#include <future>

namespace haru::engine::core {

class AsyncStartupTask {
public:
    using Work = std::function<void()>;

    AsyncStartupTask() = default;
    ~AsyncStartupTask();

    AsyncStartupTask(const AsyncStartupTask&) = delete;
    AsyncStartupTask& operator=(const AsyncStartupTask&) = delete;

    bool start(Work work);
    bool started() const;
    bool ready() const;
    void wait();

private:
    std::future<void> future_;
    bool started_ = false;
    std::atomic_bool completed_{false};
};

} // namespace haru::engine::core
