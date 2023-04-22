#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace ass::count_ones_detail
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
}  // namespace ass::count_ones_detail
