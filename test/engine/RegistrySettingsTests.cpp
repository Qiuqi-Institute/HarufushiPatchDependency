#include "support/TestHarness.hpp"

#ifdef _WIN32

#include <HaruWin32EncryptedRegistrySettings>

#include <windows.h>

#include <chrono>
#include <string>
#include <vector>

namespace {

std::wstring uniqueSubkey() {
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    return L"Software\\Qiuqi Institute\\Harufushi Patch Dependency\\Tests\\" +
           std::to_wstring(stamp);
}

std::vector<unsigned char> readRawValue(const std::wstring& subkey, const wchar_t* name) {
    HKEY key{};
    if (RegOpenKeyExW(HKEY_CURRENT_USER, subkey.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return {};
    }

    DWORD type = 0;
    DWORD size = 0;
    if (RegQueryValueExW(key, name, nullptr, &type, nullptr, &size) != ERROR_SUCCESS ||
        type != REG_BINARY) {
        RegCloseKey(key);
        return {};
    }

    std::vector<unsigned char> bytes(size);
    RegQueryValueExW(key, name, nullptr, &type, bytes.data(), &size);
    RegCloseKey(key);
    return bytes;
}

bool containsAscii(const std::vector<unsigned char>& bytes, const std::string& needle) {
    if (bytes.size() < needle.size()) {
        return false;
    }
    for (std::size_t index = 0; index + needle.size() <= bytes.size(); ++index) {
        bool match = true;
        for (std::size_t offset = 0; offset < needle.size(); ++offset) {
            if (bytes[index + offset] != static_cast<unsigned char>(needle[offset])) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}

} // namespace

HARU_TEST(win32_encrypted_registry_settings_store_values_as_non_plaintext_binary) {
    const std::wstring subkey = uniqueSubkey();
    haru::engine::platform::windows::Win32EncryptedRegistrySettings store(subkey);

    HARU_EXPECT_TRUE(store.setString("locale", "ja-JP"));
    HARU_EXPECT_EQ(store.string("locale", "en-US"), "ja-JP");

    const auto raw = readRawValue(subkey, L"locale");
    HARU_EXPECT_TRUE(!raw.empty());
    HARU_EXPECT_FALSE(containsAscii(raw, "ja-JP"));
    HARU_EXPECT_FALSE(containsAscii(raw, "locale"));

    RegDeleteTreeW(HKEY_CURRENT_USER, subkey.c_str());
}

#endif
