#include "engine/core/AsyncStartupTask.hpp"

#include <utility>

namespace haru::engine::core {

AsyncStartupTask::~AsyncStartupTask() {
    try {
        wait();
    } catch (...) {
    }
}

bool AsyncStartupTask::start(Work work) {
    if (started_ || !work) {
        return false;
    }

    started_ = true;
    completed_ = false;
    future_ = std::async(std::launch::async, [this, work = std::move(work)]() mutable {
        try {
            work();
        } catch (...) {
            completed_ = true;
            throw;
        }
        completed_ = true;
    });
    return true;
}

bool AsyncStartupTask::started() const {
    return started_;
}

bool AsyncStartupTask::ready() const {
    return started_ && completed_.load();
}

void AsyncStartupTask::wait() {
    if (!future_.valid()) {
        return;
    }

    future_.get();
    completed_ = true;
}

} // namespace haru::engine::core
