cmake_minimum_required(VERSION 3.24)

project(HarufushiPatchDependency LANGUAGES CXX)

include(cmake/ProjectOptions.cmake)

set(HARUFUSHI_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

function(harufushi_configure_engine_module target_name output_name)
    target_include_directories(${target_name}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/include
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_compile_features(${target_name} PUBLIC cxx_std_17)
    harufushi_apply_common_options(${target_name})
    set_target_properties(${target_name} PROPERTIES
        OUTPUT_NAME ${output_name}
        RUNTIME_OUTPUT_DIRECTORY ${HARUFUSHI_RUNTIME_OUTPUT_DIRECTORY}
        LIBRARY_OUTPUT_DIRECTORY ${HARUFUSHI_RUNTIME_OUTPUT_DIRECTORY}
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endfunction()

add_library(harufushi_engine_core SHARED
    src/engine/core/Application.cpp
    src/engine/core/FrameLoop.cpp
    src/engine/core/HaruFrame.cpp
    src/engine/graphics/RenderQueue.cpp
    src/engine/graphics/SoftwareSurface.cpp
)

harufushi_configure_engine_module(harufushi_engine_core HarufushiEngine)

add_custom_command(TARGET harufushi_engine_core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/harufushi_engine.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/harufushi_engine.pdb
)

add_library(harufushi_engine_localization SHARED
    src/engine/localization/LocaleRegistry.cpp
)

harufushi_configure_engine_module(harufushi_engine_localization HarufushiEngine.Localization)
target_link_libraries(harufushi_engine_localization PUBLIC harufushi_engine_core)

add_library(harufushi_engine_resources SHARED
    src/engine/resources/ProtectedResourceStore.cpp
    src/engine/resources/ResourceId.cpp
    src/engine/resources/ResourceManifest.cpp
)

harufushi_configure_engine_module(harufushi_engine_resources HarufushiEngine.Resources)
target_link_libraries(harufushi_engine_resources PUBLIC harufushi_engine_core)

add_library(harufushi_engine_ui SHARED
    src/engine/ui/Button.cpp
    src/engine/ui/UiNode.cpp
)

harufushi_configure_engine_module(harufushi_engine_ui HarufushiEngine.UI)
target_link_libraries(harufushi_engine_ui PUBLIC harufushi_engine_core)

add_library(harufushi_engine_platform SHARED
    src/engine/platform/Window.cpp
)

harufushi_configure_engine_module(harufushi_engine_platform HarufushiEngine.Platform)
target_link_libraries(harufushi_engine_platform PUBLIC harufushi_engine_core)

if(WIN32)
    target_sources(harufushi_engine_platform
        PRIVATE
            src/engine/platform/windows/Win32SoftwarePresenter.cpp
            src/engine/platform/windows/Win32Window.cpp
    )
    target_link_libraries(harufushi_engine_platform PUBLIC gdi32 user32)
endif()

add_library(harufushi_engine_modules INTERFACE)
target_link_libraries(harufushi_engine_modules
    INTERFACE
        harufushi_engine_core
        harufushi_engine_localization
        harufushi_engine_resources
        harufushi_engine_ui
        harufushi_engine_platform
)

add_library(harufushi_game STATIC
    src/game/systems/HarufushiGame.cpp
)

target_include_directories(harufushi_game
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/game
)

target_link_libraries(harufushi_game PUBLIC harufushi_engine_modules)
target_compile_features(harufushi_game PUBLIC cxx_std_17)
harufushi_apply_common_options(harufushi_game)

add_executable(harufushi_app
    src/app/main.cpp
)

target_link_libraries(harufushi_app PRIVATE harufushi_game)
target_compile_features(harufushi_app PRIVATE cxx_std_17)
harufushi_apply_common_options(harufushi_app)

set_target_properties(harufushi_app PROPERTIES
    OUTPUT_NAME HarufushiPatchDependency
    RUNTIME_OUTPUT_DIRECTORY ${HARUFUSHI_RUNTIME_OUTPUT_DIRECTORY}
)

if(WIN32)
    set_target_properties(harufushi_app PROPERTIES WIN32_EXECUTABLE TRUE)
    if(MSVC)
        target_link_options(harufushi_app PRIVATE /ENTRY:mainCRTStartup)
    endif()
endif()

add_custom_command(TARGET harufushi_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/harufushi_app.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/harufushi_app.pdb
)

enable_testing()

add_executable(harufushi_public_api_smoke
    test/engine/PublicApiSmoke.cpp
)

target_link_libraries(harufushi_public_api_smoke PRIVATE harufushi_engine_modules)
target_compile_features(harufushi_public_api_smoke PRIVATE cxx_std_17)
harufushi_apply_common_options(harufushi_public_api_smoke)

set_target_properties(harufushi_public_api_smoke PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${HARUFUSHI_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(harufushi_tests
    test/Main.cpp
    test/support/TestHarness.cpp
    test/engine/ApplicationTests.cpp
    test/engine/ButtonTests.cpp
    test/engine/FrameLoopTests.cpp
    test/engine/HaruFrameTests.cpp
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

target_link_libraries(harufushi_tests PRIVATE harufushi_engine_modules harufushi_game)
target_compile_features(harufushi_tests PRIVATE cxx_std_17)
harufushi_apply_common_options(harufushi_tests)

set_target_properties(harufushi_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${HARUFUSHI_RUNTIME_OUTPUT_DIRECTORY}
)

add_test(NAME harufushi_tests COMMAND harufushi_tests)
add_test(NAME harufushi_public_api_smoke COMMAND harufushi_public_api_smoke)
add_test(
    NAME harufushi_engine_dll_layout
    COMMAND ${CMAKE_COMMAND}
        -DCORE_BINARY=$<TARGET_FILE:harufushi_engine_core>
        -DUI_BINARY=$<TARGET_FILE:harufushi_engine_ui>
        -DPLATFORM_BINARY=$<TARGET_FILE:harufushi_engine_platform>
        -DRESOURCES_BINARY=$<TARGET_FILE:harufushi_engine_resources>
        -DLOCALIZATION_BINARY=$<TARGET_FILE:harufushi_engine_localization>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test/cmake/AssertEngineDll.cmake
)
add_test(
    NAME haru_frame_public_header_gate
    COMMAND ${CMAKE_COMMAND}
        -DPUBLIC_INCLUDE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/src/engine/include
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test/cmake/AssertHaruFramePublicHeaders.cmake
)
add_test(
    NAME harufushi_app_executable_layout
    COMMAND ${CMAKE_COMMAND}
        -DAPP_BINARY=$<TARGET_FILE:harufushi_app>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test/cmake/AssertAppExecutable.cmake
)

if(WIN32)
    add_test(
        NAME harufushi_app_windows_gui_subsystem
        COMMAND powershell
            -NoProfile
            -ExecutionPolicy Bypass
            -File ${CMAKE_CURRENT_SOURCE_DIR}/test/cmake/AssertWindowsGuiSubsystem.ps1
            -AppBinary $<TARGET_FILE:harufushi_app>
    )
endif()
