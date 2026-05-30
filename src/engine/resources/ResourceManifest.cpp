#include "engine/resources/ResourceManifest.hpp"

#include <utility>

namespace haru::engine::resources {

ResourceManifest::ResourceManifest(int packageVersion) : packageVersion_(packageVersion) {}

bool ResourceManifest::add(ResourceRecord record) {
    if (find(record.id) != nullptr) {
        return false;
    }

    records_.push_back(std::move(record));
    return true;
}

const ResourceRecord* ResourceManifest::find(const ResourceId& id) const {
    for (const auto& record : records_) {
        if (record.id == id) {
            return &record;
        }
    }

    return nullptr;
}

int ResourceManifest::packageVersion() const {
    return packageVersion_;
}

const std::vector<ResourceRecord>& ResourceManifest::records() const {
    return records_;
}

} // namespace haru::engine::resources
