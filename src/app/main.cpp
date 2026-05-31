#include <HaruFrame>

#include "localization/GameText.hpp"
#include "scenes/HomeScene.hpp"
#include "scenes/StudioSplashScene.hpp"
#include "scenes/TitleScene.hpp"
#include "systems/HarufushiGame.hpp"

#include <cstring>
#include <chrono>
#include <exception>
#include <iostream>
#include <thread>

namespace {

enum class GameScreen {
    StudioSplash,
    Home,
    DailyLoop,
};

} // namespace

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
        haru::game::scenes::StudioSplashScene studioSplashScene(2.0);
        haru::game::localization::GameText gameText =
            haru::game::localization::GameText::loadDefault("en-US");
        haru::game::scenes::HomeScene homeScene(gameText);
        haru::game::scenes::TitleScene dailyScene(gameText);
        haru::game::scenes::HomePanel homePanel = haru::game::scenes::HomePanel::Main;
        GameScreen screen = GameScreen::StudioSplash;
        haru::game::systems::DailyLoopState dailyLoopState;
        const auto applyLocale = [&](const char* localeTag) {
            if (!gameText.setLocale(localeTag)) {
                return;
            }

            homeScene = haru::game::scenes::HomeScene(gameText);
            dailyScene = haru::game::scenes::TitleScene(gameText);
        };
        window.show();

        return app.run(game, [&](const haru::engine::core::FrameContext& frame) {
            const auto events = window.pollEvents();
            for (const auto& event : events) {
                if (event.kind == haru::engine::platform::WindowEventKind::CloseRequested) {
                    window.requestClose();
                } else if (event.kind ==
                               haru::engine::platform::WindowEventKind::MouseButtonReleased &&
                           event.button == haru::engine::platform::MouseButton::Left) {
                    if (screen == GameScreen::Home) {
                        const auto action =
                            homeScene.actionAt({event.x, event.y},
                                               {surface.width(), surface.height()},
                                               homePanel);
                        if (action == haru::game::scenes::HomeAction::NewGame) {
                            dailyLoopState = haru::game::systems::DailyLoopState{};
                            screen = GameScreen::DailyLoop;
                        } else if (action == haru::game::scenes::HomeAction::OpenSaves) {
                            homePanel = haru::game::scenes::HomePanel::Saves;
                        } else if (action == haru::game::scenes::HomeAction::OpenSettings) {
                            homePanel = haru::game::scenes::HomePanel::Settings;
                        } else if (action ==
                                   haru::game::scenes::HomeAction::SetLocaleEnglish) {
                            applyLocale("en-US");
                        } else if (action == haru::game::scenes::HomeAction::
                                                  SetLocaleSimplifiedChinese) {
                            applyLocale("zh-CN");
                        } else if (action ==
                                   haru::game::scenes::HomeAction::SetLocaleJapanese) {
                            applyLocale("ja-JP");
                        } else if (action == haru::game::scenes::HomeAction::Back) {
                            homePanel = haru::game::scenes::HomePanel::Main;
                        } else if (action == haru::game::scenes::HomeAction::Quit) {
                            window.requestClose();
                        }
                    } else if (screen == GameScreen::DailyLoop) {
                        const auto action =
                            dailyScene.actionAt({event.x, event.y},
                                                {surface.width(), surface.height()});
                        if (action.has_value()) {
                            dailyLoopState.apply(*action);
                        }
                    }
                }
            }

            haru::engine::graphics::RenderQueue queue;
            engineFrame.render(queue,
                               {surface.width(), surface.height()},
                               frame.deltaSeconds,
                               [&](haru::engine::graphics::RenderQueue& contentQueue) {
                                   if (screen == GameScreen::StudioSplash) {
                                       studioSplashScene.update(frame.deltaSeconds);
                                       if (studioSplashScene.active()) {
                                           studioSplashScene.render(
                                               contentQueue,
                                               {surface.width(), surface.height()});
                                           return;
                                       }
                                       screen = GameScreen::Home;
                                   }

                                   if (screen == GameScreen::Home) {
                                       homeScene.render(contentQueue,
                                                        {surface.width(), surface.height()},
                                                        homePanel);
                                   } else {
                                       dailyScene.render(contentQueue,
                                                         {surface.width(), surface.height()},
                                                         dailyLoopState.stats());
                                   }
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
