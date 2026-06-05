#include "engine/platform/windows/Win32SoftwarePresenter.hpp"
#include "engine/graphics/ViewportScaler.hpp"
#include "engine/resources/ResourceId.hpp"
#include "engine/resources/ResourcePackage.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
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

std::filesystem::path executableDirectory() {
    wchar_t modulePath[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return std::filesystem::current_path();
    }

    std::filesystem::path path(std::wstring(modulePath, modulePath + length));
    return path.parent_path();
}

std::optional<std::filesystem::path> findEnvFile() {
    std::filesystem::path directory = executableDirectory();
    for (int depth = 0; depth < 5; ++depth) {
        const auto candidate = directory / ".env";
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
        if (!directory.has_parent_path() || directory == directory.parent_path()) {
            break;
        }
        directory = directory.parent_path();
    }

    const auto cwdCandidate = std::filesystem::current_path() / ".env";
    if (std::filesystem::exists(cwdCandidate)) {
        return cwdCandidate;
    }
    return std::nullopt;
}

std::filesystem::path packageRootDirectory() {
    return executableDirectory() / "resources";
}

std::shared_ptr<haru::engine::resources::PackagedResourceRuntime> packagedResourceRuntime() {
    static std::mutex mutex;
    static bool attempted = false;
    static std::shared_ptr<haru::engine::resources::PackagedResourceRuntime> runtime;

    std::lock_guard<std::mutex> lock(mutex);
    if (attempted) {
        return runtime;
    }
    attempted = true;

    const auto envFile = findEnvFile();
    if (!envFile.has_value()) {
        return nullptr;
    }
    const auto key = haru::engine::resources::readResourceMasterKeyFromEnvFile(*envFile);
    if (!key.has_value()) {
        return nullptr;
    }

    auto opened = haru::engine::resources::PackagedResourceRuntime::open(packageRootDirectory(),
                                                                         *key);
    if (!opened.has_value()) {
        return nullptr;
    }

    runtime =
        std::make_shared<haru::engine::resources::PackagedResourceRuntime>(std::move(*opened));
    return runtime;
}

std::optional<std::vector<std::byte>> loadPackagedResourceBytes(const std::string& resourceId) {
    const auto parsedId = haru::engine::resources::ResourceId::parse(resourceId);
    if (!parsedId.has_value()) {
        return std::nullopt;
    }

    const auto runtime = packagedResourceRuntime();
    if (runtime == nullptr) {
        return std::nullopt;
    }
    std::vector<std::byte> bytes;
    const auto result =
        runtime->withPresentedResource(*parsedId,
                                       [&](const haru::engine::resources::PresentedResourceView& view) {
                                           bytes.assign(view.data(), view.data() + view.size());
                                       });
    if (result != haru::engine::resources::ResourceReadError::None) {
        return std::nullopt;
    }
    return bytes;
}

std::string developmentPathForResourceId(const std::string& resourceId) {
    std::string path = "resources/";
    for (const char character : resourceId) {
        path.push_back(character == '.' ? '/' : character);
    }
    path += ".png";
    return path;
}

COLORREF toColorRef(graphics::Color color) {
    return RGB(color.r, color.g, color.b);
}

bool isSplashTitleLetter(const graphics::DrawCommand& command) {
    return command.text.size() == 1U && command.rect.height >= 60 &&
           command.color == graphics::Color{11, 119, 155, 255};
}

bool isHomeMenuText(const graphics::DrawCommand& command) {
    return command.rect.y >= 660 && command.rect.height <= 34 &&
           command.color != graphics::Color{11, 119, 155, 255};
}

std::wstring resolveResourcePath(const std::filesystem::path& relativePath) {
    std::filesystem::path directory = executableDirectory();
    for (int depth = 0; depth < 5; ++depth) {
        const auto candidate = directory / relativePath;
        if (std::filesystem::exists(candidate)) {
            return candidate.wstring();
        }
        if (!directory.has_parent_path() || directory == directory.parent_path()) {
            break;
        }
        directory = directory.parent_path();
    }

    const auto cwdCandidate = std::filesystem::current_path() / relativePath;
    if (std::filesystem::exists(cwdCandidate)) {
        return cwdCandidate.wstring();
    }
    return relativePath.wstring();
}

void loadPrivateFontFile(const std::filesystem::path& relativePath,
                         const std::string& packagedResourceId) {
    const std::wstring fontPath = resolveResourcePath(relativePath);
    if (GetFileAttributesW(fontPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        AddFontResourceExW(fontPath.c_str(), FR_PRIVATE, nullptr);
        return;
    }

    auto packagedBytes = loadPackagedResourceBytes(packagedResourceId);
    if (!packagedBytes.has_value() || packagedBytes->empty()) {
        return;
    }

    DWORD faceCount = 0;
    HANDLE fontHandle = AddFontMemResourceEx(
        reinterpret_cast<void*>(packagedBytes->data()),
        static_cast<DWORD>(packagedBytes->size()),
        nullptr,
        &faceCount);
    if (fontHandle == nullptr) {
        return;
    }

    static std::vector<std::vector<std::byte>> loadedFontBytes;
    static std::vector<HANDLE> loadedFontHandles;
    loadedFontBytes.push_back(std::move(*packagedBytes));
    loadedFontHandles.push_back(fontHandle);
}

void ensureZenMaruFontsLoaded() {
    static bool loaded = false;
    if (loaded) {
        return;
    }

    loadPrivateFontFile("resources/fonts/ZenMaruGothic-Black.ttf",
                        "fonts.ZenMaruGothic-Black");
    loadPrivateFontFile("resources/fonts/ZenMaruGothic-Bold.ttf",
                        "fonts.ZenMaruGothic-Bold");
    loadPrivateFontFile("resources/fonts/ResourceHanRoundedSC-Heavy.ttf",
                        "fonts.ResourceHanRoundedSC-Heavy");
    loadPrivateFontFile("resources/fonts/ResourceHanRoundedSC-Bold.ttf",
                        "fonts.ResourceHanRoundedSC-Bold");
    loaded = true;
}

bool usesZenMaruRole(const graphics::DrawCommand& command) {
    return command.textRole == graphics::TextRole::ZenMaruBlack ||
           command.textRole == graphics::TextRole::ZenMaruBold;
}

int fontSizeForCommand(const graphics::DrawCommand& command) {
    int baseFontSize = 20;
    if (command.textRole == graphics::TextRole::ZenMaruBlack) {
        baseFontSize = std::clamp(command.rect.height - 8, 22, 44);
    } else if (command.textRole == graphics::TextRole::ZenMaruBold) {
        baseFontSize = std::clamp(command.rect.height - 7, 18, 28);
    } else {
        const bool splashTitle = isSplashTitleLetter(command);
        const bool homeMenuText = isHomeMenuText(command);
        const bool displayText = !splashTitle && command.rect.height >= 56 &&
                                 command.rect.width <= 600;
        const bool sceneTitle = !splashTitle && !homeMenuText && !displayText &&
                                command.rect.height >= 40 &&
                                command.rect.width >= 520;
        baseFontSize =
            displayText ? 54 : (splashTitle ? 42 : (sceneTitle ? 30 : (homeMenuText ? 24 : 20)));
    }

    return std::max(1, (baseFontSize * std::clamp(command.fontScalePercent, 25, 200) + 50) /
                           100);
}

int fontWeightForCommand(const graphics::DrawCommand& command) {
    if (command.textRole == graphics::TextRole::ZenMaruBlack) {
        return FW_BLACK;
    }
    if (command.textRole == graphics::TextRole::ZenMaruBold) {
        return FW_BOLD;
    }

    const bool splashTitle = isSplashTitleLetter(command);
    const bool homeMenuText = isHomeMenuText(command);
    const bool displayText = !splashTitle && command.rect.height >= 56 &&
                             command.rect.width <= 600;
    const bool sceneTitle = !splashTitle && !homeMenuText && !displayText &&
                            command.rect.height >= 40 &&
                            command.rect.width >= 520;
    return homeMenuText ? FW_BLACK :
                          ((splashTitle || displayText || sceneTitle) ? FW_BOLD :
                                                                        FW_SEMIBOLD);
}

const wchar_t* fontFaceForCommand(const graphics::DrawCommand& command) {
    if (command.textRole == graphics::TextRole::ZenMaruBlack) {
        return L"Zen Maru Gothic Black";
    }
    if (command.textRole == graphics::TextRole::ZenMaruBold) {
        return L"Zen Maru Gothic";
    }

    const bool splashTitle = isSplashTitleLetter(command);
    const bool homeMenuText = isHomeMenuText(command);
    const bool displayText = !splashTitle && command.rect.height >= 56 &&
                             command.rect.width <= 600;
    const bool sceneTitle = !splashTitle && !homeMenuText && !displayText &&
                            command.rect.height >= 40 &&
                            command.rect.width >= 520;
    return homeMenuText ? L"Comic Sans MS" :
                          (displayText ? L"Bahnschrift" :
                                         (splashTitle ? L"Segoe Print" :
                                                        (sceneTitle ? L"Yu Mincho" :
                                                                      L"Yu Gothic UI")));
}

const wchar_t* fallbackFontFaceForCommand(const graphics::DrawCommand& command) {
    if (command.textRole == graphics::TextRole::ZenMaruBlack) {
        return L"Resource Han Rounded SC Heavy";
    }
    if (command.textRole == graphics::TextRole::ZenMaruBold) {
        return L"Resource Han Rounded SC";
    }
    return fontFaceForCommand(command);
}

int fallbackFontWeightForCommand(const graphics::DrawCommand& command) {
    if (command.textRole == graphics::TextRole::ZenMaruBlack) {
        return FW_HEAVY;
    }
    if (command.textRole == graphics::TextRole::ZenMaruBold) {
        return FW_BOLD;
    }
    return fontWeightForCommand(command);
}

graphics::Rect scaleRect(graphics::Rect rect,
                         graphics::Rect presentationRect,
                         const graphics::SoftwareSurface& surface) {
    return {presentationRect.x + (rect.x * presentationRect.width) / surface.width(),
            presentationRect.y + (rect.y * presentationRect.height) / surface.height(),
            std::max(1, (rect.width * presentationRect.width) / surface.width()),
            std::max(1, (rect.height * presentationRect.height) / surface.height())};
}

HFONT createFontForFace(const graphics::DrawCommand& command,
                        double scale,
                        const wchar_t* faceName,
                        int fontWeight) {
    if (usesZenMaruRole(command)) {
        ensureZenMaruFontsLoaded();
    }

    const int baseFontSize = fontSizeForCommand(command);
    const int fontSize =
        -std::max(1, static_cast<int>(std::round(static_cast<double>(baseFontSize) * scale)));
    return CreateFontW(fontSize,
                       0,
                       0,
                       0,
                       fontWeight,
                       FALSE,
                       FALSE,
                       FALSE,
                       DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS,
                       CLEARTYPE_QUALITY,
                       DEFAULT_PITCH | FF_DONTCARE,
                       faceName);
}

HFONT createFontForText(const graphics::DrawCommand& command, double scale) {
    return createFontForFace(command, scale, fontFaceForCommand(command),
                             fontWeightForCommand(command));
}

HFONT createFallbackFontForText(const graphics::DrawCommand& command, double scale) {
    return createFontForFace(command, scale, fallbackFontFaceForCommand(command),
                             fallbackFontWeightForCommand(command));
}

std::size_t codeUnitCountAt(const std::wstring& text, std::size_t index) {
    if (index + 1U < text.size() && text[index] >= 0xD800 && text[index] <= 0xDBFF &&
        text[index + 1U] >= 0xDC00 && text[index + 1U] <= 0xDFFF) {
        return 2U;
    }
    return 1U;
}

bool fontHasGlyph(HDC deviceContext, HFONT font, const std::wstring& text,
                  std::size_t index) {
    if (font == nullptr) {
        return false;
    }

    const HGDIOBJ previousFont = SelectObject(deviceContext, font);
    const std::size_t codeUnits = codeUnitCountAt(text, index);
    WORD glyphs[2]{0, 0};
    const DWORD result = GetGlyphIndicesW(deviceContext,
                                          text.data() + index,
                                          static_cast<int>(codeUnits),
                                          glyphs,
                                          GGI_MARK_NONEXISTING_GLYPHS);
    if (previousFont != nullptr) {
        SelectObject(deviceContext, previousFont);
    }
    if (result == GDI_ERROR) {
        return false;
    }
    for (std::size_t glyphIndex = 0; glyphIndex < codeUnits; ++glyphIndex) {
        if (glyphs[glyphIndex] == 0xFFFFU) {
            return false;
        }
    }
    return true;
}

struct TextRun {
    std::wstring text;
    bool fallback = false;
    int width = 0;
};

int textWidth(HDC deviceContext, HFONT font, const std::wstring& text) {
    if (font == nullptr || text.empty()) {
        return 0;
    }

    const HGDIOBJ previousFont = SelectObject(deviceContext, font);
    SIZE size{0, 0};
    GetTextExtentPoint32W(deviceContext,
                          text.c_str(),
                          static_cast<int>(text.size()),
                          &size);
    if (previousFont != nullptr) {
        SelectObject(deviceContext, previousFont);
    }
    return std::max(size.cx, 0L);
}

std::vector<TextRun> splitFallbackRuns(HDC deviceContext,
                                       HFONT primaryFont,
                                       HFONT fallbackFont,
                                       const std::wstring& text) {
    std::vector<TextRun> runs;
    std::size_t cursor = 0;
    while (cursor < text.size()) {
        const bool fallback = !fontHasGlyph(deviceContext, primaryFont, text, cursor);
        const std::size_t count = codeUnitCountAt(text, cursor);
        if (runs.empty() || runs.back().fallback != fallback) {
            runs.push_back({text.substr(cursor, count), fallback, 0});
        } else {
            runs.back().text += text.substr(cursor, count);
        }
        cursor += count;
    }

    for (auto& run : runs) {
        run.width = textWidth(deviceContext, run.fallback ? fallbackFont : primaryFont, run.text);
    }
    return runs;
}

std::wstring fallbackRunCacheKey(const graphics::DrawCommand& command,
                                 const std::wstring& text) {
    return std::to_wstring(static_cast<int>(command.textRole)) + L"|" +
           std::to_wstring(fontSizeForCommand(command)) + L"|" + text;
}

const std::vector<TextRun>& cachedFallbackRuns(HDC deviceContext,
                                               const graphics::DrawCommand& command,
                                               HFONT primaryFont,
                                               HFONT fallbackFont,
                                               const std::wstring& text) {
    static std::unordered_map<std::wstring, std::vector<TextRun>> cache;
    const std::wstring key = fallbackRunCacheKey(command, text);
    const auto found = cache.find(key);
    if (found != cache.end()) {
        return found->second;
    }

    auto inserted = cache.emplace(key,
                                  splitFallbackRuns(deviceContext,
                                                    primaryFont,
                                                    fallbackFont,
                                                    text));
    return inserted.first->second;
}

void drawFallbackText(HDC deviceContext,
                      const graphics::DrawCommand& command,
                      graphics::Rect scaledRect,
                      double scale,
                      const std::wstring& text) {
    HFONT primaryFont = createFontForText(command, scale);
    HFONT fallbackFont = createFallbackFontForText(command, scale);
    const auto& runs =
        cachedFallbackRuns(deviceContext, command, primaryFont, fallbackFont, text);

    int totalWidth = 0;
    for (const auto& run : runs) {
        totalWidth += run.width;
    }
    const int textX = scaledRect.x + std::max((scaledRect.width - totalWidth) / 2, 0);
    const int textY = scaledRect.y + std::max((scaledRect.height - fontSizeForCommand(command)) / 2,
                                             0);
    RECT clip{scaledRect.x,
              scaledRect.y,
              scaledRect.x + scaledRect.width,
              scaledRect.y + scaledRect.height};

    int cursorX = textX;
    for (const auto& run : runs) {
        HFONT font = run.fallback ? fallbackFont : primaryFont;
        const HGDIOBJ previousFont = font != nullptr ? SelectObject(deviceContext, font) : nullptr;
        ExtTextOutW(deviceContext,
                    cursorX,
                    textY,
                    ETO_CLIPPED,
                    &clip,
                    run.text.c_str(),
                    static_cast<UINT>(run.text.size()),
                    nullptr);
        if (previousFont != nullptr) {
            SelectObject(deviceContext, previousFont);
        }
        cursorX += run.width;
    }

    if (primaryFont != nullptr) {
        DeleteObject(primaryFont);
    }
    if (fallbackFont != nullptr) {
        DeleteObject(fallbackFont);
    }
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

        SetTextColor(deviceContext, toColorRef(command.color));
        RECT rect{scaledRect.x,
                  scaledRect.y,
                  scaledRect.x + scaledRect.width,
                  scaledRect.y + scaledRect.height};
        const std::wstring text = utf8ToWide(command.text);
        if (usesZenMaruRole(scaledCommand)) {
            drawFallbackText(deviceContext, scaledCommand, scaledRect, scale, text);
            continue;
        }

        HFONT font = createFontForText(scaledCommand, scale);
        const HGDIOBJ previousFont =
            font != nullptr ? SelectObject(deviceContext, font) : nullptr;
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

struct DecodedImage {
    UINT width = 0;
    UINT height = 0;
    std::vector<std::uint32_t> pixels;
};

std::optional<DecodedImage> decodeImageCommand(const graphics::DrawCommand& command,
                                               WicFactory& wicFactory) {
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    IWICStream* stream = nullptr;
    auto packagedBytes = loadPackagedResourceBytes(command.text);
    HRESULT result = E_FAIL;
    if (packagedBytes.has_value()) {
        result = wicFactory.get()->CreateStream(&stream);
        if (SUCCEEDED(result)) {
            result = stream->InitializeFromMemory(
                reinterpret_cast<BYTE*>(packagedBytes->data()),
                static_cast<DWORD>(packagedBytes->size()));
        }
        if (SUCCEEDED(result)) {
            result = wicFactory.get()->CreateDecoderFromStream(stream,
                                                               nullptr,
                                                               WICDecodeMetadataCacheOnLoad,
                                                               &decoder);
        }
    } else {
        const bool looksLikeResourceId =
            command.text.find('/') == std::string::npos &&
            command.text.find('\\') == std::string::npos;
        const std::wstring path =
            resolveImagePath(looksLikeResourceId ? developmentPathForResourceId(command.text)
                                                 : command.text);
        result = wicFactory.get()->CreateDecoderFromFilename(path.c_str(),
                                                              nullptr,
                                                              GENERIC_READ,
                                                              WICDecodeMetadataCacheOnLoad,
                                                              &decoder);
    }
    if (FAILED(result)) {
        releaseCom(stream);
        releaseCom(converter);
        releaseCom(frame);
        releaseCom(decoder);
        return std::nullopt;
    }

    result = decoder->GetFrame(0, &frame);
    if (FAILED(result)) {
        releaseCom(stream);
        releaseCom(converter);
        releaseCom(frame);
        releaseCom(decoder);
        return std::nullopt;
    }

    result = wicFactory.get()->CreateFormatConverter(&converter);
    if (FAILED(result)) {
        releaseCom(stream);
        releaseCom(converter);
        releaseCom(frame);
        releaseCom(decoder);
        return std::nullopt;
    }

    result = converter->Initialize(frame,
                                   GUID_WICPixelFormat32bppBGRA,
                                   WICBitmapDitherTypeNone,
                                   nullptr,
                                   0.0,
                                   WICBitmapPaletteTypeCustom);
    if (FAILED(result)) {
        releaseCom(stream);
        releaseCom(converter);
        releaseCom(frame);
        releaseCom(decoder);
        return std::nullopt;
    }

    DecodedImage image;
    result = converter->GetSize(&image.width, &image.height);
    if (FAILED(result) || image.width == 0 || image.height == 0) {
        releaseCom(stream);
        releaseCom(converter);
        releaseCom(frame);
        releaseCom(decoder);
        return std::nullopt;
    }

    image.pixels.resize(static_cast<std::size_t>(image.width) * image.height);
    result = converter->CopyPixels(nullptr,
                                   image.width * sizeof(std::uint32_t),
                                   static_cast<UINT>(image.pixels.size() *
                                                     sizeof(std::uint32_t)),
                                   reinterpret_cast<BYTE*>(image.pixels.data()));

    releaseCom(converter);
    releaseCom(frame);
    releaseCom(decoder);
    releaseCom(stream);

    if (FAILED(result)) {
        return std::nullopt;
    }
    return image;
}

const DecodedImage* cachedDecodedImage(const graphics::DrawCommand& command,
                                       WicFactory& wicFactory) {
    static std::unordered_map<std::string, DecodedImage> cache;
    const auto found = cache.find(command.text);
    if (found != cache.end()) {
        return &found->second;
    }

    auto decoded = decodeImageCommand(command, wicFactory);
    if (!decoded.has_value()) {
        return nullptr;
    }

    auto inserted = cache.emplace(command.text, std::move(*decoded));
    return &inserted.first->second;
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

        const DecodedImage* image = cachedDecodedImage(command, wicFactory);
        if (image == nullptr) {
            continue;
        }

        BITMAPINFO imageInfo{};
        imageInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        imageInfo.bmiHeader.biWidth = static_cast<LONG>(image->width);
        imageInfo.bmiHeader.biHeight = -static_cast<LONG>(image->height);
        imageInfo.bmiHeader.biPlanes = 1;
        imageInfo.bmiHeader.biBitCount = 32;
        imageInfo.bmiHeader.biCompression = BI_RGB;
        const graphics::Rect targetRect = scaleRect(command.rect, presentationRect, surface);
        StretchDIBits(deviceContext,
                      targetRect.x,
                      targetRect.y,
                      targetRect.width,
                      targetRect.height,
                      0,
                      0,
                      static_cast<int>(image->width),
                      static_cast<int>(image->height),
                      image->pixels.data(),
                      &imageInfo,
                      DIB_RGB_COLORS,
                      SRCCOPY);
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
