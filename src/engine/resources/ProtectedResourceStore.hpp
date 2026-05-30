#pragma once

#include "engine/resources/ResourceId.hpp"
#include "engine/resources/ResourceManifest.hpp"
#include "engine/security/ContentCipher.hpp"

#include <cstddef>
#include <functional>
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

class PresentedResourceView {
public:
    PresentedResourceView(const std::byte* data, std::size_t size);

    const std::byte* data() const;
    std::size_t size() const;
    bool empty() const;

private:
    const std::byte* data_;
    std::size_t size_;
};

class ProtectedResourceStore {
public:
    ProtectedResourceStore(ResourceManifest manifest,
                           const security::ContentCipher* cipher);

    void putSealed(const ResourceId& id, std::vector<std::byte> sealedBytes);
    ResourceReadError withPresentedResource(
        const ResourceId& id,
        const std::function<void(const PresentedResourceView&)>& present) const;

private:
    static void secureZero(std::vector<std::byte>& bytes);

    ResourceManifest manifest_;
    const security::ContentCipher* cipher_;
    std::unordered_map<std::string, std::vector<std::byte>> sealedPayloads_;
};

} // namespace haru::engine::resources
