#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace haru::engine::security {

struct CipherContext {
    std::string resourceId;
    int packageVersion;
};

class ContentCipher {
public:
    virtual ~ContentCipher() = default;

    virtual std::vector<std::byte> open(const CipherContext& context,
                                        const std::vector<std::byte>& sealedBytes) const = 0;
};

} // namespace haru::engine::security
