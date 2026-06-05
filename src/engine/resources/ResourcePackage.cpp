#include "engine/resources/ResourcePackage.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace haru::engine::resources {

namespace {

constexpr std::array<std::byte, 8> indexMagic{
    static_cast<std::byte>(0x8F),
    static_cast<std::byte>(0x48),
    static_cast<std::byte>(0xA7),
    static_cast<std::byte>(0x21),
    static_cast<std::byte>(0x63),
    static_cast<std::byte>(0xD2),
    static_cast<std::byte>(0x0E),
    static_cast<std::byte>(0x91),
};
constexpr const char* indexFileName = "index.harupack";

struct SealContext {
    std::string resourceId;
    int packageVersion = 0;
    std::size_t fragmentIndex = 0;
    std::uint32_t keySlot = 0;
    std::vector<std::uint8_t> nonce;
    std::vector<std::uint8_t> tag;
};

struct SealedFragment {
    std::vector<std::byte> bytes;
    SealContext context;
};

void pushU8(std::vector<std::byte>& bytes, std::uint8_t value) {
    bytes.push_back(static_cast<std::byte>(value));
}

void pushU32(std::vector<std::byte>& bytes, std::uint32_t value) {
    for (int shift = 0; shift < 32; shift += 8) {
        pushU8(bytes, static_cast<std::uint8_t>((value >> shift) & 0xFFU));
    }
}

void pushU64(std::vector<std::byte>& bytes, std::uint64_t value) {
    for (int shift = 0; shift < 64; shift += 8) {
        pushU8(bytes, static_cast<std::uint8_t>((value >> shift) & 0xFFU));
    }
}

std::uint8_t readU8(const std::vector<std::byte>& bytes, std::size_t& cursor) {
    if (cursor >= bytes.size()) {
        throw std::runtime_error("truncated resource package bytes");
    }
    return std::to_integer<std::uint8_t>(bytes[cursor++]);
}

std::uint32_t readU32(const std::vector<std::byte>& bytes, std::size_t& cursor) {
    std::uint32_t value = 0;
    for (int shift = 0; shift < 32; shift += 8) {
        value |= static_cast<std::uint32_t>(readU8(bytes, cursor)) << shift;
    }
    return value;
}

std::uint64_t readU64(const std::vector<std::byte>& bytes, std::size_t& cursor) {
    std::uint64_t value = 0;
    for (int shift = 0; shift < 64; shift += 8) {
        value |= static_cast<std::uint64_t>(readU8(bytes, cursor)) << shift;
    }
    return value;
}

void pushString(std::vector<std::byte>& bytes, const std::string& text) {
    pushU32(bytes, static_cast<std::uint32_t>(text.size()));
    for (const char character : text) {
        pushU8(bytes, static_cast<std::uint8_t>(static_cast<unsigned char>(character)));
    }
}

std::string readString(const std::vector<std::byte>& bytes, std::size_t& cursor) {
    const auto size = readU32(bytes, cursor);
    if (cursor + size > bytes.size()) {
        throw std::runtime_error("truncated resource package string");
    }
    std::string text;
    text.reserve(size);
    for (std::uint32_t index = 0; index < size; ++index) {
        text.push_back(static_cast<char>(readU8(bytes, cursor)));
    }
    return text;
}

void pushByteVector(std::vector<std::byte>& bytes, const std::vector<std::uint8_t>& values) {
    pushU32(bytes, static_cast<std::uint32_t>(values.size()));
    for (const auto value : values) {
        pushU8(bytes, value);
    }
}

std::vector<std::uint8_t> readByteVector(const std::vector<std::byte>& bytes,
                                         std::size_t& cursor) {
    const auto size = readU32(bytes, cursor);
    std::vector<std::uint8_t> values;
    values.reserve(size);
    for (std::uint32_t index = 0; index < size; ++index) {
        values.push_back(readU8(bytes, cursor));
    }
    return values;
}

std::uint64_t fnv1aBytes(const std::vector<std::byte>& bytes, std::uint64_t seed) {
    std::uint64_t hash = seed;
    for (const auto byte : bytes) {
        hash ^= std::to_integer<std::uint8_t>(byte);
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::uint64_t fnv1aText(const std::string& text, std::uint64_t seed) {
    std::uint64_t hash = seed;
    for (const char character : text) {
        hash ^= static_cast<std::uint8_t>(static_cast<unsigned char>(character));
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::uint64_t splitmix64(std::uint64_t& state) {
    std::uint64_t result = (state += 0x9E3779B97F4A7C15ULL);
    result = (result ^ (result >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    result = (result ^ (result >> 27U)) * 0x94D049BB133111EBULL;
    return result ^ (result >> 31U);
}

std::uint64_t contextSeed(const std::string& masterKey,
                          const SealContext& context,
                          std::uint64_t layer) {
    std::uint64_t seed = fnv1aText(masterKey, 1469598103934665603ULL ^ layer);
    seed = fnv1aText(context.resourceId, seed);
    seed ^= static_cast<std::uint64_t>(context.packageVersion) * 0xD6E8FEB86659FD93ULL;
    seed ^= static_cast<std::uint64_t>(context.fragmentIndex + 1U) * 0xA5A3564E27F8867FULL;
    seed ^= static_cast<std::uint64_t>(context.keySlot + 3U) * 0xC2B2AE3D27D4EB4FULL;
    for (const auto value : context.nonce) {
        seed ^= value;
        seed *= 1099511628211ULL;
    }
    return seed;
}

std::vector<std::uint8_t> deriveNonce(const std::string& masterKey,
                                      const std::string& resourceId,
                                      int packageVersion,
                                      std::size_t fragmentIndex) {
    SealContext context;
    context.resourceId = resourceId;
    context.packageVersion = packageVersion;
    context.fragmentIndex = fragmentIndex;
    context.keySlot = static_cast<std::uint32_t>(
        contextSeed(masterKey, context, 0xD00DFEEDULL) & 0x03U);

    std::uint64_t state = contextSeed(masterKey, context, 0xBADC0FFEEULL);
    std::vector<std::uint8_t> nonce;
    nonce.reserve(12);
    for (int index = 0; index < 12; ++index) {
        if ((index % 8) == 0) {
            splitmix64(state);
        }
        nonce.push_back(static_cast<std::uint8_t>((state >> ((index % 8) * 8)) & 0xFFU));
    }
    return nonce;
}

void xorWithStream(std::vector<std::byte>& bytes, std::uint64_t seed) {
    std::uint64_t state = seed;
    std::uint64_t lane = 0;
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        if ((index % 8U) == 0U) {
            lane = splitmix64(state);
        }
        bytes[index] = static_cast<std::byte>(
            std::to_integer<std::uint8_t>(bytes[index]) ^
            static_cast<std::uint8_t>((lane >> ((index % 8U) * 8U)) & 0xFFU));
    }
}

std::vector<std::size_t> permutationSwaps(std::size_t size, std::uint64_t seed) {
    std::vector<std::size_t> swaps(size, 0);
    std::uint64_t state = seed;
    for (std::size_t index = size; index > 1U; --index) {
        swaps[index - 1U] = static_cast<std::size_t>(splitmix64(state) % index);
    }
    return swaps;
}

void permute(std::vector<std::byte>& bytes, std::uint64_t seed, bool reverse) {
    const auto swaps = permutationSwaps(bytes.size(), seed);
    if (!reverse) {
        for (std::size_t index = bytes.size(); index > 1U; --index) {
            std::swap(bytes[index - 1U], bytes[swaps[index - 1U]]);
        }
        return;
    }

    for (std::size_t index = 2U; index <= bytes.size(); ++index) {
        std::swap(bytes[index - 1U], bytes[swaps[index - 1U]]);
    }
}

std::vector<std::uint8_t> authenticationTag(const std::vector<std::byte>& sealedBytes,
                                            const std::string& masterKey,
                                            const SealContext& context) {
    std::vector<std::byte> material = sealedBytes;
    for (const auto value : context.nonce) {
        material.push_back(static_cast<std::byte>(value));
    }
    const std::uint64_t h1 = fnv1aBytes(material, contextSeed(masterKey, context, 0xA17A6ULL));
    const std::uint64_t h2 =
        fnv1aBytes(material, contextSeed(masterKey, context, 0x71E57A6ULL) ^ h1);

    std::vector<std::uint8_t> tag;
    tag.reserve(16);
    for (int shift = 0; shift < 64; shift += 8) {
        tag.push_back(static_cast<std::uint8_t>((h1 >> shift) & 0xFFU));
    }
    for (int shift = 0; shift < 64; shift += 8) {
        tag.push_back(static_cast<std::uint8_t>((h2 >> shift) & 0xFFU));
    }
    return tag;
}

SealedFragment sealFragment(const std::vector<std::byte>& plainBytes,
                            const std::string& masterKey,
                            SealContext context) {
    context.nonce =
        deriveNonce(masterKey, context.resourceId, context.packageVersion, context.fragmentIndex);
    context.keySlot = static_cast<std::uint32_t>(
        contextSeed(masterKey, context, 0x517A9EULL) & 0x03U);

    std::vector<std::byte> sealed = plainBytes;
    xorWithStream(sealed, contextSeed(masterKey, context, 0x1111111111111111ULL));
    permute(sealed, contextSeed(masterKey, context, 0x2222222222222222ULL), false);
    xorWithStream(sealed, contextSeed(masterKey, context, 0x3333333333333333ULL));
    context.tag = authenticationTag(sealed, masterKey, context);
    return {std::move(sealed), std::move(context)};
}

std::vector<std::byte> openFragment(const std::vector<std::byte>& sealedBytes,
                                    const std::string& masterKey,
                                    const SealContext& context) {
    if (authenticationTag(sealedBytes, masterKey, context) != context.tag) {
        throw std::runtime_error("resource package authentication failed");
    }

    std::vector<std::byte> plain = sealedBytes;
    xorWithStream(plain, contextSeed(masterKey, context, 0x3333333333333333ULL));
    permute(plain, contextSeed(masterKey, context, 0x2222222222222222ULL), true);
    xorWithStream(plain, contextSeed(masterKey, context, 0x1111111111111111ULL));
    return plain;
}

std::string packagePathFor(std::size_t index) {
    std::string number = std::to_string(index);
    while (number.size() < 3U) {
        number.insert(number.begin(), '0');
    }
    return "res_" + number + ".harupack";
}

std::vector<std::byte> serializeManifest(const ResourceManifest& manifest) {
    std::vector<std::byte> bytes;
    pushU32(bytes, static_cast<std::uint32_t>(manifest.packageVersion()));
    pushU32(bytes, static_cast<std::uint32_t>(manifest.records().size()));
    for (const auto& record : manifest.records()) {
        pushString(bytes, record.id.value());
        pushU32(bytes, static_cast<std::uint32_t>(record.kind));
        pushString(bytes, record.packagePath);
        pushU64(bytes, static_cast<std::uint64_t>(record.byteSize));
        pushU8(bytes, record.encrypted ? 1U : 0U);
        pushString(bytes, record.locale);
        pushU32(bytes, static_cast<std::uint32_t>(record.fragments.size()));
        for (const auto& fragment : record.fragments) {
            pushString(bytes, fragment.packagePath);
            pushU64(bytes, static_cast<std::uint64_t>(fragment.offset));
            pushU64(bytes, static_cast<std::uint64_t>(fragment.sealedSize));
            pushU64(bytes, static_cast<std::uint64_t>(fragment.plainOffset));
            pushU64(bytes, static_cast<std::uint64_t>(fragment.plainSize));
            pushU32(bytes, fragment.keySlot);
            pushByteVector(bytes, fragment.nonce);
            pushByteVector(bytes, fragment.tag);
        }
    }
    return bytes;
}

ResourceManifest deserializeManifest(const std::vector<std::byte>& bytes) {
    std::size_t cursor = 0;
    ResourceManifest manifest(static_cast<int>(readU32(bytes, cursor)));
    const auto recordCount = readU32(bytes, cursor);
    for (std::uint32_t index = 0; index < recordCount; ++index) {
        const auto id = ResourceId::parse(readString(bytes, cursor));
        if (!id.has_value()) {
            throw std::runtime_error("resource package manifest contains invalid id");
        }
        ResourceRecord record{*id,
                              static_cast<ResourceKind>(readU32(bytes, cursor)),
                              readString(bytes, cursor),
                              static_cast<std::size_t>(readU64(bytes, cursor)),
                              readU8(bytes, cursor) != 0,
                              readString(bytes, cursor)};
        const auto fragmentCount = readU32(bytes, cursor);
        for (std::uint32_t fragmentIndex = 0; fragmentIndex < fragmentCount; ++fragmentIndex) {
            ResourceFragment fragment;
            fragment.packagePath = readString(bytes, cursor);
            fragment.offset = static_cast<std::size_t>(readU64(bytes, cursor));
            fragment.sealedSize = static_cast<std::size_t>(readU64(bytes, cursor));
            fragment.plainOffset = static_cast<std::size_t>(readU64(bytes, cursor));
            fragment.plainSize = static_cast<std::size_t>(readU64(bytes, cursor));
            fragment.keySlot = readU32(bytes, cursor);
            fragment.nonce = readByteVector(bytes, cursor);
            fragment.tag = readByteVector(bytes, cursor);
            record.fragments.push_back(std::move(fragment));
        }
        manifest.add(std::move(record));
    }
    return manifest;
}

std::vector<std::byte> readFileBytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to open resource package file");
    }
    std::vector<char> raw((std::istreambuf_iterator<char>(input)),
                          std::istreambuf_iterator<char>());
    std::vector<std::byte> bytes;
    bytes.reserve(raw.size());
    for (const char byte : raw) {
        bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(byte)));
    }
    return bytes;
}

void writeFileBytes(const std::filesystem::path& path, const std::vector<std::byte>& bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("failed to write resource package file");
    }
    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
}

} // namespace

ResourcePackageBundle ResourcePackageBuilder::build(
    const std::vector<ResourcePackageInput>& inputs,
    const ResourcePackageBuildOptions& options) {
    if (options.masterKey.size() < 32U) {
        throw std::invalid_argument("resource package master key is too short");
    }
    if (options.maxPackageBytes == 0U || options.maxFragmentPlainBytes == 0U) {
        throw std::invalid_argument("resource package size limits must be positive");
    }

    ResourcePackageBundle bundle{ResourceManifest(options.packageVersion), {}};
    bundle.packages.push_back({packagePathFor(0), {}});
    struct PendingFragment {
        std::size_t recordIndex = 0;
        std::size_t fragmentIndex = 0;
        ResourceFragment fragment;
        std::vector<std::byte> sealedBytes;
        std::uint64_t shuffleKey = 0;
    };

    std::vector<ResourceRecord> records;
    std::vector<PendingFragment> pending;
    for (const auto& input : inputs) {
        ResourceRecord record{input.id,
                              input.kind,
                              std::string{},
                              input.bytes.size(),
                              true,
                              input.locale};
        const std::size_t recordIndex = records.size();
        std::size_t cursor = 0;
        while (cursor < input.bytes.size() || (input.bytes.empty() && cursor == 0U)) {
            const std::size_t plainSize =
                input.bytes.empty()
                    ? 0U
                    : std::min(options.maxFragmentPlainBytes, input.bytes.size() - cursor);
            std::vector<std::byte> plain(input.bytes.begin() + static_cast<std::ptrdiff_t>(cursor),
                                         input.bytes.begin() +
                                             static_cast<std::ptrdiff_t>(cursor + plainSize));

            SealContext context;
            context.resourceId = input.id.value();
            context.packageVersion = options.packageVersion;
            context.fragmentIndex = record.fragments.size();
            auto sealed = sealFragment(plain, options.masterKey, std::move(context));

            ResourceFragment fragment;
            fragment.sealedSize = sealed.bytes.size();
            fragment.plainOffset = cursor;
            fragment.plainSize = plainSize;
            fragment.keySlot = sealed.context.keySlot;
            fragment.nonce = sealed.context.nonce;
            fragment.tag = sealed.context.tag;
            record.fragments.push_back(std::move(fragment));
            pending.push_back({recordIndex,
                               record.fragments.size() - 1U,
                               record.fragments.back(),
                               std::move(sealed.bytes),
                               contextSeed(options.masterKey, sealed.context, 0x5A17EDULL)});

            if (input.bytes.empty()) {
                break;
            }
            cursor += plainSize;
        }

        records.push_back(std::move(record));
    }

    std::sort(pending.begin(), pending.end(), [](const auto& left, const auto& right) {
        return left.shuffleKey < right.shuffleKey;
    });

    for (auto& fragment : pending) {
        if (bundle.packages.back().bytes.size() + fragment.sealedBytes.size() >
                options.maxPackageBytes &&
            !bundle.packages.back().bytes.empty()) {
            bundle.packages.push_back({packagePathFor(bundle.packages.size()), {}});
        }

        auto& package = bundle.packages.back();
        fragment.fragment.packagePath = package.path;
        fragment.fragment.offset = package.bytes.size();
        package.bytes.insert(package.bytes.end(),
                             fragment.sealedBytes.begin(),
                             fragment.sealedBytes.end());

        auto& record = records[fragment.recordIndex];
        record.fragments[fragment.fragmentIndex].packagePath = fragment.fragment.packagePath;
        record.fragments[fragment.fragmentIndex].offset = fragment.fragment.offset;
    }

    for (auto& record : records) {
        if (!record.fragments.empty()) {
            record.packagePath = record.fragments.front().packagePath;
        }
        bundle.manifest.add(std::move(record));
    }
    return bundle;
}

void ResourcePackageWriter::write(const std::filesystem::path& directory,
                                  const ResourcePackageBundle& bundle,
                                  const std::string& masterKey) {
    std::filesystem::create_directories(directory);
    for (const auto& package : bundle.packages) {
        writeFileBytes(directory / package.path, package.bytes);
    }

    SealContext context;
    context.resourceId = "harufushi.package.index";
    context.packageVersion = bundle.manifest.packageVersion();
    context.fragmentIndex = 0;
    auto sealed = sealFragment(serializeManifest(bundle.manifest), masterKey, std::move(context));

    std::vector<std::byte> indexBytes(indexMagic.begin(), indexMagic.end());
    pushU32(indexBytes, static_cast<std::uint32_t>(bundle.manifest.packageVersion()));
    pushU64(indexBytes, static_cast<std::uint64_t>(sealed.bytes.size()));
    pushU32(indexBytes, sealed.context.keySlot);
    pushByteVector(indexBytes, sealed.context.nonce);
    pushByteVector(indexBytes, sealed.context.tag);
    indexBytes.insert(indexBytes.end(), sealed.bytes.begin(), sealed.bytes.end());
    writeFileBytes(directory / indexFileName, indexBytes);
}

std::optional<ResourceManifest> ResourcePackageReader::readManifest(
    const std::filesystem::path& directory,
    const std::string& masterKey) {
    try {
        const auto bytes = readFileBytes(directory / indexFileName);
        std::size_t cursor = 0;
        for (const auto expected : indexMagic) {
            if (readU8(bytes, cursor) != std::to_integer<std::uint8_t>(expected)) {
                return std::nullopt;
            }
        }

        SealContext context;
        context.resourceId = "harufushi.package.index";
        context.packageVersion = static_cast<int>(readU32(bytes, cursor));
        context.fragmentIndex = 0;
        const auto sealedSize = static_cast<std::size_t>(readU64(bytes, cursor));
        context.keySlot = readU32(bytes, cursor);
        context.nonce = readByteVector(bytes, cursor);
        context.tag = readByteVector(bytes, cursor);
        if (cursor + sealedSize > bytes.size()) {
            return std::nullopt;
        }
        std::vector<std::byte> sealed(bytes.begin() + static_cast<std::ptrdiff_t>(cursor),
                                      bytes.begin() +
                                          static_cast<std::ptrdiff_t>(cursor + sealedSize));
        return deserializeManifest(openFragment(sealed, masterKey, context));
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

PackagedResourceStore::PackagedResourceStore(ResourceManifest manifest,
                                             std::filesystem::path packageRoot,
                                             std::string masterKey)
    : PackagedResourceStore(std::move(manifest),
                            std::move(packageRoot),
                            std::move(masterKey),
                            readFileBytes) {}

PackagedResourceStore::PackagedResourceStore(ResourceManifest manifest,
                                             std::filesystem::path packageRoot,
                                             std::string masterKey,
                                             PackageFileReader packageFileReader)
    : manifest_(std::move(manifest)),
      packageRoot_(std::move(packageRoot)),
      masterKey_(std::move(masterKey)),
      packageFileReader_(std::move(packageFileReader)) {
    if (!packageFileReader_) {
        throw std::invalid_argument("resource package file reader is required");
    }
}

ResourceReadError PackagedResourceStore::withPresentedResource(
    const ResourceId& id,
    const std::function<void(const PresentedResourceView&)>& present) const {
    const auto* record = manifest_.find(id);
    if (record == nullptr) {
        return ResourceReadError::NotFound;
    }
    if (record->fragments.empty()) {
        return ResourceReadError::MissingPayload;
    }

    std::vector<std::byte> plain(record->byteSize);
    std::unordered_map<std::string, std::vector<std::byte>> packageCache;
    try {
        for (std::size_t index = 0; index < record->fragments.size(); ++index) {
            const auto& fragment = record->fragments[index];
            auto packageIt = packageCache.find(fragment.packagePath);
            if (packageIt == packageCache.end()) {
                auto packageBytes = packageFileReader_(packageRoot_ / fragment.packagePath);
                packageIt = packageCache.emplace(fragment.packagePath, std::move(packageBytes)).first;
            }
            const auto& packageBytes = packageIt->second;
            if (fragment.offset + fragment.sealedSize > packageBytes.size() ||
                fragment.plainOffset + fragment.plainSize > plain.size()) {
                secureZero(plain);
                return ResourceReadError::MissingPayload;
            }

            std::vector<std::byte> sealed(
                packageBytes.begin() + static_cast<std::ptrdiff_t>(fragment.offset),
                packageBytes.begin() +
                    static_cast<std::ptrdiff_t>(fragment.offset + fragment.sealedSize));
            SealContext context;
            context.resourceId = record->id.value();
            context.packageVersion = manifest_.packageVersion();
            context.fragmentIndex = index;
            context.keySlot = fragment.keySlot;
            context.nonce = fragment.nonce;
            context.tag = fragment.tag;
            auto opened = openFragment(sealed, masterKey_, context);
            if (opened.size() != fragment.plainSize) {
                secureZero(opened);
                secureZero(plain);
                return ResourceReadError::CipherFailed;
            }
            std::copy(opened.begin(),
                      opened.end(),
                      plain.begin() + static_cast<std::ptrdiff_t>(fragment.plainOffset));
            secureZero(opened);
        }
    } catch (const std::exception&) {
        secureZero(plain);
        return ResourceReadError::CipherFailed;
    }

    present(PresentedResourceView(plain.data(), plain.size()));
    secureZero(plain);
    return ResourceReadError::None;
}

void PackagedResourceStore::secureZero(std::vector<std::byte>& bytes) {
    volatile std::byte* cursor = bytes.data();
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        cursor[index] = static_cast<std::byte>(0);
    }
}

std::optional<PackagedResourceRuntime> PackagedResourceRuntime::open(
    std::filesystem::path packageRoot,
    std::string masterKey) {
    return open(std::move(packageRoot),
                std::move(masterKey),
                [](const std::filesystem::path& directory, const std::string& key) {
                    return ResourcePackageReader::readManifest(directory, key);
                },
                readFileBytes);
}

std::optional<PackagedResourceRuntime> PackagedResourceRuntime::open(
    std::filesystem::path packageRoot,
    std::string masterKey,
    ManifestReader manifestReader,
    PackagedResourceStore::PackageFileReader packageFileReader) {
    if (!manifestReader || !packageFileReader) {
        throw std::invalid_argument("resource package runtime readers are required");
    }

    auto manifest = manifestReader(packageRoot, masterKey);
    if (!manifest.has_value()) {
        return std::nullopt;
    }

    PackagedResourceStore store(std::move(*manifest),
                                std::move(packageRoot),
                                std::move(masterKey),
                                std::move(packageFileReader));
    return PackagedResourceRuntime(std::move(store));
}

ResourceReadError PackagedResourceRuntime::withPresentedResource(
    const ResourceId& id,
    const std::function<void(const PresentedResourceView&)>& present) const {
    return store_.withPresentedResource(id, present);
}

PackagedResourceRuntime::PackagedResourceRuntime(PackagedResourceStore store)
    : store_(std::move(store)) {}

std::optional<std::string> readResourceMasterKeyFromEnvFile(
    const std::filesystem::path& envFile) {
    std::ifstream input(envFile);
    if (!input) {
        return std::nullopt;
    }

    std::string line;
    while (std::getline(input, line)) {
        constexpr const char* keyName = "HARUFUSHI_RESOURCE_MASTER_KEY=";
        if (line.rfind(keyName, 0) == 0) {
            const std::string value = line.substr(std::string(keyName).size());
            if (value.size() >= 32U) {
                return value;
            }
        }
    }
    return std::nullopt;
}

} // namespace haru::engine::resources
