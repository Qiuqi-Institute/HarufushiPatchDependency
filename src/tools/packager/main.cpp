#include "HaruResourcePackage"

#include <algorithm>
#include <cstddef>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

namespace {

struct Arguments {
    std::filesystem::path source;
    std::filesystem::path output;
    std::filesystem::path env;
};

std::optional<Arguments> parseArguments(int argc, char** argv) {
    Arguments arguments;
    for (int index = 1; index + 1 < argc; index += 2) {
        const std::string name = argv[index];
        const std::string value = argv[index + 1];
        if (name == "--source") {
            arguments.source = value;
        } else if (name == "--output") {
            arguments.output = value;
        } else if (name == "--env") {
            arguments.env = value;
        } else {
            return std::nullopt;
        }
    }

    if (arguments.source.empty() || arguments.output.empty() || arguments.env.empty()) {
        return std::nullopt;
    }
    return arguments;
}

std::vector<std::byte> readBytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to read resource input");
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

std::string lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char value) {
        return static_cast<char>(std::tolower(value));
    });
    return text;
}

haru::engine::resources::ResourceKind kindFor(const std::filesystem::path& relative) {
    const std::string first = relative.begin() == relative.end() ? "" : lower(relative.begin()->string());
    const std::string extension = lower(relative.extension().string());
    if (first == "images") {
        return haru::engine::resources::ResourceKind::Image;
    }
    if (first == "icons") {
        return haru::engine::resources::ResourceKind::Icon;
    }
    if (first == "audio") {
        return haru::engine::resources::ResourceKind::Audio;
    }
    if (first == "localization" || extension == ".harulang") {
        return haru::engine::resources::ResourceKind::Localization;
    }
    return haru::engine::resources::ResourceKind::Data;
}

std::string localeFor(const std::filesystem::path& relative) {
    auto iterator = relative.begin();
    if (iterator == relative.end() || lower(iterator->string()) != "localization") {
        return {};
    }
    ++iterator;
    if (iterator == relative.end()) {
        return {};
    }
    return iterator->string();
}

std::string idFor(const std::filesystem::path& relative) {
    std::filesystem::path withoutExtension = relative;
    withoutExtension.replace_extension();
    std::string id;
    for (const auto& part : withoutExtension) {
        if (!id.empty()) {
            id.push_back('.');
        }
        std::string segment = part.string();
        for (char& character : segment) {
            if (character == ' ') {
                character = '_';
            }
        }
        id += segment;
    }
    return id;
}

bool shouldPackage(const std::filesystem::path& path) {
    if (!std::filesystem::is_regular_file(path)) {
        return false;
    }
    const std::string extension = lower(path.extension().string());
    return extension == ".png" || extension == ".ico" || extension == ".jpg" ||
           extension == ".jpeg" || extension == ".webp" || extension == ".wav" ||
           extension == ".ogg" || extension == ".mp3" || extension == ".harulang" ||
           extension == ".json" || extension == ".harudlg" || extension == ".txt" ||
           extension == ".bin" || extension == ".ttf";
}

} // namespace

int main(int argc, char** argv) {
    const auto arguments = parseArguments(argc, argv);
    if (!arguments.has_value()) {
        std::cerr << "usage: harufushi_packager --source <resources> --output <dir> --env <.env>\n";
        return 2;
    }

    const auto key =
        haru::engine::resources::readResourceMasterKeyFromEnvFile(arguments->env);
    if (!key.has_value()) {
        std::cerr << "missing HARUFUSHI_RESOURCE_MASTER_KEY in " << arguments->env << "\n";
        return 3;
    }

    std::vector<haru::engine::resources::ResourcePackageInput> inputs;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(arguments->source)) {
        if (!shouldPackage(entry.path())) {
            continue;
        }
        const auto relative = std::filesystem::relative(entry.path(), arguments->source);
        const auto id = haru::engine::resources::ResourceId::parse(idFor(relative));
        if (!id.has_value()) {
            std::cerr << "invalid resource id for " << relative << "\n";
            return 4;
        }
        inputs.push_back({*id,
                          kindFor(relative),
                          relative.generic_string(),
                          localeFor(relative),
                          readBytes(entry.path())});
    }

    std::sort(inputs.begin(), inputs.end(), [](const auto& left, const auto& right) {
        return left.id.value() < right.id.value();
    });

    haru::engine::resources::ResourcePackageBuildOptions options;
    options.masterKey = *key;
    options.packageVersion = 1;
    options.maxPackageBytes = 128ULL * 1024ULL * 1024ULL;
    options.maxFragmentPlainBytes = 4ULL * 1024ULL * 1024ULL;

    const auto bundle = haru::engine::resources::ResourcePackageBuilder::build(inputs, options);
    haru::engine::resources::ResourcePackageWriter::write(arguments->output, bundle, *key);
    return 0;
}
