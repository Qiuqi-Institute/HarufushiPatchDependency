# Render UI Core Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Build the first renderable vertical slice: platform-neutral draw commands, a software pixel surface, a project-owned UI tree, and Win32 presentation.

**Architecture:** Keep draw data and UI layout independent from Win32 so tests can validate behavior without opening a window. Use a simple software surface now, then present it through a Win32 GDI backend; later render backends can consume the same command stream.

**Tech Stack:** C++17, CMake, existing test harness, Win32 GDI for the first native presentation path.

---

## File Structure

- Create `src/engine/graphics/Color.hpp` for RGBA colors.
- Create `src/engine/graphics/Geometry.hpp` for `Point`, `Size`, and `Rect`.
- Create `src/engine/graphics/RenderQueue.hpp` and `RenderQueue.cpp` for draw command capture.
- Create `src/engine/graphics/SoftwareSurface.hpp` and `SoftwareSurface.cpp` for clipped software raster operations.
- Create `src/engine/ui/UiNode.hpp` and `UiNode.cpp` for a tiny project-owned UI tree.
- Create `src/engine/platform/windows/Win32SoftwarePresenter.hpp` and `Win32SoftwarePresenter.cpp` for GDI presentation.
- Modify `src/app/main.cpp` to draw a simple first frame every tick.
- Modify `CMakeLists.txt` to compile the new files and link `gdi32` on Windows.
- Create `test/engine/RenderQueueTests.cpp`, `test/engine/SoftwareSurfaceTests.cpp`, and `test/engine/UiNodeTests.cpp`.

### Task 1: Render Commands and Software Surface

**Files:**
- Create: `src/engine/graphics/Color.hpp`
- Create: `src/engine/graphics/Geometry.hpp`
- Create: `src/engine/graphics/RenderQueue.hpp`
- Create: `src/engine/graphics/RenderQueue.cpp`
- Create: `src/engine/graphics/SoftwareSurface.hpp`
- Create: `src/engine/graphics/SoftwareSurface.cpp`
- Create: `test/engine/RenderQueueTests.cpp`
- Create: `test/engine/SoftwareSurfaceTests.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write failing tests**

```cpp
HARU_TEST(render_queue_records_clear_and_rect_commands_in_order) {
    haru::engine::graphics::RenderQueue queue;
    queue.clear({10, 20, 30, 255});
    queue.fillRect({4, 5, 6, 7}, {1, 2, 3, 255});

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(2));
    HARU_EXPECT_EQ(queue.commands()[0].kind, haru::engine::graphics::DrawCommandKind::Clear);
    HARU_EXPECT_EQ(queue.commands()[1].rect.x, 4);
}
```

```cpp
HARU_TEST(software_surface_clips_filled_rectangles_to_surface_bounds) {
    haru::engine::graphics::SoftwareSurface surface(4, 4);
    surface.clear({0, 0, 0, 255});
    surface.fillRect({2, 2, 4, 4}, {200, 10, 20, 255});

    HARU_EXPECT_EQ(surface.pixelAt(3, 3), haru::engine::graphics::Color{200, 10, 20, 255});
    HARU_EXPECT_EQ(surface.pixelAt(1, 1), haru::engine::graphics::Color{0, 0, 0, 255});
}
```

- [x] **Step 2: Run failing build**

Run: `cmake --build build --config Debug --target harufushi_tests`

Expected: compile failure because graphics headers do not exist.

- [x] **Step 3: Implement minimal graphics core**

`RenderQueue` stores clear and filled-rectangle commands. `SoftwareSurface` owns a `std::vector<Color>`, supports `clear`, `fillRect`, `draw`, `pixelAt`, and clips writes to surface bounds.

- [x] **Step 4: Verify and commit**

Run: `ctest --test-dir build -C Debug --output-on-failure`

Commit: `feat: 添加软件渲染核心`

### Task 2: Project UI Tree

**Files:**
- Create: `src/engine/ui/UiNode.hpp`
- Create: `src/engine/ui/UiNode.cpp`
- Create: `test/engine/UiNodeTests.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write failing tests**

```cpp
HARU_TEST(ui_node_emits_background_rects_with_child_offsets) {
    using namespace haru::engine;

    ui::UiNode root({10, 20, 100, 80}, {20, 20, 24, 255});
    root.addChild(ui::UiNode({5, 6, 30, 12}, {180, 80, 120, 255}));

    graphics::RenderQueue queue;
    root.render(queue);

    HARU_EXPECT_EQ(queue.commands().size(), static_cast<std::size_t>(2));
    HARU_EXPECT_EQ(queue.commands()[1].rect.x, 15);
    HARU_EXPECT_EQ(queue.commands()[1].rect.y, 26);
}
```

- [x] **Step 2: Run failing build**

Run: `cmake --build build --config Debug --target harufushi_tests`

Expected: compile failure because `UiNode.hpp` does not exist.

- [x] **Step 3: Implement minimal UI tree**

`UiNode` owns local bounds, background color, and children. `render` appends background rectangles to a `RenderQueue`, offsetting child bounds by parent origin.

- [x] **Step 4: Verify and commit**

Run: `ctest --test-dir build -C Debug --output-on-failure`

Commit: `feat: 添加项目专用ui树`

### Task 3: Win32 Software Presentation

**Files:**
- Create: `src/engine/platform/windows/Win32SoftwarePresenter.hpp`
- Create: `src/engine/platform/windows/Win32SoftwarePresenter.cpp`
- Modify: `src/app/main.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Compile Win32 presenter**

Add the presenter source on Windows and link `gdi32`.

- [x] **Step 2: Implement presentation path**

`Win32SoftwarePresenter::present` takes a `Win32Window` and `SoftwareSurface`, prepares a top-down 32-bit DIB, and pushes pixels with `SetDIBitsToDevice`.

- [x] **Step 3: Draw a first scene**

`main.cpp` creates a `SoftwareSurface`, clears it, renders a simple UI root panel plus accent rectangles, draws the command queue into the surface, and presents every frame.

- [x] **Step 4: Verify and commit**

Run: `cmake --build build --config Debug`

Run: `ctest --test-dir build -C Debug --output-on-failure`

Run: `.\bin\Debug\harufushi_app.exe --smoke`

Commit: `feat: 接入win32软件呈现`
