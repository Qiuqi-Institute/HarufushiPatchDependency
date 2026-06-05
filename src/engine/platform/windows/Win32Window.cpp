#include "engine/platform/windows/Win32Window.hpp"

#include <windowsx.h>

#include <stdexcept>
#include <string>

namespace haru::engine::platform::windows {

namespace {

const wchar_t* windowClassName() {
    return L"HarufushiPatchDependencyWindow";
}

constexpr int appIconResourceId = 101;

HICON loadIconResource(HINSTANCE instance, int width, int height) {
    return static_cast<HICON>(LoadImageW(instance,
                                         MAKEINTRESOURCEW(appIconResourceId),
                                         IMAGE_ICON,
                                         width,
                                         height,
                                         LR_DEFAULTCOLOR | LR_SHARED));
}

HICON loadDefaultIcon(int width, int height) {
    return static_cast<HICON>(LoadImageW(nullptr,
                                         MAKEINTRESOURCEW(32512),
                                         IMAGE_ICON,
                                         width,
                                         height,
                                         LR_SHARED));
}

std::wstring utf8ToWide(const std::string& text) {
    if (text.empty()) {
        return {};
    }

    const int required = MultiByteToWideChar(CP_UTF8, 0, text.data(),
                                             static_cast<int>(text.size()), nullptr, 0);
    if (required <= 0) {
        throw std::runtime_error("failed to convert window title to UTF-16");
    }

    std::wstring result(static_cast<std::size_t>(required), L'\0');
    const int written = MultiByteToWideChar(CP_UTF8, 0, text.data(),
                                            static_cast<int>(text.size()), result.data(),
                                            required);
    if (written != required) {
        throw std::runtime_error("failed to write UTF-16 window title");
    }

    return result;
}

ResizeEdge resizeEdgeFromNative(WPARAM edge) {
    switch (edge) {
    case WMSZ_LEFT:
        return ResizeEdge::Left;
    case WMSZ_RIGHT:
        return ResizeEdge::Right;
    case WMSZ_TOP:
        return ResizeEdge::Top;
    case WMSZ_BOTTOM:
        return ResizeEdge::Bottom;
    case WMSZ_TOPLEFT:
        return ResizeEdge::TopLeft;
    case WMSZ_TOPRIGHT:
        return ResizeEdge::TopRight;
    case WMSZ_BOTTOMLEFT:
        return ResizeEdge::BottomLeft;
    case WMSZ_BOTTOMRIGHT:
    default:
        return ResizeEdge::BottomRight;
    }
}

} // namespace

Win32Window::Win32Window(WindowConfig config) : config_(std::move(config)) {
    if (!config_.valid()) {
        throw std::invalid_argument("invalid window configuration");
    }

    const HINSTANCE instance = GetModuleHandleW(nullptr);
    ensureWindowClass(instance);

    RECT rect{0, 0, config_.width, config_.height};
    if (AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE) == FALSE) {
        throw std::runtime_error("failed to calculate native window bounds");
    }

    const std::wstring title = utf8ToWide(config_.title);
    handle_ = CreateWindowExW(0,
                              windowClassName(),
                              title.c_str(),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              rect.right - rect.left,
                              rect.bottom - rect.top,
                              nullptr,
                              nullptr,
                              instance,
                              this);

    if (handle_ == nullptr) {
        throw std::runtime_error("failed to create native window");
    }
}

Win32Window::~Win32Window() {
    if (handle_ != nullptr) {
        DestroyWindow(handle_);
        handle_ = nullptr;
    }
}

void Win32Window::show() {
    ShowWindow(handle_, SW_SHOW);
    UpdateWindow(handle_);
}

std::vector<WindowEvent> Win32Window::pollEvents() {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    auto events = pendingEvents_;
    pendingEvents_.clear();
    return events;
}

void Win32Window::requestClose() {
    if (!closeRequested_) {
        closeRequested_ = true;
        pendingEvents_.push_back(WindowEvent::closeRequested());
    }
}

bool Win32Window::shouldClose() const {
    return closeRequested_;
}

void Win32Window::setTitle(std::string title) {
    const std::wstring wideTitle = utf8ToWide(title);
    if (SetWindowTextW(handle_, wideTitle.c_str()) == FALSE) {
        throw std::runtime_error("failed to update native window title");
    }

    config_.title = std::move(title);
}

HWND Win32Window::nativeHandle() const {
    return handle_;
}

int Win32Window::clientWidth() const {
    RECT rect{};
    if (GetClientRect(handle_, &rect) == FALSE) {
        return config_.width;
    }
    return static_cast<int>(rect.right - rect.left);
}

int Win32Window::clientHeight() const {
    RECT rect{};
    if (GetClientRect(handle_, &rect) == FALSE) {
        return config_.height;
    }
    return static_cast<int>(rect.bottom - rect.top);
}

ATOM Win32Window::ensureWindowClass(HINSTANCE instance) {
    static ATOM registeredClass = 0;
    if (registeredClass != 0) {
        return registeredClass;
    }

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = 0;
    windowClass.lpfnWndProc = &Win32Window::staticWindowProc;
    windowClass.hInstance = instance;
    windowClass.hIcon = loadIconResource(instance,
                                         GetSystemMetrics(SM_CXICON),
                                         GetSystemMetrics(SM_CYICON));
    if (windowClass.hIcon == nullptr) {
        windowClass.hIcon = loadDefaultIcon(GetSystemMetrics(SM_CXICON),
                                            GetSystemMetrics(SM_CYICON));
    }
    windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    windowClass.hbrBackground = nullptr;
    windowClass.lpszClassName = windowClassName();
    windowClass.hIconSm = loadIconResource(instance,
                                           GetSystemMetrics(SM_CXSMICON),
                                           GetSystemMetrics(SM_CYSMICON));
    if (windowClass.hIconSm == nullptr) {
        windowClass.hIconSm = loadDefaultIcon(GetSystemMetrics(SM_CXSMICON),
                                              GetSystemMetrics(SM_CYSMICON));
    }

    registeredClass = RegisterClassExW(&windowClass);
    if (registeredClass == 0) {
        throw std::runtime_error("failed to register native window class");
    }

    return registeredClass;
}

LRESULT CALLBACK Win32Window::staticWindowProc(HWND handle,
                                               UINT message,
                                               WPARAM wordParam,
                                               LPARAM longParam) {
    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<CREATESTRUCTW*>(longParam);
        auto* window = static_cast<Win32Window*>(create->lpCreateParams);
        SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        window->handle_ = handle;
    }

    auto* window = reinterpret_cast<Win32Window*>(
        GetWindowLongPtrW(handle, GWLP_USERDATA));

    if (window != nullptr) {
        return window->handleMessage(message, wordParam, longParam);
    }

    return DefWindowProcW(handle, message, wordParam, longParam);
}

LRESULT Win32Window::handleMessage(UINT message, WPARAM wordParam, LPARAM longParam) {
    switch (message) {
    case WM_CLOSE:
        requestClose();
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        BeginPaint(handle_, &paint);
        EndPaint(handle_, &paint);
        return 0;
    }
    case WM_LBUTTONUP:
        pendingEvents_.push_back(WindowEvent::mouseButtonReleased(
            static_cast<int>(GET_X_LPARAM(longParam)),
            static_cast<int>(GET_Y_LPARAM(longParam)),
            MouseButton::Left));
        return 0;
    case WM_SIZING: {
        auto* rect = reinterpret_cast<RECT*>(longParam);
        const auto constrained = WindowSizingPolicy::constrainAspectRatio(
            {rect->left, rect->top, rect->right, rect->bottom},
            resizeEdgeFromNative(wordParam),
            config_.width,
            config_.height);
        rect->left = constrained.left;
        rect->top = constrained.top;
        rect->right = constrained.right;
        rect->bottom = constrained.bottom;
        return TRUE;
    }
    case WM_SIZE:
        pendingEvents_.push_back(WindowEvent::resized(static_cast<int>(LOWORD(longParam)),
                                                      static_cast<int>(HIWORD(longParam))));
        return 0;
    default:
        return DefWindowProcW(handle_, message, wordParam, longParam);
    }
}

} // namespace haru::engine::platform::windows
