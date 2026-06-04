#include "engine/platform/windows/Win32SoftwarePresenter.hpp"
#include "engine/graphics/ViewportScaler.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <wincodec.h>

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

bool hasImageCommands(const graphics::RenderQueue* queue) {
    if (queue == nullptr) {
        return false;
    }

    for (const auto& command : queue->commands()) {
        if (command.kind == graphics::DrawCommandKind::Image && !command.text.empty()) {
            return true;
        }
    }

    return false;
}

std::vector<std::uint32_t> toPremultipliedBgraPixels(
    const graphics::SoftwareSurface& surface) {
    std::vector<std::uint32_t> pixels;
    pixels.reserve(surface.pixels().size());

    for (const auto& color : surface.pixels()) {
        const std::uint32_t alpha = color.a;
        const std::uint32_t blue = (static_cast<std::uint32_t>(color.b) * alpha) / 255U;
        const std::uint32_t green = (static_cast<std::uint32_t>(color.g) * alpha) / 255U;
        const std::uint32_t red = (static_cast<std::uint32_t>(color.r) * alpha) / 255U;
        pixels.push_back(blue | (green << 8U) | (red << 16U) | (alpha << 24U));
    }

    return pixels;
}

template <typename T>
void releaseCom(T*& pointer) {
    if (pointer != nullptr) {
        pointer->Release();
        pointer = nullptr;
    }
}

class WicFactory {
public:
    WicFactory() {
        HRESULT result = CoCreateInstance(CLSID_WICImagingFactory,
                                          nullptr,
                                          CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARGS(&factory_));
        if (FAILED(result)) {
            throw std::runtime_error("failed to create WIC imaging factory");
        }
    }

    WicFactory(const WicFactory&) = delete;
    WicFactory& operator=(const WicFactory&) = delete;

    ~WicFactory() {
        releaseCom(factory_);
    }

    IWICImagingFactory* get() const {
        return factory_;
    }

private:
    IWICImagingFactory* factory_ = nullptr;
};

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

std::wstring resolveImagePath(const std::string& imagePath) {
    const std::wstring widePath = utf8ToWide(imagePath);
    if (GetFileAttributesW(widePath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        return widePath;
    }

    wchar_t modulePath[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return widePath;
    }

    std::wstring directory(modulePath, modulePath + length);
    const std::size_t slash = directory.find_last_of(L"\\/");
    if (slash != std::wstring::npos) {
        directory.resize(slash);
    }

    for (int depth = 0; depth < 4; ++depth) {
        const std::wstring candidate = directory + L"\\" + widePath;
        if (GetFileAttributesW(candidate.c_str()) != INVALID_FILE_ATTRIBUTES) {
            return candidate;
        }

        const std::size_t parentSlash = directory.find_last_of(L"\\/");
        if (parentSlash == std::wstring::npos) {
            break;
        }
        directory.resize(parentSlash);
    }

    return widePath;
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

void drawImageCommands(HDC deviceContext,
                       const graphics::RenderQueue& imageSource,
                       graphics::Rect presentationRect,
                       const graphics::SoftwareSurface& surface,
                       WicFactory& wicFactory) {
    for (const auto& command : imageSource.commands()) {
        if (command.kind != graphics::DrawCommandKind::Image || command.text.empty()) {
            continue;
        }

        IWICBitmapDecoder* decoder = nullptr;
        IWICBitmapFrameDecode* frame = nullptr;
        IWICFormatConverter* converter = nullptr;
        const std::wstring path = resolveImagePath(command.text);
        HRESULT result = wicFactory.get()->CreateDecoderFromFilename(path.c_str(),
                                                                      nullptr,
                                                                      GENERIC_READ,
                                                                      WICDecodeMetadataCacheOnLoad,
                                                                      &decoder);
        if (FAILED(result)) {
            releaseCom(converter);
            releaseCom(frame);
            releaseCom(decoder);
            continue;
        }

        result = decoder->GetFrame(0, &frame);
        if (FAILED(result)) {
            releaseCom(converter);
            releaseCom(frame);
            releaseCom(decoder);
            continue;
        }

        result = wicFactory.get()->CreateFormatConverter(&converter);
        if (FAILED(result)) {
            releaseCom(converter);
            releaseCom(frame);
            releaseCom(decoder);
            continue;
        }

        result = converter->Initialize(frame,
                                       GUID_WICPixelFormat32bppBGRA,
                                       WICBitmapDitherTypeNone,
                                       nullptr,
                                       0.0,
                                       WICBitmapPaletteTypeCustom);
        if (FAILED(result)) {
            releaseCom(converter);
            releaseCom(frame);
            releaseCom(decoder);
            continue;
        }

        UINT imageWidth = 0;
        UINT imageHeight = 0;
        result = converter->GetSize(&imageWidth, &imageHeight);
        if (FAILED(result) || imageWidth == 0 || imageHeight == 0) {
            releaseCom(converter);
            releaseCom(frame);
            releaseCom(decoder);
            continue;
        }

        std::vector<std::uint32_t> pixels(static_cast<std::size_t>(imageWidth) * imageHeight);
        result = converter->CopyPixels(nullptr,
                                       imageWidth * sizeof(std::uint32_t),
                                       static_cast<UINT>(pixels.size() *
                                                         sizeof(std::uint32_t)),
                                       reinterpret_cast<BYTE*>(pixels.data()));
        if (SUCCEEDED(result)) {
            BITMAPINFO imageInfo{};
            imageInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            imageInfo.bmiHeader.biWidth = static_cast<LONG>(imageWidth);
            imageInfo.bmiHeader.biHeight = -static_cast<LONG>(imageHeight);
            imageInfo.bmiHeader.biPlanes = 1;
            imageInfo.bmiHeader.biBitCount = 32;
            imageInfo.bmiHeader.biCompression = BI_RGB;
            const graphics::Rect targetRect =
                scaleRect(command.rect, presentationRect, surface);
            StretchDIBits(deviceContext,
                          targetRect.x,
                          targetRect.y,
                          targetRect.width,
                          targetRect.height,
                          0,
                          0,
                          static_cast<int>(imageWidth),
                          static_cast<int>(imageHeight),
                          pixels.data(),
                          &imageInfo,
                          DIB_RGB_COLORS,
                          SRCCOPY);
        }

        releaseCom(converter);
        releaseCom(frame);
        releaseCom(decoder);
    }
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
    const bool imageBackedFrame = hasImageCommands(textSource);
    const auto pixels = imageBackedFrame ? toPremultipliedBgraPixels(surface)
                                         : toBgraPixels(surface);
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
    WicFactory wicFactory;
    if (textSource != nullptr) {
        drawImageCommands(finalContext, *textSource, presentation, surface, wicFactory);
    }

    BOOL stretched = FALSE;
    if (imageBackedFrame) {
        BLENDFUNCTION blend{};
        blend.BlendOp = AC_SRC_OVER;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;
        stretched = AlphaBlend(finalContext,
                               presentation.x,
                               presentation.y,
                               presentation.width,
                               presentation.height,
                               memoryContext,
                               0,
                               0,
                               surface.width(),
                               surface.height(),
                               blend);
    } else {
        stretched = StretchBlt(finalContext,
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
    }
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
