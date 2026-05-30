#pragma once

#include "engine/resources/ResourceId.hpp"
#include "engine/resources/ResourceManifest.hpp"
#include "engine/security/ContentCipher.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace haru::engine::resources {

enum class ResourceReadError {
    None,
    NotFound,
    MissingPayload,
    MissingCipher,
    CipherFailed,
};

class ResourceReadResult {
public:
    static ResourceReadResult success(std::vector<std::byte> bytes);
    static ResourceReadResult failure(ResourceReadError error);

    bool ok() const;
    const std::vector<std::byte>& bytes() const;
    ResourceReadError error() const;

private:
    ResourceReadResult(bool ok, std::vector<std::byte> bytes, ResourceReadError error);

    bool ok_;
    std::vector<std::byte> bytes_;
    ResourceReadError error_;
};

class ProtectedResourceStore {
public:
    ProtectedResourceStore(ResourceManifest manifest,
                           const security::ContentCipher* cipher);

    void putSealed(const ResourceId& id, std::vector<std::byte> sealedBytes);
    ResourceReadResult read(const ResourceId& id) const;

private:
    ResourceManifest manifest_;
    const security::ContentCipher* cipher_;
    std::unordered_map<std::string, std::vector<std::byte>> sealedPayloads_;
};

} // namespace haru::engine::resources
