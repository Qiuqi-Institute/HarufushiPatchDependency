#include "engine/platform/UserDirectories.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj_core.h>
#endif

namespace haru::engine::platform {

namespace {

#ifdef _WIN32
std::string wideToUtf8(const std::wstring& value) {
    if (value.empty()) {
        return {};
    }

    const int required = WideCharToMultiByte(CP_UTF8,
                                             0,
                                             value.data(),
                                             static_cast<int>(value.size()),
                                             nullptr,
                                             0,
                                             nullptr,
                                             nullptr);
    if (required <= 0) {
        throw std::runtime_error("failed to convert documents path to UTF-8");
    }

    std::string result(static_cast<std::size_t>(required), '\0');
    const int written = WideCharToMultiByte(CP_UTF8,
                                            0,
                                            value.data(),
                                            static_cast<int>(value.size()),
                                            result.data(),
                                            required,
                                            nullptr,
                                            nullptr);
    if (written != required) {
        throw std::runtime_error("failed to write UTF-8 documents path");
    }

    return result;
}
#endif

} // namespace

std::filesystem::path UserDirectories::documentsDirectory() {
#ifdef _WIN32
    PWSTR documentsPath = nullptr;
    const HRESULT result =
        SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &documentsPath);
    if (FAILED(result) || documentsPath == nullptr) {
        throw std::runtime_error("failed to resolve Windows Documents directory");
    }

    std::wstring widePath(documentsPath);
    CoTaskMemFree(documentsPath);
    return std::filesystem::u8path(wideToUtf8(widePath));
#else
    const char* home = std::getenv("HOME");
    if (home == nullptr || *home == '\0') {
        throw std::runtime_error("failed to resolve home directory");
    }
    return std::filesystem::path(home) / "Documents";
#endif
}

} // namespace haru::engine::platform
