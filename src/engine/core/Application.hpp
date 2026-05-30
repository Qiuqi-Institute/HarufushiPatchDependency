#pragma once

#include "FrameLoop.hpp"

#include <string>

namespace haru::engine::core {

struct ApplicationConfig {
    std::string title;
    std::string version;
};

class GameRuntime {
public:
    virtual ~GameRuntime() = default;

    virtual void start(const ApplicationConfig& config) = 0;
};

class Application {
public:
    explicit Application(ApplicationConfig config);

    int run(GameRuntime& runtime);
    int run(GameRuntime& runtime, const FrameCallback& onFrame);
    const ApplicationConfig& config() const;

private:
    ApplicationConfig config_;
};

} // namespace haru::engine::core
