# Text Button UI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Extend the current UI/rendering slice with text placeholder commands and a basic project-owned button control.

**Architecture:** Keep text as a render command so later font backends can replace the software placeholder without changing UI code. Keep button behavior in `src/engine/ui/` and let it emit only generic render commands.

**Tech Stack:** C++17, existing in-repo test harness, current software surface renderer.

---

## File Structure

- Modify `src/engine/graphics/RenderQueue.hpp` and `RenderQueue.cpp` to add text draw commands.
- Modify `src/engine/graphics/SoftwareSurface.cpp` to draw deterministic placeholder glyph blocks for text commands.
- Modify `src/engine/ui/UiNode.hpp` and `UiNode.cpp` to support optional text.
- Create `src/engine/ui/Button.hpp` and `Button.cpp` for a basic renderable button control.
- Modify `src/app/main.cpp` to render a small first-screen button set.
- Modify `CMakeLists.txt` for the new UI source and tests.
- Modify/add tests in `test/engine/RenderQueueTests.cpp`, `SoftwareSurfaceTests.cpp`, `UiNodeTests.cpp`, and `ButtonTests.cpp`.

### Task 1: Text Draw Command

**Files:**
- Modify: `src/engine/graphics/RenderQueue.hpp`
- Modify: `src/engine/graphics/RenderQueue.cpp`
- Modify: `src/engine/graphics/SoftwareSurface.cpp`
- Modify: `test/engine/RenderQueueTests.cpp`
- Modify: `test/engine/SoftwareSurfaceTests.cpp`

- [x] **Step 1: Write failing tests**

Add tests that call `queue.drawText({1, 2, 80, 20}, "秋起", color)` and assert the command kind, bounds, text, and color.

Add a software surface test that draws a text command over a cleared background and asserts at least one pixel inside the text bounds changes while a pixel outside stays as background.

- [x] **Step 2: Run failing build**

Run: `cmake --build build --config Debug --target harufushi_tests`

Expected: compile failure because `RenderQueue::drawText` and `DrawCommandKind::Text` do not exist.

- [x] **Step 3: Implement minimal text placeholder**

Add `Text` to `DrawCommandKind`, add `std::string text` to `DrawCommand`, and implement `RenderQueue::drawText`. `SoftwareSurface::draw` handles text by painting fixed-size clipped blocks per non-space character.

- [x] **Step 4: Verify and commit**

Run: `ctest --test-dir build -C Debug --output-on-failure`

Commit: `feat: 添加文本占位绘制命令`

### Task 2: Text Nodes and Buttons

**Files:**
- Modify: `src/engine/ui/UiNode.hpp`
- Modify: `src/engine/ui/UiNode.cpp`
- Create: `src/engine/ui/Button.hpp`
- Create: `src/engine/ui/Button.cpp`
- Modify: `test/engine/UiNodeTests.cpp`
- Create: `test/engine/ButtonTests.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write failing tests**

Add a `UiNode` test asserting `setText("春伏", color)` emits a text command after the background command.

Add a `Button` test asserting `Button({10, 20, 100, 32}, "写 Mod")` emits a background command and text command, and `contains({20, 30})` is true while `contains({1, 1})` is false.

- [x] **Step 2: Run failing build**

Run: `cmake --build build --config Debug --target harufushi_tests`

Expected: compile failure because `UiNode::setText` and `Button.hpp` do not exist.

- [x] **Step 3: Implement controls**

`UiNode` stores optional text and text color. `Button` stores bounds, label, style colors, exposes `contains`, and emits background plus padded text.

- [x] **Step 4: Verify and commit**

Run: `ctest --test-dir build -C Debug --output-on-failure`

Commit: `feat: 添加基础按钮控件`

### Task 3: App First-Screen UI

**Files:**
- Modify: `src/app/main.cpp`

- [x] **Step 1: Use button control in app**

Replace hard-coded accent rectangles with button controls for `Study`, `Modding`, and `Harufushi`.

- [x] **Step 2: Verify and commit**

Run: `cmake --build build --config Debug`

Run: `ctest --test-dir build -C Debug --output-on-failure`

Run: `.\bin\Debug\harufushi_app.exe --smoke`

Commit: `feat: 绘制首屏基础按钮`
