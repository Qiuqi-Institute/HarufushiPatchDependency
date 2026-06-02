#pragma once

#include <filesystem>

namespace haru::engine::platform {

class UserDirectories {
public:
    static std::filesystem::path documentsDirectory();
};

} // namespace haru::engine::platform
