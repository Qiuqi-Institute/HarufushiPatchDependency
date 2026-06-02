#include "engine/platform/windows/Win32SoftwarePresenter.hpp"
#include "engine/graphics/ViewportScaler.hpp"

#include <algorithm>
#include <cmath>
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

RECT clientRect(const Win32Window& window) {
    RECT rect{};
    if (GetClientRect(window.nativeHandle(), &rect) == FALSE) {
        throw std::runtime_error("failed to query window client rect");
    }
    return rect;
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

bool isSplashTitleLetter(const graphics::DrawCommand& command) {
    return command.text.size() == 1U && command.rect.height >= 60 &&
           command.color == graphics::Color{11, 119, 155, 255};
}

graphics::Rect scaleRect(graphics::Rect rect,
                         graphics::Rect presentationRect,
                         const graphics::SoftwareSurface& surface) {
    return {presentationRect.x + (rect.x * presentationRect.width) / surface.width(),
            presentationRect.y + (rect.y * presentationRect.height) / surface.height(),
            std::max(1, (rect.width * presentationRect.width) / surface.width()),
            std::max(1, (rect.height * presentationRect.height) / surface.height())};
}

HFONT createFontForText(const graphics::DrawCommand& command, double scale) {
    const bool splashTitle = isSplashTitleLetter(command);
    const bool displayText = !splashTitle && command.rect.height >= 56 &&
                             command.rect.width <= 600;
    const bool sceneTitle = !splashTitle && !displayText && command.rect.height >= 40 &&
                            command.rect.width >= 520;
    const int baseFontSize = displayText ? 54 : (splashTitle ? 42 : (sceneTitle ? 30 : 20));
    const int fontSize =
        -std::max(1, static_cast<int>(std::round(static_cast<double>(baseFontSize) * scale)));
    return CreateFontW(fontSize,
                       0,
                       0,
                       0,
                       (splashTitle || displayText || sceneTitle) ? FW_BOLD : FW_SEMIBOLD,
                       FALSE,
                       FALSE,
                       FALSE,
                       DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS,
                       CLEARTYPE_QUALITY,
                       DEFAULT_PITCH | FF_DONTCARE,
                       displayText ? L"Bahnschrift" :
                                     (splashTitle ? L"Segoe Print" :
                                                    (sceneTitle ? L"Yu Mincho" :
                                                                  L"Yu Gothic UI")));
}

void drawTextCommands(HDC deviceContext,
                      const graphics::RenderQueue& textSource,
                      graphics::Rect presentationRect,
                      const graphics::SoftwareSurface& surface) {
    const int previousBkMode = SetBkMode(deviceContext, TRANSPARENT);
    const double scale = static_cast<double>(presentationRect.height) /
                         static_cast<double>(std::max(surface.height(), 1));

    for (const auto& command : textSource.commands()) {
        if (command.kind != graphics::DrawCommandKind::Text || command.text.empty()) {
            continue;
        }

        const graphics::Rect scaledRect = scaleRect(command.rect, presentationRect, surface);
        graphics::DrawCommand scaledCommand = command;
        scaledCommand.rect = scaledRect;
        HFONT font = createFontForText(scaledCommand, scale);
        const HGDIOBJ previousFont =
            font != nullptr ? SelectObject(deviceContext, font) : nullptr;

        SetTextColor(deviceContext, toColorRef(command.color));
        RECT rect{scaledRect.x,
                  scaledRect.y,
                  scaledRect.x + scaledRect.width,
                  scaledRect.y + scaledRect.height};
        const std::wstring text = utf8ToWide(command.text);
        const bool splashTitleLetter = isSplashTitleLetter(command);
        const UINT format =
            (splashTitleLetter ? (DT_LEFT | DT_NOCLIP) : (DT_CENTER | DT_END_ELLIPSIS)) |
            DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX;
        DrawTextW(deviceContext,
                  text.c_str(),
                  static_cast<int>(text.size()),
                  &rect,
                  format);

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
                       const graphics::RenderQueue* textSource,
                       int resolutionScalePercent) {
    HDC windowContext = GetDC(window.nativeHandle());
    if (windowContext == nullptr) {
        throw std::runtime_error("failed to acquire window device context");
    }

    HDC memoryContext = CreateCompatibleDC(windowContext);
    if (memoryContext == nullptr) {
        ReleaseDC(window.nativeHandle(), windowContext);
        throw std::runtime_error("failed to create offscreen device context");
    }

    const RECT targetClient = clientRect(window);
    const int targetWidth = std::max(targetClient.right - targetClient.left, 1L);
    const int targetHeight = std::max(targetClient.bottom - targetClient.top, 1L);
    const graphics::ViewportScaler scaler({surface.width(), surface.height()});
    const graphics::Rect presentation =
        scaler.presentationRect({targetWidth, targetHeight}, resolutionScalePercent);

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

    HDC finalContext = CreateCompatibleDC(windowContext);
    if (finalContext == nullptr) {
        SelectObject(memoryContext, previousBitmap);
        DeleteObject(bitmap);
        DeleteDC(memoryContext);
        ReleaseDC(window.nativeHandle(), windowContext);
        throw std::runtime_error("failed to create final presentation device context");
    }

    BITMAPINFO finalBitmapInfo{};
    finalBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    finalBitmapInfo.bmiHeader.biWidth = targetWidth;
    finalBitmapInfo.bmiHeader.biHeight = -targetHeight;
    finalBitmapInfo.bmiHeader.biPlanes = 1;
    finalBitmapInfo.bmiHeader.biBitCount = 32;
    finalBitmapInfo.bmiHeader.biCompression = BI_RGB;

    void* finalBitmapBits = nullptr;
    HBITMAP finalBitmap = CreateDIBSection(windowContext,
                                           &finalBitmapInfo,
                                           DIB_RGB_COLORS,
                                           &finalBitmapBits,
                                           nullptr,
                                           0);
    if (finalBitmap == nullptr || finalBitmapBits == nullptr) {
        DeleteDC(finalContext);
        SelectObject(memoryContext, previousBitmap);
        DeleteObject(bitmap);
        DeleteDC(memoryContext);
        ReleaseDC(window.nativeHandle(), windowContext);
        throw std::runtime_error("failed to create final presentation DIB section");
    }

    const HGDIOBJ previousFinalBitmap = SelectObject(finalContext, finalBitmap);
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    RECT fillRect{0, 0, targetWidth, targetHeight};
    FillRect(finalContext, &fillRect, blackBrush);
    DeleteObject(blackBrush);

    SetStretchBltMode(finalContext, HALFTONE);
    const BOOL stretched = StretchBlt(finalContext,
                                      presentation.x,
                                      presentation.y,
                                      presentation.width,
                                      presentation.height,
                                      memoryContext,
                                      0,
                                      0,
                                      surface.width(),
                                      surface.height(),
                                      SRCCOPY);
    if (textSource != nullptr) {
        drawTextCommands(finalContext, *textSource, presentation, surface);
    }

    const BOOL copied = BitBlt(windowContext,
                               0,
                               0,
                               targetWidth,
                               targetHeight,
                               finalContext,
                               0,
                               0,
                               SRCCOPY);

    SelectObject(finalContext, previousFinalBitmap);
    DeleteObject(finalBitmap);
    DeleteDC(finalContext);
    SelectObject(memoryContext, previousBitmap);
    DeleteObject(bitmap);
    DeleteDC(memoryContext);
    ReleaseDC(window.nativeHandle(), windowContext);

    if (stretched == FALSE || copied == FALSE) {
        throw std::runtime_error("failed to present composited software surface");
    }
}

} // namespace

Win32SoftwarePresenter::Win32SoftwarePresenter(double openingSeconds)
    : openingGate_(openingSeconds) {}

void Win32SoftwarePresenter::present(const Win32Window& window,
                                     const graphics::SoftwareSurface& surface) const {
    presentWithEngineGate(window, surface, nullptr);
}

void Win32SoftwarePresenter::present(const Win32Window& window,
                                     const graphics::SoftwareSurface& surface,
                                     const graphics::RenderQueue& textSource) const {
    presentWithEngineGate(window, surface, &textSource);
}

bool Win32SoftwarePresenter::engineOpeningActive() const {
    return openingGate_.openingActive();
}

void Win32SoftwarePresenter::setResolutionScalePercent(int scalePercent) {
    resolutionScalePercent_ = std::clamp(scalePercent, 50, 200);
}

int Win32SoftwarePresenter::resolutionScalePercent() const {
    return resolutionScalePercent_;
}

void Win32SoftwarePresenter::presentWithEngineGate(const Win32Window& window,
                                                   const graphics::SoftwareSurface& surface,
                                                   const graphics::RenderQueue* textSource) const {
    graphics::RenderQueue callerQueue;
    if (textSource != nullptr) {
        callerQueue = *textSource;
    }

    graphics::RenderQueue presentedQueue;
    const bool opening = openingGate_.compose(presentedQueue,
                                              {surface.width(), surface.height()},
                                              1.0 / 60.0,
                                              callerQueue);
    if (opening) {
        graphics::SoftwareSurface openingSurface(surface.width(), surface.height());
        openingSurface.draw(presentedQueue, graphics::TextRasterization::Skip);
        presentComposited(window, openingSurface, &presentedQueue, resolutionScalePercent_);
        return;
    }

    presentComposited(window, surface, textSource, resolutionScalePercent_);
}

} // namespace haru::engine::platform::windows
