#include "support/TestHarness.hpp"

#ifdef _WIN32

#include "engine/platform/windows/Win32Window.hpp"

#include <windows.h>

#include <string>

HARU_TEST(win32_window_updates_native_title_from_utf8_text) {
    haru::engine::platform::windows::Win32Window window({"Initial", 320, 240});

    window.setTitle("ハルフシ・パッチ・ディペンデンシー");

    wchar_t buffer[128]{};
    const int length = GetWindowTextW(window.nativeHandle(), buffer, 128);
    HARU_EXPECT_TRUE(length > 0);
    HARU_EXPECT_TRUE(std::wstring(buffer).find(L"ハルフシ") != std::wstring::npos);
}

#endif
