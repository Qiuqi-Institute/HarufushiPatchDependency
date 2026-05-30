#include <HaruApplication>

#include "scenes/EngineSplashScene.hpp"
#include "systems/HarufushiGame.hpp"

#ifdef _WIN32
#include <HaruButton>
#include <HaruRenderQueue>
#include <HaruSoftwareSurface>
#include <HaruUiNode>
#include <HaruWin32SoftwarePresenter>
#include <HaruWin32Window>
#endif

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
        haru::game::scenes::EngineSplashScene splash(2.0);
        window.show();

        return app.run(game, [&](const haru::engine::core::FrameContext& frame) {
            const auto events = window.pollEvents();
            for (const auto& event : events) {
                if (event.kind == haru::engine::platform::WindowEventKind::CloseRequested) {
                    window.requestClose();
                }
            }

            haru::engine::graphics::RenderQueue queue;
            if (splash.active()) {
                splash.update(frame.deltaSeconds);
                splash.render(queue, {surface.width(), surface.height()});
            } else {
                queue.clear({18, 18, 22, 255});

                haru::engine::ui::UiNode root({0, 0, 1280, 720}, {22, 22, 28, 255});
                root.addChild({{40, 40, 1200, 112}, {54, 38, 62, 255}});
                root.addChild({{40, 188, 360, 420}, {32, 42, 54, 255}});
                root.addChild({{432, 188, 808, 420}, {42, 35, 48, 255}});
                root.setText("Harufushi Patch Dependency", {245, 235, 228, 255});
                root.render(queue);

                const haru::engine::ui::ButtonStyle primaryButton{{206, 86, 132, 255},
                                                                  {255, 246, 240, 255},
                                                                  12};
                const haru::engine::ui::ButtonStyle secondaryButton{{74, 88, 112, 255},
                                                                    {240, 236, 230, 255},
                                                                    12};
                haru::engine::ui::Button studyButton({72, 232, 284, 48}, "Study",
                                                     secondaryButton);
                haru::engine::ui::Button moddingButton({72, 296, 284, 48}, "Modding",
                                                       primaryButton);
                haru::engine::ui::Button harufushiButton({72, 360, 284, 48}, "Harufushi",
                                                         secondaryButton);

                studyButton.render(queue);
                moddingButton.render(queue);
                harufushiButton.render(queue);
            }

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
