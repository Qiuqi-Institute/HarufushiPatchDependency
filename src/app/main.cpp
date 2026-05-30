#include "engine/core/Application.hpp"
#include "game/systems/HarufushiGame.hpp"

#ifdef _WIN32
#include "engine/graphics/RenderQueue.hpp"
#include "engine/graphics/SoftwareSurface.hpp"
#include "engine/platform/windows/Win32SoftwarePresenter.hpp"
#include "engine/platform/windows/Win32Window.hpp"
#include "engine/ui/UiNode.hpp"
#endif

#include <cstring>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    haru::engine::core::Application app({"春伏补丁依存症", "0.0.1"});
    haru::game::HarufushiGame game;

    if (argc > 1 && std::strcmp(argv[1], "--smoke") == 0) {
        const int exitCode = app.run(game);
        std::cout << game.lastBootLine() << '\n';
        return exitCode;
    }

#ifdef _WIN32
    try {
        haru::engine::platform::windows::Win32Window window(
            haru::engine::platform::WindowConfig::defaultGameWindow());
        haru::engine::platform::windows::Win32SoftwarePresenter presenter;
        haru::engine::graphics::SoftwareSurface surface(1280, 720);
        window.show();

        return app.run(game, [&](const haru::engine::core::FrameContext&) {
            const auto events = window.pollEvents();
            for (const auto& event : events) {
                if (event.kind == haru::engine::platform::WindowEventKind::CloseRequested) {
                    window.requestClose();
                }
            }

            haru::engine::graphics::RenderQueue queue;
            queue.clear({18, 18, 22, 255});

            haru::engine::ui::UiNode root({0, 0, 1280, 720}, {22, 22, 28, 255});
            root.addChild({{40, 40, 1200, 112}, {54, 38, 62, 255}});
            root.addChild({{40, 188, 360, 420}, {32, 42, 54, 255}});
            root.addChild({{432, 188, 808, 420}, {42, 35, 48, 255}});
            root.addChild({{72, 72, 336, 48}, {206, 86, 132, 255}});
            root.render(queue);

            surface.draw(queue);
            presenter.present(window, surface);

            return window.shouldClose() ? haru::engine::core::LoopDecision::Stop
                                        : haru::engine::core::LoopDecision::Continue;
        });
    } catch (const std::exception& error) {
        std::cerr << "Failed to start native window: " << error.what() << '\n';
        return 1;
    }
#else
    const int exitCode = app.run(game);
    std::cout << game.lastBootLine() << '\n';
    return exitCode;
#endif
}
