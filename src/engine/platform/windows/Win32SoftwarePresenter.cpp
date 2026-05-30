#include "engine/platform/windows/Win32SoftwarePresenter.hpp"

#include <cstring>
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

HFONT createFontForText(const std::string& text) {
    const bool splashTitle = text == "Harufushi Frame";
    return CreateFontW(splashTitle ? -42 : -20,
                       0,
                       0,
                       0,
                       splashTitle ? FW_BOLD : FW_SEMIBOLD,
                       FALSE,
                       FALSE,
                       FALSE,
                       DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS,
                       CLEARTYPE_QUALITY,
                       DEFAULT_PITCH | FF_DONTCARE,
                       splashTitle ? L"Segoe Print" : L"Microsoft YaHei UI");
}

void drawTextCommands(HDC deviceContext, const graphics::RenderQueue& textSource) {
    const int previousBkMode = SetBkMode(deviceContext, TRANSPARENT);

    for (const auto& command : textSource.commands()) {
        if (command.kind != graphics::DrawCommandKind::Text || command.text.empty()) {
            continue;
        }

        HFONT font = createFontForText(command.text);
        const HGDIOBJ previousFont =
            font != nullptr ? SelectObject(deviceContext, font) : nullptr;

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
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        if (previousFont != nullptr) {
            SelectObject(deviceContext, previousFont);
        }
        if (font != nullptr) {
            DeleteObject(font);
        }
    }

    SetBkMode(deviceContext, previousBkMode);
}

void presentComposited(const Win32Window& window,
                       const graphics::SoftwareSurface& surface,
                       const graphics::RenderQueue* textSource) {
    HDC windowContext = GetDC(window.nativeHandle());
    if (windowContext == nullptr) {
        throw std::runtime_error("failed to acquire window device context");
    }

    HDC memoryContext = CreateCompatibleDC(windowContext);
    if (memoryContext == nullptr) {
        ReleaseDC(window.nativeHandle(), windowContext);
        throw std::runtime_error("failed to create offscreen device context");
    }

    BITMAPINFO bitmapInfo{};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = surface.width();
    bitmapInfo.bmiHeader.biHeight = -surface.height();
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    void* bitmapBits = nullptr;
    HBITMAP bitmap = CreateDIBSection(windowContext,
                                      &bitmapInfo,
                                      DIB_RGB_COLORS,
                                      &bitmapBits,
                                      nullptr,
                                      0);
    if (bitmap == nullptr || bitmapBits == nullptr) {
        DeleteDC(memoryContext);
        ReleaseDC(window.nativeHandle(), windowContext);
        throw std::runtime_error("failed to create offscreen DIB section");
    }

    const HGDIOBJ previousBitmap = SelectObject(memoryContext, bitmap);
    const auto pixels = toBgraPixels(surface);
    std::memcpy(bitmapBits, pixels.data(), pixels.size() * sizeof(std::uint32_t));

    if (textSource != nullptr) {
        drawTextCommands(memoryContext, *textSource);
    }

    const BOOL copied = BitBlt(windowContext,
                               0,
                               0,
                               surface.width(),
                               surface.height(),
                               memoryContext,
                               0,
                               0,
                               SRCCOPY);

    SelectObject(memoryContext, previousBitmap);
    DeleteObject(bitmap);
    DeleteDC(memoryContext);
    ReleaseDC(window.nativeHandle(), windowContext);

    if (copied == FALSE) {
        throw std::runtime_error("failed to present composited software surface");
    }
}

} // namespace

void Win32SoftwarePresenter::present(const Win32Window& window,
                                     const graphics::SoftwareSurface& surface) const {
    presentComposited(window, surface, nullptr);
}

void Win32SoftwarePresenter::present(const Win32Window& window,
                                     const graphics::SoftwareSurface& surface,
                                     const graphics::RenderQueue& textSource) const {
    presentComposited(window, surface, &textSource);
}

} // namespace haru::engine::platform::windows
