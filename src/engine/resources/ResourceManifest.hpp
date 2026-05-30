#pragma once

#include "engine/resources/ResourceId.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace haru::engine::resources {

enum class ResourceKind {
    Image,
    Icon,
    Audio,
    Data,
    Localization,
};

struct ResourceRecord {
    ResourceId id;
    ResourceKind kind;
    std::string packagePath;
    std::size_t byteSize;
    bool encrypted;
    std::string locale;
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
