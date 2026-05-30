# Native Window Loop Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Build the first native Windows window and deterministic frame-loop foundation for the custom C++ game frame.

**Architecture:** Keep platform-neutral loop behavior in `src/engine/core/` so it can be tested without creating OS windows. Put Win32 window creation and message polling behind `src/engine/platform/windows/`, and expose only opaque platform behavior to application startup.

**Tech Stack:** C++17, CMake, Win32 API on Windows, existing in-repo test harness.

---

## File Structure

- Create `src/engine/core/FrameLoop.hpp` and `src/engine/core/FrameLoop.cpp` for platform-neutral loop control.
- Create `src/engine/platform/Window.hpp` for the window abstraction and config.
- Create `src/engine/platform/windows/Win32Window.hpp` and `src/engine/platform/windows/Win32Window.cpp` for native Windows implementation.
- Modify `src/engine/core/Application.hpp` and `src/engine/core/Application.cpp` so the app can run a bounded frame loop after runtime startup.
- Modify `src/app/main.cpp` to create a native window on Windows and run the application through it.
- Modify `CMakeLists.txt` to compile the new files and link `user32` on Windows.
- Create `test/engine/FrameLoopTests.cpp` and `test/engine/WindowConfigTests.cpp`.

### Task 1: Platform-Neutral Frame Loop

**Files:**
- Create: `src/engine/core/FrameLoop.hpp`
- Create: `src/engine/core/FrameLoop.cpp`
- Create: `test/engine/FrameLoopTests.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write the failing tests**

```cpp
HARU_TEST(frame_loop_stops_when_runtime_requests_stop) {
    haru::engine::core::FrameLoop loop;
    int ticks = 0;

    const auto result = loop.run([&](const haru::engine::core::FrameContext& context) {
        ++ticks;
        HARU_EXPECT_EQ(context.frameIndex, static_cast<std::uint64_t>(ticks - 1));
        if (ticks == 3) {
            return haru::engine::core::LoopDecision::Stop;
        }
        return haru::engine::core::LoopDecision::Continue;
    });

    HARU_EXPECT_EQ(result.framesRun, static_cast<std::uint64_t>(3));
    HARU_EXPECT_EQ(ticks, 3);
}
```

- [x] **Step 2: Run the test to verify it fails**

Run: `cmake --build build --config Debug --target harufushi_tests`

Expected: compile failure because `FrameLoop.hpp` does not exist.

- [x] **Step 3: Implement minimal frame loop**

```cpp
enum class LoopDecision { Continue, Stop };

struct FrameContext {
    std::uint64_t frameIndex;
    double deltaSeconds;
};

struct FrameLoopResult {
    std::uint64_t framesRun;
};
```

`FrameLoop::run` increments frame index, invokes the callback, and exits when callback returns `Stop`.

- [x] **Step 4: Run tests and commit**

Run: `ctest --test-dir build -C Debug --output-on-failure`

Commit: `feat: 添加平台中立帧循环`

### Task 2: Window Configuration Contract

**Files:**
- Create: `src/engine/platform/Window.hpp`
- Create: `test/engine/WindowConfigTests.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write the failing tests**

```cpp
HARU_TEST(window_config_rejects_empty_title_and_zero_size) {
    using haru::engine::platform::WindowConfig;

    HARU_EXPECT_FALSE(WindowConfig{"", 1280, 720}.valid());
    HARU_EXPECT_FALSE(WindowConfig{"春伏补丁依存症", 0, 720}.valid());
    HARU_EXPECT_FALSE(WindowConfig{"春伏补丁依存症", 1280, 0}.valid());
    HARU_EXPECT_TRUE(WindowConfig{"春伏补丁依存症", 1280, 720}.valid());
}
```

- [x] **Step 2: Run the test to verify it fails**

Run: `cmake --build build --config Debug --target harufushi_tests`

Expected: compile failure because `WindowConfig` does not exist.

- [x] **Step 3: Implement minimal abstraction**

`Window.hpp` defines `WindowConfig`, `WindowEvent`, `WindowEventKind`, and abstract `Window` with `show`, `pollEvents`, `requestClose`, `shouldClose`.

- [x] **Step 4: Run tests and commit**

Run: `ctest --test-dir build -C Debug --output-on-failure`

Commit: `feat: 定义窗口平台抽象`

### Task 3: Application Loop Integration

**Files:**
- Modify: `src/engine/core/Application.hpp`
- Modify: `src/engine/core/Application.cpp`
- Modify: `test/engine/ApplicationTests.cpp`

- [x] **Step 1: Write the failing test**

```cpp
HARU_TEST(application_runs_until_frame_callback_stops) {
    haru::game::HarufushiGame game;
    haru::engine::core::Application app({"春伏补丁依存症", "0.0.1"});
    int frames = 0;

    const int exitCode = app.run(game, [&](const haru::engine::core::FrameContext&) {
        ++frames;
        return frames == 2
            ? haru::engine::core::LoopDecision::Stop
            : haru::engine::core::LoopDecision::Continue;
    });

    HARU_EXPECT_EQ(exitCode, 0);
    HARU_EXPECT_EQ(frames, 2);
    HARU_EXPECT_EQ(game.startCount(), 1);
}
```

- [x] **Step 2: Run the test to verify it fails**

Run: `cmake --build build --config Debug --target harufushi_tests`

Expected: compile failure because `Application::run` does not accept a frame callback.

- [x] **Step 3: Implement minimal integration**

Application starts `GameRuntime`, then uses `FrameLoop` when a frame callback is provided. Existing `run(GameRuntime&)` remains as one-start bootstrap behavior for tests.

- [x] **Step 4: Run tests and commit**

Run: `ctest --test-dir build -C Debug --output-on-failure`

Commit: `feat: 接入应用帧循环`

### Task 4: Win32 Native Window Backend

**Files:**
- Create: `src/engine/platform/windows/Win32Window.hpp`
- Create: `src/engine/platform/windows/Win32Window.cpp`
- Modify: `src/app/main.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Add compile coverage**

Update CMake so `Win32Window.cpp` compiles only on Windows and links `user32`.

- [x] **Step 2: Implement native backend**

`Win32Window` registers a private window class, creates a Unicode Win32 window, polls messages with `PeekMessageW`, maps close events to `WindowEventKind::CloseRequested`, and exposes `nativeHandle`.

- [x] **Step 3: Wire app startup**

`main.cpp` creates `Win32Window`, shows it, then runs frames until the window requests close. Non-Windows builds keep console bootstrap behavior.

- [x] **Step 4: Build, run, and commit**

Run: `cmake --build build --config Debug`

Run: `ctest --test-dir build -C Debug --output-on-failure`

Commit: `feat: 建立原生窗口循环`
