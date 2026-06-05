#include "support/TestHarness.hpp"

#ifdef _WIN32

#include "engine/platform/windows/Win32Window.hpp"

#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace {

constexpr int harufushiAppIconResourceId = 101;

struct BitmapPixels {
    int width = 0;
    int height = 0;
    std::vector<std::uint32_t> pixels;
};

BitmapPixels pixelsForIcon(HICON icon) {
    ICONINFO iconInfo{};
    HARU_EXPECT_TRUE(GetIconInfo(icon, &iconInfo) != FALSE);

    BITMAP bitmap{};
    HARU_EXPECT_TRUE(GetObjectW(iconInfo.hbmColor,
                                sizeof(BITMAP),
                                &bitmap) != 0);

    BitmapPixels result;
    result.width = bitmap.bmWidth;
    result.height = bitmap.bmHeight;
    result.pixels.resize(static_cast<std::size_t>(result.width) *
                         static_cast<std::size_t>(result.height));

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = result.width;
    info.bmiHeader.biHeight = -result.height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    HDC deviceContext = GetDC(nullptr);
    HARU_EXPECT_TRUE(deviceContext != nullptr);
    const int scanLines = GetDIBits(deviceContext,
                                    iconInfo.hbmColor,
                                    0,
                                    static_cast<UINT>(result.height),
                                    result.pixels.data(),
                                    &info,
                                    DIB_RGB_COLORS);
    ReleaseDC(nullptr, deviceContext);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    HARU_EXPECT_EQ(scanLines, result.height);
    return result;
}

} // namespace

HARU_TEST(win32_window_updates_native_title_from_utf8_text) {
    haru::engine::platform::windows::Win32Window window({"Initial", 320, 240});

    window.setTitle("ハルフシ・パッチ・ディペンデンシー");

    wchar_t buffer[128]{};
    const int length = GetWindowTextW(window.nativeHandle(), buffer, 128);
    HARU_EXPECT_TRUE(length > 0);
    HARU_EXPECT_TRUE(std::wstring(buffer).find(L"ハルフシ") != std::wstring::npos);
}

HARU_TEST(win32_window_uses_app_resource_for_titlebar_small_icon) {
    const HINSTANCE instance = GetModuleHandleW(nullptr);
    const int iconWidth = GetSystemMetrics(SM_CXSMICON);
    const int iconHeight = GetSystemMetrics(SM_CYSMICON);
    HICON expectedIcon = static_cast<HICON>(LoadImageW(instance,
                                                       MAKEINTRESOURCEW(harufushiAppIconResourceId),
                                                       IMAGE_ICON,
                                                       iconWidth,
                                                       iconHeight,
                                                       LR_DEFAULTCOLOR));
    HARU_EXPECT_TRUE(expectedIcon != nullptr);

    haru::engine::platform::windows::Win32Window window({"Icon check", 320, 240});
    HICON actualIcon = reinterpret_cast<HICON>(
        SendMessageW(window.nativeHandle(), WM_GETICON, ICON_SMALL, 0));
    if (actualIcon == nullptr) {
        actualIcon = reinterpret_cast<HICON>(
            GetClassLongPtrW(window.nativeHandle(), GCLP_HICONSM));
    }

    HARU_EXPECT_TRUE(actualIcon != nullptr);
    const auto expectedPixels = pixelsForIcon(expectedIcon);
    const auto actualPixels = pixelsForIcon(actualIcon);
    HARU_EXPECT_EQ(actualPixels.width, expectedPixels.width);
    HARU_EXPECT_EQ(actualPixels.height, expectedPixels.height);
    HARU_EXPECT_EQ(actualPixels.pixels, expectedPixels.pixels);
    DestroyIcon(expectedIcon);
}

#endif
