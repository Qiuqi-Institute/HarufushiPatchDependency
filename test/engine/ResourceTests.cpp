#include "support/TestHarness.hpp"

#include "engine/resources/ProtectedResourceStore.hpp"
#include "engine/resources/ResourceId.hpp"
#include "engine/resources/ResourceManifest.hpp"
#include "engine/security/ContentCipher.hpp"

#include <cstddef>
#include <memory>

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

} // namespace

HARU_TEST(resource_id_rejects_empty_and_path_like_values) {
    HARU_EXPECT_FALSE(haru::engine::resources::ResourceId::parse(""));
    HARU_EXPECT_FALSE(haru::engine::resources::ResourceId::parse("../spring"));
    HARU_EXPECT_FALSE(haru::engine::resources::ResourceId::parse("images/ui/logo"));
    HARU_EXPECT_FALSE(haru::engine::resources::ResourceId::parse("ui..logo"));
    HARU_EXPECT_TRUE(haru::engine::resources::ResourceId::parse("ui.logo"));
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
