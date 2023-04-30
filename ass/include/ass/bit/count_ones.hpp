#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "../detail/bit/count_ones_detail.hpp"
#include "bit_count_to_type.hpp"

namespace ass
{

inline constexpr size_t CountBits(uint8_t v)
{
    return count_ones_detail::BitCountTableStorage::kTable[v];
}

template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
inline constexpr size_t CountBits(T v)
{
    // Split the bitset into two parts and count bits in these parts recursively
    // util we come to uint8_t, which has it's own table
    constexpr size_t kBitsCount = sizeof(v) * 8;
    constexpr size_t kHalfBitsCount = kBitsCount / 2;
    using HalfType = BitsCountToUnsignedIntT<kHalfBitsCount>;
    return CountBits(static_cast<HalfType>(v)) + CountBits(static_cast<HalfType>(v >> kHalfBitsCount));
}

}  // namespace ass
