#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "ass/bit/bit_count_to_type.hpp"

namespace detail
{
inline constexpr auto MakeBitCountTable()
{
    constexpr size_t table_size = 256;
    std::array<uint8_t, table_size> t{};
    t[0] = 0;

    auto count_ones_in_byte = [](uint8_t v) -> uint8_t
    {
        uint8_t n = 0;
        while (v != 0)
        {
            if (v & 1) ++n;
            v >>= 1;
        }

        return n;
    };

    for (size_t i = 1; i != table_size; ++i)
    {
        t[i] = count_ones_in_byte(static_cast<uint8_t>(i));
    }

    return t;
}
struct BitCountTableStorage
{
    static constexpr auto kTable = MakeBitCountTable();
};
}  // namespace detail

namespace ass
{

inline constexpr size_t CountBits(uint8_t v)
{
    return detail::BitCountTableStorage::kTable[v];
}

template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
inline constexpr size_t CountBits(T v)
{
    constexpr size_t kBitsCount = sizeof(v) * 8;
    constexpr size_t kHalfBitsCount = kBitsCount / 2;
    using HalfType = BitsCountToUnsignedIntT<kHalfBitsCount>;
    return CountBits(static_cast<HalfType>(v)) + CountBits(static_cast<HalfType>(v >> kHalfBitsCount));
}

}  // namespace ass