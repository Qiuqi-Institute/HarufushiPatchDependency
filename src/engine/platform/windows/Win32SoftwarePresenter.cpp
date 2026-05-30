#include "engine/platform/windows/Win32SoftwarePresenter.hpp"

#include <stdexcept>
#include <vector>

namespace haru::engine::platform::windows {

namespace {

std::vector<std::uint32_t> toBgraPixels(const graphics::SoftwareSurface& surface) {
    std::vector<std::uint32_t> pixels;
    pixels.reserve(surface.pixels().size());

    for (const auto& color : surface.pixels()) {
        pixels.push_back(static_cast<std::uint32_t>(color.b) |
                         (static_cast<std::uint32_t>(color.g) << 8U) |
                         (static_cast<std::uint32_t>(color.r) << 16U) |
                         (static_cast<std::uint32_t>(color.a) << 24U));
    }

    return pixels;
}

} // namespace

void Win32SoftwarePresenter::present(const Win32Window& window,
                                     const graphics::SoftwareSurface& surface) const {
    HDC deviceContext = GetDC(window.nativeHandle());
    if (deviceContext == nullptr) {
        throw std::runtime_error("failed to acquire window device context");
    }

    BITMAPINFO bitmapInfo{};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = surface.width();
    bitmapInfo.bmiHeader.biHeight = -surface.height();
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    const auto pixels = toBgraPixels(surface);
    const int scanLines = SetDIBitsToDevice(deviceContext,
                                            0,
                                            0,
                                            static_cast<DWORD>(surface.width()),
                                            static_cast<DWORD>(surface.height()),
                                            0,
                                            0,
                                            0,
                                            static_cast<UINT>(surface.height()),
                                            pixels.data(),
                                            &bitmapInfo,
                                            DIB_RGB_COLORS);

    ReleaseDC(window.nativeHandle(), deviceContext);

    if (scanLines == 0) {
        throw std::runtime_error("failed to present software surface");
    }
}

} // namespace haru::engine::platform::windows
