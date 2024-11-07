#pragma once

#include <bit>
#include <cassert>
#include <span>

#include "macro/empty_bases.hpp"

namespace ass::bit_span_detail
{

template <size_t value>
struct StaticSizeContainer
{
};

struct DynamicSizeContainer
{
    constexpr DynamicSizeContainer(size_t size) noexcept : size_(size) {}

    size_t size_ = 0;
};

template <size_t value>
using SizeContainer =
    std::conditional_t<value == std::dynamic_extent, DynamicSizeContainer, StaticSizeContainer<value>>;

template <size_t value>
struct StaticPartsCountContainer
{
};

struct DynamicPartsCountContainer
{
    constexpr DynamicPartsCountContainer(size_t parts_count) noexcept : parts_count_(parts_count) {}

    size_t parts_count_ = 0;
};

template <size_t value>
using PartsCountContainer =
    std::conditional_t<value == std::dynamic_extent, DynamicPartsCountContainer, StaticPartsCountContainer<value>>;

}  // namespace ass::bit_span_detail

namespace ass
{
struct BitSpanStaticExtents
{
    size_t parts_count = std::dynamic_extent;
    size_t size = std::dynamic_extent;

    [[nodiscard]] constexpr bool AllStatic() const
    {
        return parts_count != std::dynamic_extent && size != std::dynamic_extent;
    }
};

struct BitSpanSize
{
    size_t size = 0;
};

struct BitSpanPartsCount
{
    size_t parts_count = 0;
};

struct BitSpanPartsCountAndSize
{
    size_t parts_count = 0;
    size_t size = 0;
};

template <std::integral Part, BitSpanStaticExtents static_extents = BitSpanStaticExtents{}>
    requires(static_extents.size == std::dynamic_extent || static_extents.parts_count == std::dynamic_extent ||
             static_extents.size <= (8 * static_extents.parts_count * sizeof(Part)))
class ASS_EMPTY_BASES BitSpan : public bit_span_detail::SizeContainer<static_extents.size>,
                                public bit_span_detail::PartsCountContainer<static_extents.parts_count>
{
public:
    [[nodiscard]] static constexpr size_t BitsPerPart()
    {
        return sizeof(Part) * 8;
    }

    [[nodiscard]] static constexpr bool HasStaticSize()
    {
        return static_extents.size != std::dynamic_extent;
    }

    [[nodiscard]] static constexpr size_t GetSize() noexcept
        requires(HasStaticSize())
    {
        return static_extents.size;
    }

    [[nodiscard]] constexpr size_t GetSize() const noexcept
        requires(!HasStaticSize())
    {
        return this->size_;
    }

    [[nodiscard]] static constexpr bool HasStaticCapacity()
    {
        return static_extents.parts_count != std::dynamic_extent;
    }

    [[nodiscard]] static constexpr size_t GetPartsCount() noexcept
        requires(HasStaticCapacity())
    {
        return static_extents.parts_count;
    }

    [[nodiscard]] constexpr size_t GetPartsCount() const noexcept
        requires(!HasStaticCapacity())
    {
        return this->parts_count_;
    }

    [[nodiscard]] static constexpr size_t GetCapacity()
        requires(HasStaticCapacity())
    {
        return GetPartsCount() * BitsPerPart();
    }

    [[nodiscard]] constexpr size_t GetCapacity() const noexcept
        requires(!HasStaticCapacity())
    {
        return GetPartsCount() * BitsPerPart();
    }

    constexpr BitSpan(Part* parts)
        requires(HasStaticCapacity() && HasStaticSize())
        : parts_(parts)
    {
    }

    constexpr BitSpan(Part* parts, BitSpanSize size)
        requires(HasStaticCapacity() && !HasStaticSize())
        : bit_span_detail::SizeContainer<static_extents.size>(size.size),
          parts_(parts)
    {
    }

    constexpr BitSpan(Part* parts, BitSpanPartsCount parts_count)
        requires(!HasStaticCapacity() && HasStaticSize())
        : bit_span_detail::PartsCountContainer<static_extents.parts_count>(parts_count.parts_count),
          parts_(parts)
    {
    }

    constexpr BitSpan(Part* parts, BitSpanPartsCountAndSize parts_count_and_size)
        requires(!HasStaticCapacity() && !HasStaticSize())
        : bit_span_detail::SizeContainer<static_extents.size>(parts_count_and_size.size),
          bit_span_detail::PartsCountContainer<static_extents.parts_count>(parts_count_and_size.parts_count),
          parts_(parts)
    {
    }

    [[nodiscard]] static constexpr size_t GetUnusedBitsCount()
        requires(HasStaticSize() && HasStaticCapacity())
    {
        return GetCapacity() - GetSize();
    }

    [[nodiscard]] constexpr size_t GetUnusedBitsCount() const
        requires(!HasStaticSize() && !HasStaticCapacity())
    {
        return GetCapacity() - GetSize();
    }

    [[nodiscard]] constexpr size_t CountOnes() const
        requires(HasStaticSize())
    {
        if constexpr (GetSize() == 0)
        {
            return 0;
        }
        else
        {
            return CountOnesNonEmpty();
        }
    }

    [[nodiscard]] constexpr size_t CountOnes() const
        requires(!HasStaticSize())
    {
        if (GetSize() == 0)
        {
            return 0;
        }

        return CountOnesNonEmpty();
    }

    [[nodiscard]] bool Get(size_t index) const
    {
        assert(index < GetSize());
        const PurePart mask = PurePart{1} << (index % BitsPerPart());
        return parts_[index / BitsPerPart()] & mask;  // NOLINT
    }

    void Set(size_t index, bool value) const
        requires(!std::is_const_v<Part>)
    {
        assert(index < GetSize());
        const Part mask = PurePart{1} << (index % BitsPerPart());
        Part& part = parts_[index / BitsPerPart()];  // NOLINT

        if (value)
        {
            part |= mask;
        }
        else
        {
            part &= ~mask;
        }
    }

private:
    using PurePart = std::remove_const_t<Part>;

    [[nodiscard]] constexpr size_t CountOnesNonEmpty() const
    {
        const size_t last_used_part_index = GetLastUsedPartIndex();

        size_t n = 0;
        for (size_t part_index = 0; part_index != last_used_part_index; ++part_index)
        {
            n += std::popcount(parts_[part_index]);  // NOLINT
        }

        PurePart part = parts_[last_used_part_index];  // NOLINT

        // Have to mask out unused bits (if any)
        const size_t used_bits_in_last_part = GetUsedBitsCountInLastUsedPart();
        if (used_bits_in_last_part != 0)
        {
            PurePart mask{};
            mask = ~mask;
            mask >>= BitsPerPart() - used_bits_in_last_part;
            part &= mask;
        }

        n += std::popcount(part);

        return n;
    }

    [[nodiscard]] static constexpr size_t GetLastUsedPartIndex() noexcept
        requires(HasStaticSize())
    {
        return (GetSize() - 1) / BitsPerPart();
    }

    [[nodiscard]] constexpr size_t GetLastUsedPartIndex() const noexcept
        requires(!HasStaticSize())
    {
        return (GetSize() - 1) / BitsPerPart();
    }

    [[nodiscard]] static constexpr size_t GetUsedBitsCountInLastUsedPart() noexcept
        requires(HasStaticSize())
    {
        return GetSize() % BitsPerPart();
    }

    [[nodiscard]] constexpr size_t GetUsedBitsCountInLastUsedPart() const noexcept
        requires(!HasStaticSize())
    {
        return GetSize() % BitsPerPart();
    }

private:
    Part* parts_ = nullptr;
};

}  // namespace ass
