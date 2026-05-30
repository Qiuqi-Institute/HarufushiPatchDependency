#include "engine/core/Application.hpp"
#include "game/systems/HarufushiGame.hpp"

#include <iostream>

int main() {
    haru::engine::core::Application app({"春伏补丁依存症", "0.0.1"});
    haru::game::HarufushiGame game;

    const int exitCode = app.run(game);
    std::cout << game.lastBootLine() << '\n';
    return exitCode;
}
