#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace haru::engine::security {

class UserDataCipher {
public:
    static std::vector<std::byte> protect(const std::vector<std::byte>& plainBytes,
                                          const std::string& purpose);
    static std::vector<std::byte> unprotect(const std::vector<std::byte>& sealedBytes,
                                            const std::string& purpose);
};

} // namespace haru::engine::security
