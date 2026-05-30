#include <HaruFrame>

#include "scenes/TitleScene.hpp"
#include "systems/HarufushiGame.hpp"

#include <cstring>
#include <chrono>
#include <exception>
#include <iostream>
#include <thread>

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
        haru::engine::HaruFrame engineFrame(2.0);
        haru::game::scenes::TitleScene titleScene;
        haru::game::systems::DailyLoopState dailyLoopState;
        window.show();

        return app.run(game, [&](const haru::engine::core::FrameContext& frame) {
            const auto events = window.pollEvents();
            for (const auto& event : events) {
                if (event.kind == haru::engine::platform::WindowEventKind::CloseRequested) {
                    window.requestClose();
                } else if (event.kind ==
                               haru::engine::platform::WindowEventKind::MouseButtonReleased &&
                           event.button == haru::engine::platform::MouseButton::Left) {
                    const auto action = titleScene.actionAt({event.x, event.y},
                                                            {surface.width(), surface.height()});
                    if (action.has_value()) {
                        dailyLoopState.apply(*action);
                    }
                }
            }

            haru::engine::graphics::RenderQueue queue;
            engineFrame.render(queue,
                               {surface.width(), surface.height()},
                               frame.deltaSeconds,
                               [&](haru::engine::graphics::RenderQueue& contentQueue) {
                                   titleScene.render(contentQueue,
                                                     {surface.width(), surface.height()},
                                                     dailyLoopState.stats());
                               });

            surface.draw(queue, haru::engine::graphics::TextRasterization::Skip);
            presenter.present(window, surface, queue);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));

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
