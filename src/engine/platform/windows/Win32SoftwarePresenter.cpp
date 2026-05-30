#include "engine/platform/windows/Win32SoftwarePresenter.hpp"

#include <stdexcept>
#include <string>
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

std::wstring utf8ToWide(const std::string& text) {
    if (text.empty()) {
        return {};
    }

    const int required = MultiByteToWideChar(CP_UTF8, 0, text.data(),
                                             static_cast<int>(text.size()), nullptr, 0);
    if (required <= 0) {
        throw std::runtime_error("failed to convert text command to UTF-16");
    }

    std::wstring result(static_cast<std::size_t>(required), L'\0');
    const int written = MultiByteToWideChar(CP_UTF8, 0, text.data(),
                                            static_cast<int>(text.size()), result.data(),
                                            required);
    if (written != required) {
        throw std::runtime_error("failed to write UTF-16 text command");
    }

    return result;
}

COLORREF toColorRef(graphics::Color color) {
    return RGB(color.r, color.g, color.b);
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

void Win32SoftwarePresenter::present(const Win32Window& window,
                                     const graphics::SoftwareSurface& surface,
                                     const graphics::RenderQueue& textSource) const {
    present(window, surface);

    HDC deviceContext = GetDC(window.nativeHandle());
    if (deviceContext == nullptr) {
        throw std::runtime_error("failed to acquire window device context for text");
    }

    HFONT font = CreateFontW(-20,
                             0,
                             0,
                             0,
                             FW_SEMIBOLD,
                             FALSE,
                             FALSE,
                             FALSE,
                             DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY,
                             DEFAULT_PITCH | FF_DONTCARE,
                             L"Microsoft YaHei UI");
    const HGDIOBJ previousFont = font != nullptr ? SelectObject(deviceContext, font) : nullptr;
    const int previousBkMode = SetBkMode(deviceContext, TRANSPARENT);

    for (const auto& command : textSource.commands()) {
        if (command.kind != graphics::DrawCommandKind::Text || command.text.empty()) {
            continue;
        }

        SetTextColor(deviceContext, toColorRef(command.color));
        RECT rect{command.rect.x,
                  command.rect.y,
                  command.rect.x + command.rect.width,
                  command.rect.y + command.rect.height};
        const std::wstring text = utf8ToWide(command.text);
        DrawTextW(deviceContext,
                  text.c_str(),
                  static_cast<int>(text.size()),
                  &rect,
                  DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    SetBkMode(deviceContext, previousBkMode);
    if (previousFont != nullptr) {
        SelectObject(deviceContext, previousFont);
    }
    if (font != nullptr) {
        DeleteObject(font);
    }

    ReleaseDC(window.nativeHandle(), deviceContext);
}

} // namespace haru::engine::platform::windows
