#pragma once

#include <array>
#include <cassert>
#include <cstddef>

#include "bit/bit_count_to_type.hpp"
#include "bit/bit_scan_constexpr.hpp"
#include "bit/count_ones.hpp"

namespace ass::fixed_bitset_detail
{
inline constexpr size_t GetOptimalPartSize(size_t size)
{
    if (size <= 8) return 8;
    if (size <= 16) return 16;
    if (size <= 32) return 32;
    return 64;
}
inline constexpr size_t GetRequiredPartsCount(size_t capacity, size_t part_size)
{
    if (capacity == 0) return 1;

    size_t n = capacity / part_size;
    if (capacity % part_size)
    {
        ++n;
    }

    return n;
}

}  // namespace ass::fixed_bitset_detail

namespace ass
{

template <size_t kSize>
class FixedBitset
{
public:
    static constexpr size_t kPartBitsCount = fixed_bitset_detail::GetOptimalPartSize(kSize);
    static constexpr size_t kPartsCount = fixed_bitset_detail::GetRequiredPartsCount(kSize, kPartBitsCount);
    static constexpr size_t kCapacity = kPartsCount * kPartBitsCount;
    static constexpr size_t kUnusedBitsCount = kCapacity - kSize;
    using Part = BitsCountToUnsignedIntT<kPartBitsCount>;

    constexpr FixedBitset() = default;

    constexpr bool Get(size_t index) const
    {
        const auto [part_index, bit_index] = DecomposeIndex(index);
        return (parts_[part_index] & (Part{1} << bit_index)) != 0;
    }

    // Returns true if bit at specified index was flipped
    constexpr bool Set(size_t index, bool value)
    {
        const auto [part_index, bit_index] = DecomposeIndex(index);
        Part& part = parts_[part_index];
        const Part part_prev_value = part;
        Part mask{1};
        mask <<= bit_index;
        if (value)
        {
            part |= mask;
        }
        else
        {
            part &= ~mask;
        }

        return part != part_prev_value;
    }

    constexpr size_t CountOnes() const
    {
        size_t n = 0;
        if constexpr (kUnusedBitsCount == 0)
        {
            for (Part p : parts_)
            {
                n += CountBits(p);
            }
        }
        else
        {
            // Have to mask out unused bits
            size_t last_part_index = kPartsCount - 1;
            for (size_t part_index = 0; part_index != last_part_index; ++part_index)
            {
                n += CountBits(parts_[part_index]);
            }

            Part mask{};
            mask = ~mask;
            mask >>= kUnusedBitsCount;

            Part part = parts_[last_part_index];
            part &= mask;

            n += CountBits(part);
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
            if (local != kPartBitsCount)
            {
                break;
            }
        }
        return std::min(result, kSize);
    }

    constexpr size_t CountContinuousZeroBits(const size_t ignore_first_n) const
    {
        // Skip ignored parts
        size_t part_index = ignore_first_n / kPartBitsCount;
        size_t result = part_index * kPartBitsCount;

        // Deal with remaining ignored bits
        if (ignore_first_n > result)
        {
            const size_t ignored_bits = ignore_first_n - result;
            Part mask{};
            mask = ~mask;
            mask <<= ignored_bits;
            const Part part = parts_[part_index] & mask;
            const size_t local = BitScanConstexpr(part);
            if (local != kPartBitsCount)
            {
                return result + local;
            }
            part_index += 1;
            result += kPartBitsCount;
        }

        // then do the same as in default overload without argument
        for (; part_index != parts_.size(); ++part_index)
        {
            const Part& part = parts_[part_index];
            const size_t local = BitScanConstexpr(part);
            result += local;
            if (local != kPartBitsCount)
            {
                break;
            }
        }

        return std::min(result, kSize);
    }

    constexpr FixedBitset operator~() const
    {
        auto copy = *this;
        copy.Flip();
        return copy;
    }

    constexpr void Flip()
    {
        for (Part& part : parts_) part = ~part;
    }

    constexpr FixedBitset& operator|=(const FixedBitset& another)
    {
        for (size_t index = 0; index != kPartsCount; ++index)
        {
            parts_[index] |= another.parts_[index];
        }
        return *this;
    }

    constexpr FixedBitset operator|(const FixedBitset& another) const
    {
        auto copy = *this;
        copy |= another;
        return copy;
    }

    constexpr FixedBitset& operator&=(const FixedBitset& another)
    {
        for (size_t index = 0; index != kPartsCount; ++index)
        {
            parts_[index] &= another.parts_[index];
        }
        return *this;
    }

    constexpr FixedBitset operator&(const FixedBitset& another) const
    {
        auto copy = *this;
        copy &= another;
        return copy;
    }

    constexpr const Part& GetPart(const size_t index) const
    {
        return parts_[index];
    }

    constexpr void Fill(bool value)
    {
        parts_.fill(std::numeric_limits<Part>::max());
    }

private:
    static constexpr std::pair<size_t, size_t> DecomposeIndex(size_t index) noexcept
    {
        const size_t part_index = index / kPartBitsCount;
        const size_t bit_index = index % kPartBitsCount;
        return {part_index, bit_index};
    }

private:
    std::array<Part, kPartsCount> parts_{};
};
}  // namespace ass
