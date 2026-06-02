#include <HaruFrame>

#include "localization/GameText.hpp"
#include "scenes/HomeScene.hpp"
#include "scenes/SettingsScene.hpp"
#include "scenes/StudioSplashScene.hpp"
#include "scenes/TitleScene.hpp"
#include "systems/HarufushiGame.hpp"
#include "systems/SaveManager.hpp"

#include <chrono>
#include <algorithm>
#include <cstring>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <thread>
#include <vector>

namespace {

enum class GameScreen {
    StudioSplash,
    Home,
    Settings,
    DailyLoop,
};

std::optional<std::size_t> loadSaveIndex(haru::game::scenes::HomeAction action) {
    switch (action) {
    case haru::game::scenes::HomeAction::LoadSave0:
        return 0;
    case haru::game::scenes::HomeAction::LoadSave1:
        return 1;
    case haru::game::scenes::HomeAction::LoadSave2:
        return 2;
    case haru::game::scenes::HomeAction::LoadSave3:
        return 3;
    default:
        return std::nullopt;
    }
}

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
        haru::engine::platform::windows::Win32EncryptedRegistrySettings settingsStore;
        haru::engine::graphics::SoftwareSurface surface(1280, 720);
        haru::game::scenes::StudioSplashScene studioSplashScene(2.0);
        const std::filesystem::path saveRoot =
            haru::engine::platform::UserDirectories::documentsDirectory() /
            "Harufushi Patch Dependency" / "save data";
        haru::game::systems::SaveManager saveManager(saveRoot);
        saveManager.loadAll();
        const std::string fallbackLocale =
            saveManager.activeSave() != nullptr ? saveManager.activeSave()->localeTag : "en-US";
        const std::string initialLocale = settingsStore.string("locale", fallbackLocale);
        haru::game::scenes::SettingsState settingsState{
            haru::game::scenes::SettingsTab::Game,
            std::clamp(settingsStore.integer("master_volume", 80), 0, 100),
            std::clamp(settingsStore.integer("bgm_volume", 70), 0, 100),
            std::clamp(settingsStore.integer("se_volume", 80), 0, 100),
            std::clamp(settingsStore.integer("window_scale", 100), 50, 200),
            std::clamp(settingsStore.integer("text_speed", 50), 0, 100)};
        presenter.setResolutionScalePercent(settingsState.windowScale);
        const haru::engine::graphics::ViewportScaler viewportScaler({surface.width(),
                                                                     surface.height()});
        haru::game::localization::GameText gameText =
            haru::game::localization::GameText::loadDefault(initialLocale);
        haru::game::scenes::HomeScene homeScene(gameText);
        haru::game::scenes::SettingsScene settingsScene(gameText, settingsState);
        haru::game::scenes::TitleScene dailyScene(gameText);
        haru::game::scenes::HomePanel homePanel = haru::game::scenes::HomePanel::Main;
        GameScreen screen = GameScreen::StudioSplash;
        haru::game::systems::DailyLoopState dailyLoopState;
        if (saveManager.activeSave() != nullptr) {
            dailyLoopState = haru::game::systems::DailyLoopState(saveManager.activeSave()->stats);
        }
        const auto applyLocale = [&](const char* localeTag, bool recordToSave = true) {
            if (!gameText.setLocale(localeTag)) {
                return;
            }

            homeScene = haru::game::scenes::HomeScene(gameText);
            settingsScene = haru::game::scenes::SettingsScene(gameText, settingsState);
            dailyScene = haru::game::scenes::TitleScene(gameText);
            window.setTitle(gameText.get(haru::game::localization::TextId::GameTitle));
            settingsStore.setString("locale", gameText.activeLocale());
            if (recordToSave) {
                saveManager.recordLocaleChange(gameText.activeLocale());
            }
        };
        const auto refreshSettingsScene = [&]() {
            settingsScene = haru::game::scenes::SettingsScene(gameText, settingsState);
        };
        const auto persistSettings = [&]() {
            settingsStore.setInt("master_volume", settingsState.masterVolume);
            settingsStore.setInt("bgm_volume", settingsState.bgmVolume);
            settingsStore.setInt("se_volume", settingsState.seVolume);
            settingsStore.setInt("window_scale", settingsState.windowScale);
            settingsStore.setInt("text_speed", settingsState.textSpeed);
            presenter.setResolutionScalePercent(settingsState.windowScale);
        };
        const auto saveSummaries = [&]() {
            std::vector<std::string> summaries;
            int slot = 1;
            for (const auto& save : saveManager.saves()) {
                std::ostringstream summary;
                summary << "Save " << slot << "  "
                        << gameText.get(haru::game::localization::TextId::Day) << ' '
                        << save.stats.day << "  "
                        << gameText.get(haru::game::localization::TextId::ModStat) << ' '
                        << save.stats.modProgress;
                summaries.push_back(summary.str());
                ++slot;
            }
            return summaries;
        };
        window.setTitle(gameText.get(haru::game::localization::TextId::GameTitle));
        window.show();

        return app.run(game, [&](const haru::engine::core::FrameContext& frame) {
            const auto events = window.pollEvents();
            for (const auto& event : events) {
                if (event.kind == haru::engine::platform::WindowEventKind::CloseRequested) {
                    window.requestClose();
                } else if (event.kind ==
                               haru::engine::platform::WindowEventKind::MouseButtonReleased &&
                           event.button == haru::engine::platform::MouseButton::Left) {
                    const auto designPoint =
                        viewportScaler.mapPointToDesign({event.x, event.y},
                                                        {window.clientWidth(),
                                                         window.clientHeight()},
                                                        presenter.resolutionScalePercent());
                    if (!designPoint.has_value()) {
                        continue;
                    }
                    if (screen == GameScreen::Home) {
                        const auto action =
                            homeScene.actionAt(*designPoint,
                                               {surface.width(), surface.height()},
                                               homePanel,
                                               saveManager.saves().size());
                        if (action == haru::game::scenes::HomeAction::NewGame) {
                            dailyLoopState = haru::game::systems::DailyLoopState{};
                            saveManager.createNewGame(gameText.activeLocale());
                            screen = GameScreen::DailyLoop;
                        } else if (action == haru::game::scenes::HomeAction::OpenSaves) {
                            homePanel = haru::game::scenes::HomePanel::Saves;
                        } else if (action == haru::game::scenes::HomeAction::OpenSettings) {
                            homePanel = haru::game::scenes::HomePanel::Main;
                            screen = GameScreen::Settings;
                        } else if (action == haru::game::scenes::HomeAction::Back) {
                            homePanel = haru::game::scenes::HomePanel::Main;
                        } else if (action == haru::game::scenes::HomeAction::Quit) {
                            window.requestClose();
                        } else if (action.has_value()) {
                            const auto saveIndex = loadSaveIndex(*action);
                            if (saveIndex.has_value() &&
                                *saveIndex < saveManager.saves().size()) {
                                const auto save = saveManager.saves()[*saveIndex];
                                if (saveManager.activateSave(save.id)) {
                                    dailyLoopState =
                                        haru::game::systems::DailyLoopState(save.stats);
                                    applyLocale(save.localeTag.c_str(), false);
                                    homePanel = haru::game::scenes::HomePanel::Main;
                                    screen = GameScreen::DailyLoop;
                                }
                            }
                        }
                    } else if (screen == GameScreen::Settings) {
                        const auto action =
                            settingsScene.actionAt(*designPoint,
                                                   {surface.width(), surface.height()});
                        if (action ==
                            haru::game::scenes::SettingsAction::SetLocaleEnglish) {
                            applyLocale("en-US");
                        } else if (action == haru::game::scenes::SettingsAction::
                                                 SetLocaleSimplifiedChinese) {
                            applyLocale("zh-CN");
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::SetLocaleJapanese) {
                            applyLocale("ja-JP");
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::SelectGameTab) {
                            settingsState.activeTab = haru::game::scenes::SettingsTab::Game;
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::SelectAudioTab) {
                            settingsState.activeTab = haru::game::scenes::SettingsTab::Audio;
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::SelectDisplayTab) {
                            settingsState.activeTab = haru::game::scenes::SettingsTab::Display;
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::IncreaseMasterVolume) {
                            settingsState.masterVolume =
                                std::clamp(settingsState.masterVolume + 5, 0, 100);
                            persistSettings();
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::DecreaseMasterVolume) {
                            settingsState.masterVolume =
                                std::clamp(settingsState.masterVolume - 5, 0, 100);
                            persistSettings();
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::IncreaseBgmVolume) {
                            settingsState.bgmVolume =
                                std::clamp(settingsState.bgmVolume + 5, 0, 100);
                            persistSettings();
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::DecreaseBgmVolume) {
                            settingsState.bgmVolume =
                                std::clamp(settingsState.bgmVolume - 5, 0, 100);
                            persistSettings();
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::IncreaseSeVolume) {
                            settingsState.seVolume =
                                std::clamp(settingsState.seVolume + 5, 0, 100);
                            persistSettings();
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::DecreaseSeVolume) {
                            settingsState.seVolume =
                                std::clamp(settingsState.seVolume - 5, 0, 100);
                            persistSettings();
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::IncreaseWindowScale) {
                            settingsState.windowScale =
                                std::clamp(settingsState.windowScale + 10, 50, 200);
                            persistSettings();
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::DecreaseWindowScale) {
                            settingsState.windowScale =
                                std::clamp(settingsState.windowScale - 10, 50, 200);
                            persistSettings();
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::IncreaseTextSpeed) {
                            settingsState.textSpeed =
                                std::clamp(settingsState.textSpeed + 5, 0, 100);
                            persistSettings();
                            refreshSettingsScene();
                        } else if (action ==
                                   haru::game::scenes::SettingsAction::DecreaseTextSpeed) {
                            settingsState.textSpeed =
                                std::clamp(settingsState.textSpeed - 5, 0, 100);
                            persistSettings();
                            refreshSettingsScene();
                        } else if (action == haru::game::scenes::SettingsAction::Back) {
                            screen = GameScreen::Home;
                        }
                    } else if (screen == GameScreen::DailyLoop) {
                        const auto navigation =
                            dailyScene.navigationActionAt(*designPoint,
                                                          {surface.width(), surface.height()});
                        if (navigation == haru::game::scenes::TitleNavigationAction::ReturnHome) {
                            homePanel = haru::game::scenes::HomePanel::Main;
                            screen = GameScreen::Home;
                            continue;
                        }
                        const auto action =
                            dailyScene.actionAt(*designPoint,
                                                {surface.width(), surface.height()});
                        if (action.has_value()) {
                            dailyLoopState.apply(*action);
                            saveManager.recordDailyAction(*action,
                                                          dailyLoopState.stats(),
                                                          gameText.activeLocale());
                        }
                    }
                }
            }

            haru::engine::graphics::RenderQueue queue;
            if (screen == GameScreen::StudioSplash) {
                if (presenter.engineOpeningActive()) {
                    queue.clear({255, 255, 255, 255});
                } else {
                    studioSplashScene.update(frame.deltaSeconds);
                    if (studioSplashScene.active()) {
                        studioSplashScene.render(queue, {surface.width(), surface.height()});
                    } else {
                        screen = GameScreen::Home;
                    }
                }
            }

            if (screen == GameScreen::Home) {
                homeScene.render(queue,
                                 {surface.width(), surface.height()},
                                 homePanel,
                                 saveSummaries());
            } else if (screen == GameScreen::Settings) {
                settingsScene.render(queue, {surface.width(), surface.height()});
            } else if (screen == GameScreen::DailyLoop) {
                dailyScene.render(queue,
                                  {surface.width(), surface.height()},
                                  dailyLoopState.stats());
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
