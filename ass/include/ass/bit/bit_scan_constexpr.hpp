#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "bit_count_to_type.hpp"

namespace ass::bit_scan_constexpr_detail
{
inline constexpr auto kBitScanTable = []()
{
    std::array<uint8_t, 256> table{};

    auto bit_scan_slow = [](uint8_t value)
    {
        uint8_t index = 0;
        constexpr size_t bits_in_byte = 8;
        while (!(value & (1 << index)) && index != bits_in_byte)
        {
            ++index;
        }
        return index;
    };

    uint8_t value = 0;
    do
    {
        table[value] = bit_scan_slow(value);
        value += 1;
    } while (value != std::numeric_limits<uint8_t>::max());

    return table;
}();
}  // namespace ass::bit_scan_constexpr_detail

namespace ass
{

inline constexpr size_t BitScanConstexpr(const uint8_t v)
{
    return bit_scan_constexpr_detail::kBitScanTable[v];
}

// Returns an index of first non-zero bit
// Also may be interpreted as number of leading zero bits
template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
inline constexpr size_t BitScanConstexpr(const T v)
{
    // Split the bitset into two parts and count bits in these parts recursively
    // util we come to uint8_t, which has it's own table
    constexpr size_t kBitsCount = sizeof(v) * 8;
    constexpr size_t kHalfBitsCount = kBitsCount / 2;
    using HalfType = BitsCountToUnsignedIntT<kHalfBitsCount>;

    if (const auto lo = static_cast<HalfType>(v); lo != 0)
    {
        return BitScanConstexpr(lo);
    }

    if (const auto hi = static_cast<HalfType>(v >> kHalfBitsCount); hi != 0)
    {
        return BitScanConstexpr(hi) + kHalfBitsCount;
    }

    return kBitsCount;
}
}  // namespace ass
