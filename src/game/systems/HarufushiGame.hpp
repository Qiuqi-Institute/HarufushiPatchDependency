#pragma once

#include <HaruApplication>

#include <string>

namespace haru::game {

class HarufushiGame final : public engine::core::GameRuntime {
public:
    void start(const engine::core::ApplicationConfig& config) override;

    int startCount() const;
    const std::string& lastBootLine() const;

private:
    int startCount_ = 0;
    std::string lastBootLine_;
};

} // namespace haru::game
