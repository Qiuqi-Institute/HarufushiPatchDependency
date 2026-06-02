#include "engine/platform/windows/Win32EncryptedRegistrySettings.hpp"

#include <HaruUserDataCipher>

#include <windows.h>

#include <cstddef>
#include <exception>
#include <string>
#include <utility>
#include <vector>

namespace haru::engine::platform::windows {

namespace {

std::wstring asciiToWide(const std::string& value) {
    std::wstring result;
    result.reserve(value.size());
    for (const char character : value) {
        result.push_back(static_cast<wchar_t>(static_cast<unsigned char>(character)));
    }
    return result;
}

std::vector<std::byte> toBytes(const std::string& value) {
    std::vector<std::byte> bytes;
    bytes.reserve(value.size());
    for (const char character : value) {
        bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(character)));
    }
    return bytes;
}

std::string fromBytes(const std::vector<std::byte>& bytes) {
    std::string value;
    value.reserve(bytes.size());
    for (const auto byte : bytes) {
        value.push_back(static_cast<char>(std::to_integer<unsigned char>(byte)));
    }
    return value;
}

std::string purposeFor(const std::string& name) {
    return "HarufushiPatchDependency.RegistrySettings." + name;
}

} // namespace

Win32EncryptedRegistrySettings::Win32EncryptedRegistrySettings(std::wstring subkey)
    : subkey_(std::move(subkey)) {}

bool Win32EncryptedRegistrySettings::setString(const std::string& name,
                                               const std::string& value) const {
    return setBytes(name, value);
}

std::string Win32EncryptedRegistrySettings::string(const std::string& name,
                                                   const std::string& fallback) const {
    return bytes(name, fallback);
}

bool Win32EncryptedRegistrySettings::setInt(const std::string& name, int value) const {
    return setBytes(name, std::to_string(value));
}

int Win32EncryptedRegistrySettings::integer(const std::string& name, int fallback) const {
    try {
        return std::stoi(bytes(name, std::to_string(fallback)));
    } catch (const std::exception&) {
        return fallback;
    }
}

bool Win32EncryptedRegistrySettings::setBool(const std::string& name, bool value) const {
    return setBytes(name, value ? "1" : "0");
}

bool Win32EncryptedRegistrySettings::boolean(const std::string& name, bool fallback) const {
    return bytes(name, fallback ? "1" : "0") == "1";
}

const std::wstring& Win32EncryptedRegistrySettings::subkey() const {
    return subkey_;
}

bool Win32EncryptedRegistrySettings::setBytes(const std::string& name,
                                              const std::string& value) const {
    HKEY key{};
    if (RegCreateKeyExW(HKEY_CURRENT_USER,
                        subkey_.c_str(),
                        0,
                        nullptr,
                        REG_OPTION_NON_VOLATILE,
                        KEY_SET_VALUE,
                        nullptr,
                        &key,
                        nullptr) != ERROR_SUCCESS) {
        return false;
    }

    const auto sealed = security::UserDataCipher::protect(toBytes(value), purposeFor(name));
    const std::wstring valueName = asciiToWide(name);
    const auto result = RegSetValueExW(key,
                                       valueName.c_str(),
                                       0,
                                       REG_BINARY,
                                       reinterpret_cast<const BYTE*>(sealed.data()),
                                       static_cast<DWORD>(sealed.size()));
    RegCloseKey(key);
    return result == ERROR_SUCCESS;
}

std::string Win32EncryptedRegistrySettings::bytes(const std::string& name,
                                                  const std::string& fallback) const {
    HKEY key{};
    if (RegOpenKeyExW(HKEY_CURRENT_USER, subkey_.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return fallback;
    }

    const std::wstring valueName = asciiToWide(name);
    DWORD type = 0;
    DWORD size = 0;
    if (RegQueryValueExW(key, valueName.c_str(), nullptr, &type, nullptr, &size) !=
            ERROR_SUCCESS ||
        type != REG_BINARY) {
        RegCloseKey(key);
        return fallback;
    }

    std::vector<std::byte> sealed(size);
    const auto query = RegQueryValueExW(key,
                                        valueName.c_str(),
                                        nullptr,
                                        &type,
                                        reinterpret_cast<BYTE*>(sealed.data()),
                                        &size);
    RegCloseKey(key);
    if (query != ERROR_SUCCESS) {
        return fallback;
    }

    try {
        return fromBytes(security::UserDataCipher::unprotect(sealed, purposeFor(name)));
    } catch (const std::exception&) {
        return fallback;
    }
}

} // namespace haru::engine::platform::windows
