#pragma once

#include "ResourceId.hpp"

#include <cstddef>
#include <string>
#include <cstdint>
#include <vector>

namespace haru::engine::resources {

enum class ResourceKind {
    Image,
    Icon,
    Audio,
    Data,
    Localization,
};

struct ResourceFragment {
    std::string packagePath;
    std::size_t offset = 0;
    std::size_t sealedSize = 0;
    std::size_t plainOffset = 0;
    std::size_t plainSize = 0;
    std::uint32_t keySlot = 0;
    std::vector<std::uint8_t> nonce;
    std::vector<std::uint8_t> tag;
};

struct ResourceRecord {
    ResourceId id;
    ResourceKind kind;
    std::string packagePath;
    std::size_t byteSize;
    bool encrypted;
    std::string locale;
    std::vector<ResourceFragment> fragments;
};

class ResourceManifest {
public:
    explicit ResourceManifest(int packageVersion);

    bool add(ResourceRecord record);
    const ResourceRecord* find(const ResourceId& id) const;
    int packageVersion() const;
    const std::vector<ResourceRecord>& records() const;

private:
    int packageVersion_;
    std::vector<ResourceRecord> records_;
};

} // namespace haru::engine::resources
