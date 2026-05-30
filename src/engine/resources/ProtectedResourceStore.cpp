#include "engine/resources/ProtectedResourceStore.hpp"

#include <algorithm>
#include <exception>
#include <utility>

namespace haru::engine::resources {

PresentedResourceView::PresentedResourceView(const std::byte* data, std::size_t size)
    : data_(data), size_(size) {}

const std::byte* PresentedResourceView::data() const {
    return data_;
}

std::size_t PresentedResourceView::size() const {
    return size_;
}

bool PresentedResourceView::empty() const {
    return size_ == 0;
}

ProtectedResourceStore::ProtectedResourceStore(ResourceManifest manifest,
                                               const security::ContentCipher* cipher)
    : manifest_(std::move(manifest)), cipher_(cipher) {}

void ProtectedResourceStore::putSealed(const ResourceId& id, std::vector<std::byte> sealedBytes) {
    sealedPayloads_[id.value()] = std::move(sealedBytes);
}

ResourceReadError ProtectedResourceStore::withPresentedResource(
    const ResourceId& id,
    const std::function<void(const PresentedResourceView&)>& present) const {
    const auto* record = manifest_.find(id);
    if (record == nullptr) {
        return ResourceReadError::NotFound;
    }

    const auto payload = sealedPayloads_.find(id.value());
    if (payload == sealedPayloads_.end()) {
        return ResourceReadError::MissingPayload;
    }

    std::vector<std::byte> plainBytes;

    if (!record->encrypted) {
        plainBytes = payload->second;
    } else {
        if (cipher_ == nullptr) {
            return ResourceReadError::MissingCipher;
        }

        try {
            plainBytes = cipher_->open({id.value(), manifest_.packageVersion()}, payload->second);
        } catch (const std::exception&) {
            return ResourceReadError::CipherFailed;
        }
    }

    present(PresentedResourceView(plainBytes.data(), plainBytes.size()));
    secureZero(plainBytes);
    return ResourceReadError::None;
}

void ProtectedResourceStore::secureZero(std::vector<std::byte>& bytes) {
    volatile std::byte* cursor = bytes.data();
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        cursor[index] = static_cast<std::byte>(0);
    }
}

} // namespace haru::engine::resources
