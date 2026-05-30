# Text Render Splash Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Replace visible square placeholder text in the Win32 app with real UTF-8 text rendering and add an engine splash animation before the first UI screen.

**Architecture:** Keep placeholder text in the software surface for headless tests, but add a `SkipText` raster mode so native backends can draw real text after presenting the pixel surface. Implement the engine splash as a platform-neutral game scene state machine that emits existing render commands.

**Tech Stack:** C++17, existing test harness, Win32 GDI text rendering for the current native backend.

---

## File Structure

- Modify `src/engine/graphics/SoftwareSurface.hpp` and `.cpp` to add text rasterization modes.
- Modify `src/engine/platform/windows/Win32SoftwarePresenter.hpp` and `.cpp` to overlay text commands through GDI.
- Create `src/game/scenes/EngineSplashScene.hpp` and `.cpp` for the startup animation state machine.
- Modify `src/app/main.cpp` to use the splash scene and the native text overlay path.
- Modify `CMakeLists.txt` for new scene source and tests.
- Modify `test/engine/SoftwareSurfaceTests.cpp`.
- Create `test/game/EngineSplashSceneTests.cpp`.

### Task 1: Native Text Overlay Path

**Files:**
- Modify: `src/engine/graphics/SoftwareSurface.hpp`
- Modify: `src/engine/graphics/SoftwareSurface.cpp`
- Modify: `src/engine/platform/windows/Win32SoftwarePresenter.hpp`
- Modify: `src/engine/platform/windows/Win32SoftwarePresenter.cpp`
- Modify: `test/engine/SoftwareSurfaceTests.cpp`

- [x] **Step 1: Write failing test**

Add a test asserting `surface.draw(queue, TextRasterization::Skip)` leaves text pixels as background while the existing default path still paints placeholders.

- [x] **Step 2: Run failing build**

Run: `cmake --build build --config Debug --target harufushi_tests`

Expected: compile failure because `TextRasterization::Skip` and `draw(queue, mode)` do not exist.

- [x] **Step 3: Implement renderer behavior**

Add `enum class TextRasterization { Placeholder, Skip };` and overload/default `SoftwareSurface::draw`. Update Win32 presenter with `present(window, surface, queue)` that presents pixels first and then draws text commands with `DrawTextW` using a CJK-capable UI font.

- [x] **Step 4: Verify and commit**

Run: `ctest --test-dir build -C Debug --output-on-failure`

Commit: `fix: 使用gdi绘制真实文本`

### Task 2: Engine Splash Scene

**Files:**
- Create: `src/game/scenes/EngineSplashScene.hpp`
- Create: `src/game/scenes/EngineSplashScene.cpp`
- Create: `test/game/EngineSplashSceneTests.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write failing tests**

Add tests asserting the splash scene starts active, becomes complete after the configured duration, and emits at least clear, logo block, and text commands while active.

- [x] **Step 2: Run failing build**

Run: `cmake --build build --config Debug --target harufushi_tests`

Expected: compile failure because `EngineSplashScene` does not exist.

- [x] **Step 3: Implement state machine**

`EngineSplashScene` tracks elapsed seconds, exposes `active`, `complete`, `update`, and `render`. It emits a restrained splash composition: dark background, central frame mark, progress bar, and `Harufushi Frame` text.

- [x] **Step 4: Verify and commit**

Run: `ctest --test-dir build -C Debug --output-on-failure`

Commit: `feat: 添加引擎开屏动画`

### Task 3: App Integration

**Files:**
- Modify: `src/app/main.cpp`
- Modify: `docs/superpowers/plans/2026-05-30-text-render-splash.md`

- [x] **Step 1: Integrate startup flow**

Create `EngineSplashScene` in `main.cpp`. While it is active, render only the splash; after completion, render the existing first-screen button UI. Draw the software surface with `TextRasterization::Skip`, then present with text overlay.

- [x] **Step 2: Verify and commit**

Run: `cmake --build build --config Debug`

Run: `ctest --test-dir build -C Debug --output-on-failure`

Run: `.\bin\Debug\harufushi_app.exe --smoke`

Commit: `feat: 接入引擎开屏流程`
