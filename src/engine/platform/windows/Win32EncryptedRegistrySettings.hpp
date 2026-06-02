#pragma once

#include <string>

namespace haru::engine::platform::windows {

class Win32EncryptedRegistrySettings {
public:
    explicit Win32EncryptedRegistrySettings(
        std::wstring subkey =
            L"Software\\Qiuqi Institute\\Harufushi Patch Dependency\\Settings");

    bool setString(const std::string& name, const std::string& value) const;
    std::string string(const std::string& name, const std::string& fallback) const;
    bool setInt(const std::string& name, int value) const;
    int integer(const std::string& name, int fallback) const;
    bool setBool(const std::string& name, bool value) const;
    bool boolean(const std::string& name, bool fallback) const;

    const std::wstring& subkey() const;

private:
    bool setBytes(const std::string& name, const std::string& value) const;
    std::string bytes(const std::string& name, const std::string& fallback) const;

    std::wstring subkey_;
};

} // namespace haru::engine::platform::windows
