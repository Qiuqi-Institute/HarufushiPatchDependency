#include "SaveManager.hpp"

#include <HaruUserDataCipher>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace haru::game::systems {

namespace {

constexpr std::uint32_t saveVersion = 1;
constexpr char plainMagic[4] = {'H', 'F', 'S', 'V'};
constexpr char saveCipherPurpose[] = "HarufushiPatchDependency.SaveData.v1";

std::vector<std::byte> readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to open save file");
    }
    std::vector<char> bytes((std::istreambuf_iterator<char>(input)),
                            std::istreambuf_iterator<char>());
    std::vector<std::byte> result;
    result.reserve(bytes.size());
    for (const char byte : bytes) {
        result.push_back(static_cast<std::byte>(static_cast<unsigned char>(byte)));
    }
    return result;
}

void writeFile(const std::filesystem::path& path, const std::vector<std::byte>& bytes) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("failed to write save file");
    }
    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
}

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

void pushI32(std::vector<std::byte>& bytes, int value) {
    pushU32(bytes, static_cast<std::uint32_t>(value));
}

void pushString(std::vector<std::byte>& bytes, const std::string& value) {
    if (value.size() > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
        throw std::runtime_error("save string is too large");
    }
    pushU32(bytes, static_cast<std::uint32_t>(value.size()));
    for (const char character : value) {
        pushU8(bytes, static_cast<std::uint8_t>(character));
    }
}

std::uint8_t dailyActionByte(DailyAction action) {
    switch (action) {
    case DailyAction::Study:
        return 0;
    case DailyAction::Modding:
        return 1;
    case DailyAction::SpendTimeWithHarufushi:
        return 2;
    case DailyAction::Rest:
        return 3;
    }
    return 0;
}

DailyAction dailyActionFromByte(std::uint8_t value) {
    switch (value) {
    case 0:
        return DailyAction::Study;
    case 1:
        return DailyAction::Modding;
    case 2:
        return DailyAction::SpendTimeWithHarufushi;
    case 3:
        return DailyAction::Rest;
    default:
        throw std::runtime_error("unknown daily action in save");
    }
}

void pushStats(std::vector<std::byte>& bytes, const DailyStats& stats) {
    pushI32(bytes, stats.day);
    pushI32(bytes, stats.energy);
    pushI32(bytes, stats.studyFocus);
    pushI32(bytes, stats.modProgress);
    pushI32(bytes, stats.harufushiBond);
    pushI32(bytes, stats.dependence);
}

class ReadCursor {
public:
    explicit ReadCursor(const std::vector<std::byte>& bytes) : bytes_(bytes) {}

    std::uint8_t u8() {
        require(1);
        return std::to_integer<std::uint8_t>(bytes_[cursor_++]);
    }

    std::uint32_t u32() {
        std::uint32_t value = 0;
        for (int shift = 0; shift < 32; shift += 8) {
            value |= static_cast<std::uint32_t>(u8()) << shift;
        }
        return value;
    }

    std::uint64_t u64() {
        std::uint64_t value = 0;
        for (int shift = 0; shift < 64; shift += 8) {
            value |= static_cast<std::uint64_t>(u8()) << shift;
        }
        return value;
    }

    int i32() {
        return static_cast<int>(u32());
    }

    std::string string() {
        const auto size = u32();
        require(size);
        std::string value;
        value.reserve(size);
        for (std::uint32_t index = 0; index < size; ++index) {
            value.push_back(static_cast<char>(u8()));
        }
        return value;
    }

    DailyStats stats() {
        DailyStats result;
        result.day = i32();
        result.energy = i32();
        result.studyFocus = i32();
        result.modProgress = i32();
        result.harufushiBond = i32();
        result.dependence = i32();
        return result;
    }

    bool exhausted() const {
        return cursor_ == bytes_.size();
    }

private:
    void require(std::size_t count) const {
        if (cursor_ + count > bytes_.size()) {
            throw std::runtime_error("truncated save data");
        }
    }

    const std::vector<std::byte>& bytes_;
    std::size_t cursor_ = 0;
};

std::vector<std::byte> serializeSave(const GameSave& save) {
    std::vector<std::byte> bytes;
    for (const char character : plainMagic) {
        pushU8(bytes, static_cast<std::uint8_t>(character));
    }
    pushU32(bytes, saveVersion);
    pushString(bytes, save.id);
    pushString(bytes, save.localeTag);
    pushStats(bytes, save.stats);
    pushU32(bytes, static_cast<std::uint32_t>(save.history.size()));
    for (const auto& operation : save.history) {
        pushU64(bytes, operation.sequence);
        pushU8(bytes, static_cast<std::uint8_t>(operation.kind));
        pushU8(bytes, dailyActionByte(operation.dailyAction));
        pushStats(bytes, operation.stats);
        pushString(bytes, operation.localeTag);
    }
    return bytes;
}

GameSave deserializeSave(const std::vector<std::byte>& bytes) {
    ReadCursor cursor(bytes);
    for (const char character : plainMagic) {
        if (cursor.u8() != static_cast<std::uint8_t>(character)) {
            throw std::runtime_error("invalid save magic");
        }
    }
    if (cursor.u32() != saveVersion) {
        throw std::runtime_error("unsupported save version");
    }

    GameSave save;
    save.id = cursor.string();
    save.localeTag = cursor.string();
    save.stats = cursor.stats();
    const auto historySize = cursor.u32();
    save.history.reserve(historySize);
    for (std::uint32_t index = 0; index < historySize; ++index) {
        SaveOperation operation;
        operation.sequence = cursor.u64();
        operation.kind = static_cast<SaveOperationKind>(cursor.u8());
        operation.dailyAction = dailyActionFromByte(cursor.u8());
        operation.stats = cursor.stats();
        operation.localeTag = cursor.string();
        save.history.push_back(std::move(operation));
    }
    if (!cursor.exhausted()) {
        throw std::runtime_error("save data contains trailing bytes");
    }
    return save;
}

std::vector<std::byte> protectSaveBytes(const std::vector<std::byte>& plainBytes) {
    return engine::security::UserDataCipher::protect(plainBytes, saveCipherPurpose);
}

std::vector<std::byte> unprotectSaveBytes(const std::vector<std::byte>& sealedBytes) {
    return engine::security::UserDataCipher::unprotect(sealedBytes, saveCipherPurpose);
}

SaveOperation makeOperation(SaveOperationKind kind,
                            DailyAction dailyAction,
                            const DailyStats& stats,
                            const std::string& localeTag,
                            std::uint64_t sequence) {
    SaveOperation operation;
    operation.sequence = sequence;
    operation.kind = kind;
    operation.dailyAction = dailyAction;
    operation.stats = stats;
    operation.localeTag = localeTag;
    return operation;
}

} // namespace

SaveManager::SaveManager(std::filesystem::path saveRoot) : saveRoot_(std::move(saveRoot)) {}

SaveLoadReport SaveManager::loadAll() {
    saves_.clear();
    activeIndex_.reset();
    SaveLoadReport report;

    if (!std::filesystem::exists(saveRoot_)) {
        return report;
    }

    for (const auto& entry : std::filesystem::directory_iterator(saveRoot_)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".hfsave") {
            continue;
        }

        try {
            auto save = deserializeSave(unprotectSaveBytes(readFile(entry.path())));
            saves_.push_back(std::move(save));
            ++report.loaded;
        } catch (const std::exception&) {
            ++report.rejected;
        }
    }

    std::sort(saves_.begin(), saves_.end(), [](const GameSave& left, const GameSave& right) {
        return left.id < right.id;
    });
    if (!saves_.empty()) {
        activeIndex_ = saves_.size() - 1;
    }
    return report;
}

const std::vector<GameSave>& SaveManager::saves() const {
    return saves_;
}

const GameSave& SaveManager::createNewGame(std::string localeTag) {
    GameSave save;
    save.id = createSaveId();
    save.localeTag = std::move(localeTag);
    save.stats = DailyStats{};
    save.history.push_back(makeOperation(SaveOperationKind::NewGame,
                                         DailyAction::Study,
                                         save.stats,
                                         save.localeTag,
                                         1));
    flush(save);
    saves_.push_back(std::move(save));
    activeIndex_ = saves_.size() - 1;
    return saves_[*activeIndex_];
}

void SaveManager::recordDailyAction(DailyAction action,
                                    const DailyStats& statsAfter,
                                    std::string localeTag) {
    GameSave& save = requireActiveSave(std::move(localeTag));
    save.stats = statsAfter;
    save.history.push_back(makeOperation(SaveOperationKind::DailyAction,
                                         action,
                                         save.stats,
                                         save.localeTag,
                                         static_cast<std::uint64_t>(save.history.size() + 1)));
    flush(save);
}

void SaveManager::recordLocaleChange(std::string localeTag) {
    GameSave* save = activeSave();
    if (save == nullptr) {
        return;
    }

    save->localeTag = std::move(localeTag);
    save->history.push_back(makeOperation(SaveOperationKind::LocaleChanged,
                                          DailyAction::Study,
                                          save->stats,
                                          save->localeTag,
                                          static_cast<std::uint64_t>(save->history.size() + 1)));
    flush(*save);
}

const GameSave* SaveManager::activeSave() const {
    if (!activeIndex_.has_value() || *activeIndex_ >= saves_.size()) {
        return nullptr;
    }
    return &saves_[*activeIndex_];
}

GameSave* SaveManager::activeSave() {
    if (!activeIndex_.has_value() || *activeIndex_ >= saves_.size()) {
        return nullptr;
    }
    return &saves_[*activeIndex_];
}

bool SaveManager::activateSave(const std::string& saveId) {
    for (std::size_t index = 0; index < saves_.size(); ++index) {
        if (saves_[index].id == saveId) {
            activeIndex_ = index;
            return true;
        }
    }
    return false;
}

std::filesystem::path SaveManager::savePath(const std::string& saveId) const {
    return saveRoot_ / (saveId + ".hfsave");
}

const std::filesystem::path& SaveManager::saveRoot() const {
    return saveRoot_;
}

void SaveManager::flush(const GameSave& save) const {
    writeFile(savePath(save.id), protectSaveBytes(serializeSave(save)));
}

GameSave& SaveManager::requireActiveSave(std::string localeTag) {
    if (GameSave* save = activeSave()) {
        if (!localeTag.empty()) {
            save->localeTag = std::move(localeTag);
        }
        return *save;
    }
    createNewGame(std::move(localeTag));
    return *activeSave();
}

std::string SaveManager::createSaveId() {
    static std::atomic_uint64_t counter{0};
    const auto ticks = std::chrono::system_clock::now().time_since_epoch().count();
    std::ostringstream id;
    id << "save-" << std::hex << ticks << '-' << counter.fetch_add(1);
    return id.str();
}

} // namespace haru::game::systems
