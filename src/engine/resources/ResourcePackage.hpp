#pragma once

#include "ProtectedResourceStore.hpp"
#include "ResourceManifest.hpp"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace haru::engine::resources {

struct ResourcePackageInput {
    ResourceId id;
    ResourceKind kind;
    std::string logicalPath;
    std::string locale;
    std::vector<std::byte> bytes;
};

struct ResourcePackageBuildOptions {
    int packageVersion = 1;
    std::size_t maxPackageBytes = 128ULL * 1024ULL * 1024ULL;
    std::size_t maxFragmentPlainBytes = 4ULL * 1024ULL * 1024ULL;
    std::string masterKey;
};

struct ResourcePackageFile {
    std::string path;
    std::vector<std::byte> bytes;
};

struct ResourcePackageBundle {
    ResourceManifest manifest;
    std::vector<ResourcePackageFile> packages;
};

class ResourcePackageBuilder {
public:
    static ResourcePackageBundle build(const std::vector<ResourcePackageInput>& inputs,
                                       const ResourcePackageBuildOptions& options);
};

class ResourcePackageWriter {
public:
    static void write(const std::filesystem::path& directory,
                      const ResourcePackageBundle& bundle,
                      const std::string& masterKey);
};

class ResourcePackageReader {
public:
    static std::optional<ResourceManifest> readManifest(const std::filesystem::path& directory,
                                                        const std::string& masterKey);
};

class PackagedResourceStore {
public:
    using PackageFileReader = std::function<std::vector<std::byte>(const std::filesystem::path&)>;

    PackagedResourceStore(ResourceManifest manifest,
                          std::filesystem::path packageRoot,
                          std::string masterKey);
    PackagedResourceStore(ResourceManifest manifest,
                          std::filesystem::path packageRoot,
                          std::string masterKey,
                          PackageFileReader packageFileReader);

    ResourceReadError withPresentedResource(
        const ResourceId& id,
        const std::function<void(const PresentedResourceView&)>& present) const;

private:
    static void secureZero(std::vector<std::byte>& bytes);

    ResourceManifest manifest_;
    std::filesystem::path packageRoot_;
    std::string masterKey_;
    PackageFileReader packageFileReader_;
};

class PackagedResourceRuntime {
public:
    using ManifestReader = std::function<std::optional<ResourceManifest>(
        const std::filesystem::path&,
        const std::string&)>;

    static std::optional<PackagedResourceRuntime> open(std::filesystem::path packageRoot,
                                                       std::string masterKey);
    static std::optional<PackagedResourceRuntime> open(
        std::filesystem::path packageRoot,
        std::string masterKey,
        ManifestReader manifestReader,
        PackagedResourceStore::PackageFileReader packageFileReader);

    ResourceReadError withPresentedResource(
        const ResourceId& id,
        const std::function<void(const PresentedResourceView&)>& present) const;

private:
    explicit PackagedResourceRuntime(PackagedResourceStore store);

    PackagedResourceStore store_;
};

std::optional<std::string> readResourceMasterKeyFromEnvFile(
    const std::filesystem::path& envFile);

} // namespace haru::engine::resources
