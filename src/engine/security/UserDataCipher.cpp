#include "engine/security/UserDataCipher.hpp"

#include <cstdint>
#include <cstring>
#include <stdexcept>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dpapi.h>
#endif

namespace haru::engine::security {

namespace {

constexpr char fallbackMagic[4] = {'H', 'F', 'X', '1'};

void pushU8(std::vector<std::byte>& bytes, std::uint8_t value) {
    bytes.push_back(static_cast<std::byte>(value));
}

void pushU64(std::vector<std::byte>& bytes, std::uint64_t value) {
    for (int shift = 0; shift < 64; shift += 8) {
        pushU8(bytes, static_cast<std::uint8_t>((value >> shift) & 0xFFU));
    }
}

std::uint8_t readU8(const std::vector<std::byte>& bytes, std::size_t& cursor) {
    if (cursor >= bytes.size()) {
        throw std::runtime_error("truncated sealed user data");
    }
    return std::to_integer<std::uint8_t>(bytes[cursor++]);
}

std::uint64_t readU64(const std::vector<std::byte>& bytes, std::size_t& cursor) {
    std::uint64_t value = 0;
    for (int shift = 0; shift < 64; shift += 8) {
        value |= static_cast<std::uint64_t>(readU8(bytes, cursor)) << shift;
    }
    return value;
}

std::uint64_t fnv1a(const std::vector<std::byte>& bytes, std::uint64_t seed) {
    std::uint64_t hash = seed;
    for (const auto byte : bytes) {
        hash ^= std::to_integer<std::uint8_t>(byte);
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::uint64_t purposeSeed(const std::string& purpose) {
    std::vector<std::byte> bytes;
    bytes.reserve(purpose.size());
    for (const char character : purpose) {
        bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(character)));
    }
    return fnv1a(bytes, 1469598103934665603ULL);
}

std::vector<std::byte> fallbackProtect(const std::vector<std::byte>& plainBytes,
                                       const std::string& purpose) {
    std::vector<std::byte> output;
    for (const char character : fallbackMagic) {
        pushU8(output, static_cast<std::uint8_t>(character));
    }
    const std::uint64_t tag = fnv1a(plainBytes, purposeSeed(purpose));
    pushU64(output, tag);

    std::uint64_t stream = tag ^ 0x8B7A5C6D19E3F241ULL;
    for (const auto byte : plainBytes) {
        stream ^= stream << 13U;
        stream ^= stream >> 7U;
        stream ^= stream << 17U;
        output.push_back(static_cast<std::byte>(
            std::to_integer<std::uint8_t>(byte) ^ static_cast<std::uint8_t>(stream & 0xFFU)));
    }
    return output;
}

std::vector<std::byte> fallbackUnprotect(const std::vector<std::byte>& sealedBytes,
                                         const std::string& purpose) {
    std::size_t cursor = 0;
    for (const char character : fallbackMagic) {
        if (readU8(sealedBytes, cursor) != static_cast<std::uint8_t>(character)) {
            throw std::runtime_error("invalid fallback user data magic");
        }
    }
    const std::uint64_t tag = readU64(sealedBytes, cursor);

    std::vector<std::byte> plain;
    std::uint64_t stream = tag ^ 0x8B7A5C6D19E3F241ULL;
    while (cursor < sealedBytes.size()) {
        stream ^= stream << 13U;
        stream ^= stream >> 7U;
        stream ^= stream << 17U;
        plain.push_back(static_cast<std::byte>(
            readU8(sealedBytes, cursor) ^ static_cast<std::uint8_t>(stream & 0xFFU)));
    }

    if (fnv1a(plain, purposeSeed(purpose)) != tag) {
        throw std::runtime_error("fallback user data authentication failed");
    }
    return plain;
}

} // namespace

std::vector<std::byte> UserDataCipher::protect(const std::vector<std::byte>& plainBytes,
                                               const std::string& purpose) {
#ifdef _WIN32
    DATA_BLOB input{};
    input.pbData = reinterpret_cast<BYTE*>(const_cast<std::byte*>(plainBytes.data()));
    input.cbData = static_cast<DWORD>(plainBytes.size());

    DATA_BLOB entropy{};
    entropy.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(purpose.data()));
    entropy.cbData = static_cast<DWORD>(purpose.size());

    DATA_BLOB output{};
    if (CryptProtectData(&input,
                         nullptr,
                         &entropy,
                         nullptr,
                         nullptr,
                         CRYPTPROTECT_UI_FORBIDDEN,
                         &output) == FALSE) {
        throw std::runtime_error("failed to protect user data");
    }

    std::vector<std::byte> sealed(output.cbData);
    std::memcpy(sealed.data(), output.pbData, output.cbData);
    LocalFree(output.pbData);
    return sealed;
#else
    return fallbackProtect(plainBytes, purpose);
#endif
}

std::vector<std::byte> UserDataCipher::unprotect(const std::vector<std::byte>& sealedBytes,
                                                 const std::string& purpose) {
#ifdef _WIN32
    DATA_BLOB input{};
    input.pbData = reinterpret_cast<BYTE*>(const_cast<std::byte*>(sealedBytes.data()));
    input.cbData = static_cast<DWORD>(sealedBytes.size());

    DATA_BLOB entropy{};
    entropy.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(purpose.data()));
    entropy.cbData = static_cast<DWORD>(purpose.size());

    DATA_BLOB output{};
    if (CryptUnprotectData(&input,
                           nullptr,
                           &entropy,
                           nullptr,
                           nullptr,
                           CRYPTPROTECT_UI_FORBIDDEN,
                           &output) == FALSE) {
        throw std::runtime_error("failed to unprotect user data");
    }

    std::vector<std::byte> plain(output.cbData);
    std::memcpy(plain.data(), output.pbData, output.cbData);
    LocalFree(output.pbData);
    return plain;
#else
    return fallbackUnprotect(sealedBytes, purpose);
#endif
}

} // namespace haru::engine::security
