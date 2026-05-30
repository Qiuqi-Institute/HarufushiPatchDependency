cmake_minimum_required(VERSION 3.24)

project(HarufushiPatchDependency LANGUAGES CXX)

include(cmake/ProjectOptions.cmake)

add_library(harufushi_engine STATIC
    src/engine/core/Application.cpp
    src/engine/core/FrameLoop.cpp
    src/engine/graphics/RenderQueue.cpp
    src/engine/graphics/SoftwareSurface.cpp
    src/engine/localization/LocaleRegistry.cpp
    src/engine/platform/Window.cpp
    src/engine/resources/ProtectedResourceStore.cpp
    src/engine/resources/ResourceId.cpp
    src/engine/resources/ResourceManifest.cpp
    src/engine/ui/UiNode.cpp
)

target_include_directories(harufushi_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(harufushi_engine PUBLIC cxx_std_17)
harufushi_apply_common_options(harufushi_engine)

if(WIN32)
    target_sources(harufushi_engine
        PRIVATE
            src/engine/platform/windows/Win32SoftwarePresenter.cpp
            src/engine/platform/windows/Win32Window.cpp
    )
    target_link_libraries(harufushi_engine PUBLIC gdi32 user32)
endif()

add_library(harufushi_game STATIC
    src/game/systems/HarufushiGame.cpp
)

target_include_directories(harufushi_game
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(harufushi_game PUBLIC harufushi_engine)
target_compile_features(harufushi_game PUBLIC cxx_std_17)
harufushi_apply_common_options(harufushi_game)

add_executable(harufushi_app
    src/app/main.cpp
)

target_link_libraries(harufushi_app PRIVATE harufushi_game)
target_compile_features(harufushi_app PRIVATE cxx_std_17)
harufushi_apply_common_options(harufushi_app)

set_target_properties(harufushi_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

enable_testing()

add_executable(harufushi_tests
    test/Main.cpp
    test/support/TestHarness.cpp
    test/engine/ApplicationTests.cpp
    test/engine/FrameLoopTests.cpp
    test/engine/LocalizationTests.cpp
    test/engine/RenderQueueTests.cpp
    test/engine/ResourceTests.cpp
    test/engine/SoftwareSurfaceTests.cpp
    test/engine/UiNodeTests.cpp
    test/engine/WindowConfigTests.cpp
)

target_include_directories(harufushi_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/test
)

target_link_libraries(harufushi_tests PRIVATE harufushi_engine harufushi_game)
target_compile_features(harufushi_tests PRIVATE cxx_std_17)
harufushi_apply_common_options(harufushi_tests)

set_target_properties(harufushi_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

add_test(NAME harufushi_tests COMMAND harufushi_tests)
