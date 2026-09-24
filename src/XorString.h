#pragma once
#ifndef _XOR_STRING_H_
#define _XOR_STRING_H_

#include <iostream>
#include <array>
#include <string_view>
#include <concepts>

#include <iostream>
#include <array>
#include <string>
#include <string_view>
#include <concepts>
#include <vector>

namespace __XOR_STRING__ {
    template<typename T>
    concept IsCharType = std::same_as<T, char> || std::same_as<T, wchar_t> || std::same_as<T, char8_t>;

    template <IsCharType CharT, size_t N>
    struct XorContainer {
        std::array<CharT, N> data{};
        std::array<CharT, 64> key{};
        size_t size = N;

        constexpr XorContainer(const CharT(&str)[N], const std::array<CharT, 64>& k) : key(k) {
            for (size_t i = 0; i < N; ++i) {
                data[i] = str[i] ^ key[i % 64];
            }
        }

        auto DecryptView() const {
            static thread_local std::array<CharT, N> decrypted;

            volatile CharT volatileKey[64];
            for (size_t i = 0; i < 64; ++i) {
                volatileKey[i] = key[i];
            }

            for (size_t i = 0; i < N; ++i) {
                volatile CharT* dest = &decrypted[i];
                *dest = data[i] ^ volatileKey[i % 64];
            }

            if constexpr (std::same_as<CharT, char>) return std::string_view(decrypted.data(), size - 1);
            else if constexpr (std::same_as<CharT, wchar_t>) return std::wstring_view(decrypted.data(), size - 1);
            else return std::u8string_view(decrypted.data(), size - 1);
        }

        auto DecryptString() const {
            using StringType = std::conditional_t<std::same_as<CharT, char>, std::string,
                std::conditional_t<std::same_as<CharT, wchar_t>, std::wstring,
                std::u8string>>;

            StringType decrypted;
            decrypted.resize(size - 1);

            volatile CharT volatileKey[64];
            for (size_t i = 0; i < 64; ++i) {
                volatileKey[i] = key[i];
            }

            for (size_t i = 0; i < size - 1; ++i) {
                volatile CharT* dest = &decrypted[i];
                *dest = data[i] ^ volatileKey[i % 64];
            }

            return decrypted;
        }
    };

    template<typename CharT>
    constexpr std::array<CharT, 64> Generate64ByteKey(size_t seed) {
        std::array<CharT, 64> key{};
        size_t state = seed ^ 0x55AA55AAADAFADAFULL;
        for (size_t i = 0; i < 64; ++i) {
            state = state * 6364136223846793005ULL + 1442695040888963407ULL;
            key[i] = static_cast<CharT>(state & 0xFF);
            if (key[i] == 0) key[i] = 0xAA;
        }
        return key;
    }

    template<typename T, size_t N>
    constexpr T GetCharType(const T(&)[N]) { return T{}; }

    template<typename CharType, size_t N>
    auto DebugXorStrHelper(const CharType(&str)[N]) {
        if constexpr (std::same_as<CharType, char>) return std::string_view(str, N - 1);
        else if constexpr (std::same_as<CharType, wchar_t>) return std::wstring_view(str, N - 1);
        else return std::u8string_view(str, N - 1);
    }

    template<typename CharType, size_t N>
    auto DebugXorStringHelper(const CharType(&str)[N]) {
        if constexpr (std::same_as<CharType, char>) return std::string(str, N - 1);
        else if constexpr (std::same_as<CharType, wchar_t>) return std::wstring(str, N - 1);
        else return std::u8string(str, N - 1);
    }
}

#ifdef _DEBUG

// std::string_view / std::wstring_view / std::u8string_view
#define XOR_STR(str) (__XOR_STRING__::DebugXorStrHelper(str))
// std::string / std::wstring / std::u8string
#define XOR_STRING(str) (__XOR_STRING__::DebugXorStringHelper(str))

#else
// std::string_view / std::wstring_view / std::u8string_view
#define XOR_STR(str) ([]() { \
    using CharType = decltype(__XOR_STRING__::GetCharType(str)); \
    constexpr size_t seed = []() { \
        size_t hash = 0; \
        for (char c : __TIME__) hash = hash * 31 + c; \
        return hash ^ __LINE__; \
    }(); \
    constexpr auto key = __XOR_STRING__::Generate64ByteKey<CharType>(seed); \
    static constexpr __XOR_STRING__::XorContainer<CharType, sizeof(str) / sizeof(CharType)> encrypted(str, key); \
    return encrypted.DecryptView(); \
}())

// std::string / std::wstring / std::u8string
#define XOR_STRING(str) ([]() { \
    using CharType = decltype(__XOR_STRING__::GetCharType(str)); \
    constexpr size_t seed = []() { \
        size_t hash = 0; \
        for (char c : __TIME__) hash = hash * 31 + c; \
        return hash ^ __LINE__; \
    }(); \
    constexpr auto key = __XOR_STRING__::Generate64ByteKey<CharType>(seed); \
    static constexpr __XOR_STRING__::XorContainer<CharType, sizeof(str) / sizeof(CharType)> encrypted(str, key); \
    return encrypted.DecryptString(); \
}())

#endif


#endif // !_XOR_STRING_H_