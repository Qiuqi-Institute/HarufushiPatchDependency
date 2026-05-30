#include "engine/core/Application.hpp"

#include <utility>

namespace haru::engine::core {

Application::Application(ApplicationConfig config) : config_(std::move(config)) {}

int Application::run(GameRuntime& runtime) {
    runtime.start(config_);
    return 0;
}

const ApplicationConfig& Application::config() const {
    return config_;
}

} // namespace haru::engine::core
