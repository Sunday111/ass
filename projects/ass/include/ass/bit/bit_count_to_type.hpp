#pragma once

#include <cstddef>
#include <cstdint>

namespace ass
{
template <size_t bits_count>
struct BitsCountToUnsignedInt;

inline constexpr size_t kBitsInByte = 8;

template <>
struct BitsCountToUnsignedInt<sizeof(uint64_t) * kBitsInByte>
{
    using Type = uint64_t;
};

template <>
struct BitsCountToUnsignedInt<sizeof(uint32_t) * kBitsInByte>
{
    using Type = uint32_t;
};

template <>
struct BitsCountToUnsignedInt<sizeof(uint16_t) * kBitsInByte>
{
    using Type = uint16_t;
};

template <>
struct BitsCountToUnsignedInt<sizeof(uint8_t) * kBitsInByte>
{
    using Type = uint8_t;
};

template <size_t bits_count>
using BitsCountToUnsignedIntT = typename BitsCountToUnsignedInt<bits_count>::Type;
}  // namespace ass