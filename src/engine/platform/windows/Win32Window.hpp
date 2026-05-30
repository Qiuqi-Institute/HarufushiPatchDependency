#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "../Window.hpp"

#include <windows.h>

#include <vector>

namespace haru::engine::platform::windows {

class Win32Window final : public Window {
public:
    explicit Win32Window(WindowConfig config);
    ~Win32Window() override;

    Win32Window(const Win32Window&) = delete;
    Win32Window& operator=(const Win32Window&) = delete;

    void show() override;
    std::vector<WindowEvent> pollEvents() override;
    void requestClose() override;
    bool shouldClose() const override;

    HWND nativeHandle() const;

private:
    static ATOM ensureWindowClass(HINSTANCE instance);
    static LRESULT CALLBACK staticWindowProc(HWND handle,
                                             UINT message,
                                             WPARAM wordParam,
                                             LPARAM longParam);

    LRESULT handleMessage(UINT message, WPARAM wordParam, LPARAM longParam);

    WindowConfig config_;
    HWND handle_ = nullptr;
    bool closeRequested_ = false;
    std::vector<WindowEvent> pendingEvents_;
};

} // namespace haru::engine::platform::windows
