#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "bit/count_ones.hpp"

namespace ass
{

inline constexpr size_t GetRequiredChunksCount(size_t capacity, size_t chunk_size)
{
    if (capacity == 0) return 1;

    size_t n = capacity / chunk_size;
    if (capacity % chunk_size)
    {
        ++n;
    }

    return n;
}

template <size_t Capacity>
class FixedBitset
{
public:
    constexpr FixedBitset() = default;

    constexpr bool Get(size_t index) const
    {
        const auto [part_index, bit_index] = DecomposeIndex(index);
        return (parts_[part_index] & (Part{1} << bit_index)) != 0;
    }

    constexpr void Set(size_t index, bool value)
    {
        const auto [part_index, bit_index] = DecomposeIndex(index);
        if (value)
        {
            parts_[part_index] |= (Part{1} << bit_index);
        }
        else
        {
            parts_[part_index] &= ~(Part{1} << bit_index);
        }
    }

    constexpr size_t CountOnes() const
    {
        size_t n = 0;
        for (Part p : parts_)
        {
            n += CountBits(p);
        }
        return n;
    }

private:
    static constexpr std::pair<size_t, size_t> DecomposeIndex(size_t index) noexcept
    {
        size_t part_index = index / kPartBitsSize;
        size_t bit_index = index % kPartBitsSize;
        return {part_index, bit_index};
    }

private:
    using Part = uint64_t;
    static constexpr size_t kPartBitsSize = 8 * sizeof(Part);
    static constexpr size_t kPartsCount = GetRequiredChunksCount(Capacity, kPartBitsSize);

    std::array<Part, kPartsCount> parts_{};
};
}  // namespace ass