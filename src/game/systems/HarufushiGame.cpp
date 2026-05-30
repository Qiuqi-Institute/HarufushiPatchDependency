#include "game/systems/HarufushiGame.hpp"

namespace haru::game {

void HarufushiGame::start(const engine::core::ApplicationConfig&) {
    ++startCount_;
    lastBootLine_ = "Harufushi Patch Dependency bootstrap";
}

int HarufushiGame::startCount() const {
    return startCount_;
}

const std::string& HarufushiGame::lastBootLine() const {
    return lastBootLine_;
}

} // namespace haru::game
