#include "support/TestHarness.hpp"

#include "engine/resources/ProtectedResourceStore.hpp"
#include "engine/resources/ResourcePackage.hpp"
#include "engine/resources/ResourceId.hpp"
#include "engine/resources/ResourceManifest.hpp"
#include "engine/security/ContentCipher.hpp"

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>

namespace {

class PrefixCipher final : public haru::engine::security::ContentCipher {
public:
    std::vector<std::byte> open(
        const haru::engine::security::CipherContext& context,
        const std::vector<std::byte>& sealedBytes) const override {
        HARU_EXPECT_EQ(context.packageVersion, 1);
        HARU_EXPECT_EQ(context.resourceId, "text.boot");

        std::vector<std::byte> result;
        for (std::size_t index = 1; index < sealedBytes.size(); ++index) {
            result.push_back(sealedBytes[index]);
        }
        return result;
    }
};

std::vector<std::byte> bytes(std::initializer_list<unsigned char> values) {
    std::vector<std::byte> result;
    for (const auto value : values) {
        result.push_back(static_cast<std::byte>(value));
    }
    return result;
}

std::vector<std::byte> bytesFromText(const std::string& text) {
    std::vector<std::byte> result;
    result.reserve(text.size());
    for (const char character : text) {
        result.push_back(static_cast<std::byte>(static_cast<unsigned char>(character)));
    }
    return result;
}

bool containsPlainText(const std::vector<std::byte>& bytes, const std::string& text) {
    if (text.empty() || bytes.size() < text.size()) {
        return false;
    }

    for (std::size_t index = 0; index + text.size() <= bytes.size(); ++index) {
        bool found = true;
        for (std::size_t offset = 0; offset < text.size(); ++offset) {
            if (bytes[index + offset] != static_cast<std::byte>(text[offset])) {
                found = false;
                break;
            }
        }
        if (found) {
            return true;
        }
    }

    return false;
}

std::filesystem::path uniquePackageRoot(const std::string& name) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    auto root = std::filesystem::temp_directory_path() /
                ("harufushi-pack-tests-" + name + "-" + std::to_string(stamp));
    std::filesystem::remove_all(root);
    return root;
}

std::vector<std::byte> readBytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::vector<char> raw((std::istreambuf_iterator<char>(input)),
                          std::istreambuf_iterator<char>());
    std::vector<std::byte> result;
    result.reserve(raw.size());
    for (const char byte : raw) {
        result.push_back(static_cast<std::byte>(static_cast<unsigned char>(byte)));
    }
    return result;
}

std::filesystem::path sourceResourcePath(const std::string& relativePath) {
#ifdef HARUFUSHI_SOURCE_DIR
    return std::filesystem::path(HARUFUSHI_SOURCE_DIR) / "resources" / relativePath;
#else
    return std::filesystem::path("resources") / relativePath;
#endif
}

} // namespace

HARU_TEST(resource_id_rejects_empty_and_path_like_values) {
    HARU_EXPECT_FALSE(haru::engine::resources::ResourceId::parse(""));
    HARU_EXPECT_FALSE(haru::engine::resources::ResourceId::parse("../spring"));
    HARU_EXPECT_FALSE(haru::engine::resources::ResourceId::parse("images/ui/logo"));
    HARU_EXPECT_FALSE(haru::engine::resources::ResourceId::parse("ui..logo"));
    HARU_EXPECT_TRUE(haru::engine::resources::ResourceId::parse("ui.logo"));
}

HARU_TEST(font_resources_include_zen_maru_and_resource_han_fallback_faces) {
    HARU_EXPECT_TRUE(std::filesystem::exists(
        sourceResourcePath("fonts/ZenMaruGothic-Black.ttf")));
    HARU_EXPECT_TRUE(std::filesystem::exists(
        sourceResourcePath("fonts/ZenMaruGothic-Bold.ttf")));
    HARU_EXPECT_TRUE(std::filesystem::exists(
        sourceResourcePath("fonts/ResourceHanRoundedSC-Heavy.ttf")));
    HARU_EXPECT_TRUE(std::filesystem::exists(
        sourceResourcePath("fonts/ResourceHanRoundedSC-Bold.ttf")));
}

HARU_TEST(manifest_finds_encrypted_resource_records_by_id) {
    using namespace haru::engine::resources;

    ResourceManifest manifest(1);
    const auto id = ResourceId::parse("text.boot").value();
    manifest.add({id, ResourceKind::Data, "data/boot.bin", 4, true, "zh-CN"});

    const auto* found = manifest.find(id);

    HARU_EXPECT_TRUE(found != nullptr);
    HARU_EXPECT_TRUE(found->encrypted);
    HARU_EXPECT_EQ(found->locale, "zh-CN");
    HARU_EXPECT_EQ(manifest.packageVersion(), 1);
}

HARU_TEST(protected_store_refuses_encrypted_resources_without_cipher) {
    using namespace haru::engine::resources;

    ResourceManifest manifest(1);
    const auto id = ResourceId::parse("text.boot").value();
    manifest.add({id, ResourceKind::Data, "data/boot.bin", 4, true, ""});

    ProtectedResourceStore store(manifest, nullptr);
    store.putSealed(id, bytes({0x99, 'b', 'o', 'o', 't'}));

    bool callbackCalled = false;
    const auto result = store.withPresentedResource(id, [&](const PresentedResourceView&) {
        callbackCalled = true;
    });

    HARU_EXPECT_EQ(result, ResourceReadError::MissingCipher);
    HARU_EXPECT_FALSE(callbackCalled);
}

HARU_TEST(protected_store_exposes_plaintext_only_inside_presentation_callback) {
    using namespace haru::engine::resources;

    ResourceManifest manifest(1);
    const auto id = ResourceId::parse("text.boot").value();
    manifest.add({id, ResourceKind::Data, "data/boot.bin", 4, true, ""});

    PrefixCipher cipher;
    ProtectedResourceStore store(manifest, &cipher);
    store.putSealed(id, bytes({0x99, 'b', 'o', 'o', 't'}));

    std::size_t visibleSize = 0;
    char firstByte = '\0';
    const auto result = store.withPresentedResource(id, [&](const PresentedResourceView& view) {
        visibleSize = view.size();
        firstByte = static_cast<char>(view.data()[0]);
    });

    HARU_EXPECT_EQ(result, ResourceReadError::None);
    HARU_EXPECT_EQ(visibleSize, static_cast<std::size_t>(4));
    HARU_EXPECT_EQ(firstByte, 'b');
}

HARU_TEST(protected_store_rejects_missing_payload_before_presentation_callback) {
    using namespace haru::engine::resources;

    ResourceManifest manifest(1);
    const auto id = ResourceId::parse("text.boot").value();
    manifest.add({id, ResourceKind::Data, "data/boot.bin", 4, true, ""});

    PrefixCipher cipher;
    ProtectedResourceStore store(manifest, &cipher);
    bool callbackCalled = false;

    const auto result = store.withPresentedResource(id, [&](const PresentedResourceView&) {
        callbackCalled = true;
    });

    HARU_EXPECT_EQ(result, ResourceReadError::MissingPayload);
    HARU_EXPECT_FALSE(callbackCalled);
}

HARU_TEST(resource_package_builder_slices_encrypts_and_obfuscates_assets) {
    using namespace haru::engine::resources;

    const auto textId = ResourceId::parse("text.boot").value();
    const auto imageId = ResourceId::parse("image.home").value();
    ResourcePackageBuildOptions options;
    options.packageVersion = 7;
    options.maxPackageBytes = 64;
    options.maxFragmentPlainBytes = 18;
    options.masterKey = "local-test-master-key-with-enough-length-for-package-tests";

    const auto bundle = ResourcePackageBuilder::build(
        {{textId,
          ResourceKind::Localization,
          "localization/zh-CN/game.harulang",
          "zh-CN",
          bytesFromText("text menu.settings = \"游戏设置\"")},
         {imageId,
          ResourceKind::Image,
          "images/backgrounds/home_chunfu.png",
          "",
          bytesFromText("PNG-LIKE-PLAINTEXT-CHUNFU-HOME-BACKGROUND")}},
        options);

    HARU_EXPECT_EQ(bundle.manifest.packageVersion(), 7);
    HARU_EXPECT_TRUE(bundle.packages.size() >= static_cast<std::size_t>(2));
    for (const auto& package : bundle.packages) {
        HARU_EXPECT_TRUE(package.bytes.size() <= options.maxPackageBytes);
        HARU_EXPECT_TRUE(!containsPlainText(package.bytes, "游戏设置"));
        HARU_EXPECT_TRUE(!containsPlainText(package.bytes, "CHUNFU"));
    }

    const auto* imageRecord = bundle.manifest.find(imageId);
    HARU_EXPECT_TRUE(imageRecord != nullptr);
    HARU_EXPECT_TRUE(imageRecord->encrypted);
    HARU_EXPECT_TRUE(imageRecord->fragments.size() >= static_cast<std::size_t>(2));
    HARU_EXPECT_TRUE(imageRecord->fragments[0].nonce.size() == static_cast<std::size_t>(12));
    HARU_EXPECT_TRUE(imageRecord->fragments[0].tag.size() == static_cast<std::size_t>(16));
    HARU_EXPECT_FALSE(imageRecord->packagePath == "images/backgrounds/home_chunfu.png");
}

HARU_TEST(packaged_resource_store_reads_single_resource_and_rejects_tamper) {
    using namespace haru::engine::resources;

    const auto root = uniquePackageRoot("runtime");
    const auto id = ResourceId::parse("image.home").value();
    ResourcePackageBuildOptions options;
    options.packageVersion = 3;
    options.maxPackageBytes = 80;
    options.maxFragmentPlainBytes = 16;
    options.masterKey = "runtime-test-master-key-with-enough-length-for-package-tests";
    const std::string plainText = "ONLY-THIS-IMAGE-SHOULD-OPEN";
    const auto bundle = ResourcePackageBuilder::build(
        {{id, ResourceKind::Image, "images/backgrounds/home_chunfu.png", "", bytesFromText(plainText)}},
        options);

    ResourcePackageWriter::write(root, bundle, options.masterKey);
    const auto loadedManifest = ResourcePackageReader::readManifest(root, options.masterKey);
    HARU_EXPECT_TRUE(loadedManifest.has_value());

    PackagedResourceStore store(*loadedManifest, root, options.masterKey);
    std::string observed;
    const auto result = store.withPresentedResource(id, [&](const PresentedResourceView& view) {
        observed.assign(reinterpret_cast<const char*>(view.data()), view.size());
    });

    HARU_EXPECT_EQ(result, ResourceReadError::None);
    HARU_EXPECT_EQ(observed, plainText);

    const auto* record = loadedManifest->find(id);
    HARU_EXPECT_TRUE(record != nullptr);
    HARU_EXPECT_TRUE(!record->fragments.empty());
    auto packagePath = root / record->fragments[0].packagePath;
    auto packageBytes = readBytes(packagePath);
    packageBytes[record->fragments[0].offset] =
        static_cast<std::byte>(std::to_integer<unsigned char>(
                                   packageBytes[record->fragments[0].offset]) ^
                               0x41U);
    std::ofstream output(packagePath, std::ios::binary | std::ios::trunc);
    output.write(reinterpret_cast<const char*>(packageBytes.data()),
                 static_cast<std::streamsize>(packageBytes.size()));
    output.close();

    bool callbackCalled = false;
    const auto tampered = store.withPresentedResource(id, [&](const PresentedResourceView&) {
        callbackCalled = true;
    });

    HARU_EXPECT_EQ(tampered, ResourceReadError::CipherFailed);
    HARU_EXPECT_FALSE(callbackCalled);
    std::filesystem::remove_all(root);
}

HARU_TEST(packaged_resource_store_reuses_package_file_bytes_within_single_lease) {
    using namespace haru::engine::resources;

    const auto root = uniquePackageRoot("single-lease-cache");
    const auto id = ResourceId::parse("image.fragmented").value();
    ResourcePackageBuildOptions options;
    options.packageVersion = 4;
    options.maxPackageBytes = 4096;
    options.maxFragmentPlainBytes = 7;
    options.masterKey = "lease-cache-test-master-key-with-enough-length-for-package-tests";
    const std::string plainText = "ONE-IMAGE-SPLIT-ACROSS-MANY-FRAGMENTS";
    const auto bundle = ResourcePackageBuilder::build(
        {{id,
          ResourceKind::Image,
          "images/backgrounds/fragmented.png",
          "",
          bytesFromText(plainText)}},
        options);

    ResourcePackageWriter::write(root, bundle, options.masterKey);
    const auto loadedManifest = ResourcePackageReader::readManifest(root, options.masterKey);
    HARU_EXPECT_TRUE(loadedManifest.has_value());
    const auto* record = loadedManifest->find(id);
    HARU_EXPECT_TRUE(record != nullptr);
    HARU_EXPECT_TRUE(record->fragments.size() > static_cast<std::size_t>(1));
    const auto packagePath = record->fragments.front().packagePath;
    for (const auto& fragment : record->fragments) {
        HARU_EXPECT_EQ(fragment.packagePath, packagePath);
    }

    std::size_t packageReadCount = 0;
    PackagedResourceStore store(
        *loadedManifest,
        root,
        options.masterKey,
        [&](const std::filesystem::path& path) {
            ++packageReadCount;
            return readBytes(path);
        });

    std::string observed;
    const auto result = store.withPresentedResource(id, [&](const PresentedResourceView& view) {
        observed.assign(reinterpret_cast<const char*>(view.data()), view.size());
    });

    HARU_EXPECT_EQ(result, ResourceReadError::None);
    HARU_EXPECT_EQ(observed, plainText);
    HARU_EXPECT_EQ(packageReadCount, static_cast<std::size_t>(1));
    std::filesystem::remove_all(root);
}

HARU_TEST(packaged_resource_runtime_reads_manifest_once_for_multiple_resources) {
    using namespace haru::engine::resources;

    const auto root = uniquePackageRoot("runtime-manifest-cache");
    const auto imageId = ResourceId::parse("image.home").value();
    const auto fontId = ResourceId::parse("fonts.ZenMaruGothic-Bold").value();
    ResourcePackageBuildOptions options;
    options.packageVersion = 5;
    options.maxPackageBytes = 4096;
    options.maxFragmentPlainBytes = 11;
    options.masterKey = "runtime-cache-test-master-key-with-enough-length-for-package-tests";
    const std::string imageText = "PACKAGED-HOME-IMAGE";
    const std::string fontText = "PACKAGED-FONT-BYTES";
    const auto bundle = ResourcePackageBuilder::build(
        {{imageId,
          ResourceKind::Image,
          "images/backgrounds/home_chunfu.png",
          "",
          bytesFromText(imageText)},
         {fontId,
          ResourceKind::Data,
          "fonts/ZenMaruGothic-Bold.ttf",
          "",
          bytesFromText(fontText)}},
        options);

    ResourcePackageWriter::write(root, bundle, options.masterKey);
    std::size_t manifestReadCount = 0;
    auto runtime = PackagedResourceRuntime::open(
        root,
        options.masterKey,
        [&](const std::filesystem::path& directory, const std::string& masterKey) {
            ++manifestReadCount;
            return ResourcePackageReader::readManifest(directory, masterKey);
        },
        readBytes);
    HARU_EXPECT_TRUE(runtime.has_value());

    std::string observedImage;
    const auto imageResult =
        runtime->withPresentedResource(imageId, [&](const PresentedResourceView& view) {
            observedImage.assign(reinterpret_cast<const char*>(view.data()), view.size());
        });
    std::string observedFont;
    const auto fontResult =
        runtime->withPresentedResource(fontId, [&](const PresentedResourceView& view) {
            observedFont.assign(reinterpret_cast<const char*>(view.data()), view.size());
        });

    HARU_EXPECT_EQ(imageResult, ResourceReadError::None);
    HARU_EXPECT_EQ(fontResult, ResourceReadError::None);
    HARU_EXPECT_EQ(observedImage, imageText);
    HARU_EXPECT_EQ(observedFont, fontText);
    HARU_EXPECT_EQ(manifestReadCount, static_cast<std::size_t>(1));
    std::filesystem::remove_all(root);
}
