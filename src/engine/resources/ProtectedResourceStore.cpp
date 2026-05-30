#include "engine/resources/ProtectedResourceStore.hpp"

#include <exception>
#include <utility>

namespace haru::engine::resources {

ResourceReadResult ResourceReadResult::success(std::vector<std::byte> bytes) {
    return ResourceReadResult(true, std::move(bytes), ResourceReadError::None);
}

ResourceReadResult ResourceReadResult::failure(ResourceReadError error) {
    return ResourceReadResult(false, {}, error);
}

bool ResourceReadResult::ok() const {
    return ok_;
}

const std::vector<std::byte>& ResourceReadResult::bytes() const {
    return bytes_;
}

ResourceReadError ResourceReadResult::error() const {
    return error_;
}

ResourceReadResult::ResourceReadResult(bool ok,
                                       std::vector<std::byte> bytes,
                                       ResourceReadError error)
    : ok_(ok), bytes_(std::move(bytes)), error_(error) {}

ProtectedResourceStore::ProtectedResourceStore(ResourceManifest manifest,
                                               const security::ContentCipher* cipher)
    : manifest_(std::move(manifest)), cipher_(cipher) {}

void ProtectedResourceStore::putSealed(const ResourceId& id, std::vector<std::byte> sealedBytes) {
    sealedPayloads_[id.value()] = std::move(sealedBytes);
}

ResourceReadResult ProtectedResourceStore::read(const ResourceId& id) const {
    const auto* record = manifest_.find(id);
    if (record == nullptr) {
        return ResourceReadResult::failure(ResourceReadError::NotFound);
    }

    const auto payload = sealedPayloads_.find(id.value());
    if (payload == sealedPayloads_.end()) {
        return ResourceReadResult::failure(ResourceReadError::MissingPayload);
    }

    if (!record->encrypted) {
        return ResourceReadResult::success(payload->second);
    }

    if (cipher_ == nullptr) {
        return ResourceReadResult::failure(ResourceReadError::MissingCipher);
    }

    try {
        return ResourceReadResult::success(cipher_->open(
            {id.value(), manifest_.packageVersion()}, payload->second));
    } catch (const std::exception&) {
        return ResourceReadResult::failure(ResourceReadError::CipherFailed);
    }
}

} // namespace haru::engine::resources
