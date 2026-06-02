#include "support/TestHarness.hpp"

#include "game/systems/SaveManager.hpp"

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

std::filesystem::path uniqueSaveRoot(const std::string& name) {
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    auto root = std::filesystem::temp_directory_path() /
                ("harufushi-save-tests-" + name + "-" + std::to_string(stamp));
    std::filesystem::remove_all(root);
    return root;
}

std::vector<std::byte> readBytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::vector<char> raw((std::istreambuf_iterator<char>(input)),
                          std::istreambuf_iterator<char>());
    std::vector<std::byte> bytes;
    bytes.reserve(raw.size());
    for (const char byte : raw) {
        bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(byte)));
    }
    return bytes;
}

bool containsPlainText(const std::vector<std::byte>& bytes, const std::string& needle) {
    if (needle.empty() || bytes.size() < needle.size()) {
        return false;
    }

    for (std::size_t index = 0; index + needle.size() <= bytes.size(); ++index) {
        bool match = true;
        for (std::size_t offset = 0; offset < needle.size(); ++offset) {
            if (bytes[index + offset] != static_cast<std::byte>(needle[offset])) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }

    return false;
}

std::size_t saveFileCount(const std::filesystem::path& root) {
    if (!std::filesystem::exists(root)) {
        return 0;
    }

    std::size_t count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (entry.path().extension() == ".hfsave") {
            ++count;
        }
    }
    return count;
}

} // namespace

HARU_TEST(save_manager_realtime_saves_one_active_game_to_one_encrypted_file) {
    const auto root = uniqueSaveRoot("realtime");
    haru::game::systems::SaveManager manager(root);
    const auto loadReport = manager.loadAll();
    HARU_EXPECT_EQ(loadReport.loaded, 0);
    HARU_EXPECT_EQ(loadReport.rejected, 0);

    const auto& created = manager.createNewGame("en-US");
    const std::string saveId = created.id;
    haru::game::systems::DailyLoopState state;
    state.apply(haru::game::systems::DailyAction::Modding);
    manager.recordDailyAction(haru::game::systems::DailyAction::Modding,
                              state.stats(),
                              "en-US");

    HARU_EXPECT_EQ(saveFileCount(root), static_cast<std::size_t>(1));
    const auto savePath = manager.savePath(saveId);
    const auto savedBytes = readBytes(savePath);
    HARU_EXPECT_FALSE(containsPlainText(savedBytes, "Modding"));
    HARU_EXPECT_FALSE(containsPlainText(savedBytes, "energy"));
    HARU_EXPECT_FALSE(containsPlainText(savedBytes, "Harufushi"));
    HARU_EXPECT_FALSE(containsPlainText(savedBytes, "en-US"));

    haru::game::systems::SaveManager reloaded(root);
    const auto reloadReport = reloaded.loadAll();
    HARU_EXPECT_EQ(reloadReport.loaded, 1);
    HARU_EXPECT_EQ(reloadReport.rejected, 0);
    HARU_EXPECT_EQ(reloaded.saves().size(), static_cast<std::size_t>(1));
    HARU_EXPECT_EQ(reloaded.saves()[0].id, saveId);
    HARU_EXPECT_EQ(reloaded.saves()[0].stats.energy, state.stats().energy);
    HARU_EXPECT_EQ(reloaded.saves()[0].stats.modProgress, state.stats().modProgress);
    HARU_EXPECT_EQ(reloaded.saves()[0].history.size(), static_cast<std::size_t>(2));
    HARU_EXPECT_EQ(reloaded.saves()[0].history[1].dailyAction,
                   haru::game::systems::DailyAction::Modding);

    state.apply(haru::game::systems::DailyAction::Rest);
    manager.recordDailyAction(haru::game::systems::DailyAction::Rest,
                              state.stats(),
                              "en-US");
    HARU_EXPECT_EQ(manager.activeSave()->id, saveId);
    HARU_EXPECT_EQ(saveFileCount(root), static_cast<std::size_t>(1));
    HARU_EXPECT_EQ(manager.activeSave()->history.size(), static_cast<std::size_t>(3));

    std::filesystem::remove_all(root);
}

HARU_TEST(save_manager_supports_multiple_saves_without_save_as_for_active_session) {
    const auto root = uniqueSaveRoot("multiple");
    haru::game::systems::SaveManager manager(root);

    const std::string firstId = manager.createNewGame("en-US").id;
    const std::string secondId = manager.createNewGame("ja-JP").id;

    HARU_EXPECT_FALSE(firstId == secondId);
    HARU_EXPECT_EQ(manager.saves().size(), static_cast<std::size_t>(2));
    HARU_EXPECT_EQ(saveFileCount(root), static_cast<std::size_t>(2));
    HARU_EXPECT_EQ(manager.activeSave()->id, secondId);

    haru::game::systems::DailyLoopState state;
    state.apply(haru::game::systems::DailyAction::Study);
    manager.recordDailyAction(haru::game::systems::DailyAction::Study,
                              state.stats(),
                              "ja-JP");

    HARU_EXPECT_EQ(manager.activeSave()->id, secondId);
    HARU_EXPECT_EQ(saveFileCount(root), static_cast<std::size_t>(2));

    std::filesystem::remove_all(root);
}

HARU_TEST(save_manager_can_activate_loaded_save_without_creating_copy) {
    const auto root = uniqueSaveRoot("activate");
    haru::game::systems::SaveManager manager(root);

    const std::string firstId = manager.createNewGame("en-US").id;
    const std::string secondId = manager.createNewGame("ja-JP").id;

    HARU_EXPECT_TRUE(manager.activateSave(firstId));
    HARU_EXPECT_EQ(manager.activeSave()->id, firstId);
    HARU_EXPECT_EQ(saveFileCount(root), static_cast<std::size_t>(2));
    HARU_EXPECT_FALSE(manager.activateSave("missing"));
    HARU_EXPECT_EQ(manager.activeSave()->id, firstId);
    HARU_EXPECT_FALSE(firstId == secondId);

    std::filesystem::remove_all(root);
}

HARU_TEST(save_manager_rejects_tampered_encrypted_save_files) {
    const auto root = uniqueSaveRoot("tamper");
    haru::game::systems::SaveManager manager(root);
    const std::string saveId = manager.createNewGame("zh-CN").id;
    const auto path = manager.savePath(saveId);

    auto bytes = readBytes(path);
    HARU_EXPECT_TRUE(!bytes.empty());
    bytes[bytes.size() - 1] = static_cast<std::byte>(
        std::to_integer<unsigned char>(bytes[bytes.size() - 1]) ^ 0x5A);
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
    output.close();

    haru::game::systems::SaveManager reloaded(root);
    const auto report = reloaded.loadAll();
    HARU_EXPECT_EQ(report.loaded, 0);
    HARU_EXPECT_EQ(report.rejected, 1);
    HARU_EXPECT_TRUE(reloaded.saves().empty());

    std::filesystem::remove_all(root);
}
