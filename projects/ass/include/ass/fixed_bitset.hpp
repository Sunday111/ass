#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "bit/bit_scan_constexpr.hpp"
#include "bit/count_ones.hpp"

namespace ass::fixed_bitset_detail
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
}  // namespace ass::fixed_bitset_detail

namespace ass
{

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

    constexpr size_t CountContinuousZeroBits() const
    {
        size_t result = 0;
        for (size_t part_index = 0; part_index != parts_.size(); ++part_index)
        {
            const Part& part = parts_[part_index];
            const size_t local = BitScanConstexpr(part);
            result += local;
            if (local != kPartBitsSize)
            {
                break;
            }
        }
        return std::min(result, Capacity);
    }

    constexpr size_t CountContinuousZeroBits(const size_t ignore_first_n) const
    {
        // Skip ignored parts
        size_t part_index = ignore_first_n / kPartBitsSize;
        size_t result = part_index * kPartBitsSize;

        // Deal with remaining ignored bits
        if (ignore_first_n > result)
        {
            const size_t ignored_bits = ignore_first_n - result;
            Part mask{};
            mask = ~mask;
            mask <<= ignored_bits;
            const Part part = parts_[part_index] & mask;
            const size_t local = BitScanConstexpr(part);
            if (local != kPartBitsSize)
            {
                return result + local;
            }
            part_index += 1;
            result += kPartBitsSize;
        }

        // them do the same as in overload without argument
        for (; part_index != parts_.size(); ++part_index)
        {
            const Part& part = parts_[part_index];
            const size_t local = BitScanConstexpr(part);
            result += local;
            if (local != kPartBitsSize)
            {
                break;
            }
        }

        return std::min(result, Capacity);
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
    static constexpr size_t kPartsCount = fixed_bitset_detail::GetRequiredChunksCount(Capacity, kPartBitsSize);

    std::array<Part, kPartsCount> parts_{};
};
}  // namespace ass
