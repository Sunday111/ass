#include <array>
#include <iostream>

#include "ass/bit_span.hpp"
#include "gtest/gtest.h"

namespace ass
{
static_assert(sizeof(BitSpan<size_t, {.parts_count = 10, .size = 10}>) == sizeof(void*));
static_assert(sizeof(BitSpan<size_t, {.parts_count = 10}>) == sizeof(size_t) + sizeof(void*));
static_assert(sizeof(BitSpan<size_t, {.size = 10}>) == sizeof(size_t) + sizeof(void*));
static_assert(sizeof(BitSpan<size_t>) == sizeof(size_t) * 2 + sizeof(void*));

using AllPartTypes = std::tuple<uint8_t, uint16_t, uint32_t, uint64_t>;

template <size_t v>
using SizeTConstant = std::integral_constant<size_t, v>;

template <size_t... values>
using SizeTSeq = std::integer_sequence<size_t, values...>;

template <typename Part, BitSpanStaticExtents static_extents>
constexpr auto MakeBitSpan(Part* parts, size_t parts_count, size_t size)
{
    if constexpr (static_extents.size == std::dynamic_extent && static_extents.parts_count == std::dynamic_extent)
    {
        return BitSpan<Part, static_extents>(parts, {.parts_count = parts_count, .size = size});
    }

    else if constexpr (static_extents.size != std::dynamic_extent && static_extents.parts_count == std::dynamic_extent)
    {
        return BitSpan<Part, static_extents>(parts, {.parts_count = parts_count});
    }

    else if constexpr (static_extents.size == std::dynamic_extent && static_extents.parts_count != std::dynamic_extent)
    {
        return BitSpan<Part, static_extents>(parts, {.size = size});
    }
    else
    {
        return BitSpan<Part, static_extents>(parts);
    }
}

template <size_t offset, typename S>
struct ShiftSequenceT
{
};

template <size_t offset, size_t... values>
struct ShiftSequenceT<offset, SizeTSeq<values...>>
{
    using Result = SizeTSeq<values + offset...>;
};

template <typename S, size_t offset>
using ShiftSequence = typename ShiftSequenceT<offset, S>::Result;

template <typename A, typename B>
struct JoinSequenceT
{
};

template <size_t... A, size_t... B>
struct JoinSequenceT<SizeTSeq<A...>, SizeTSeq<B...>>
{
    using Result = SizeTSeq<A..., B...>;
};

template <typename A, typename B>
using JoinSequence = typename JoinSequenceT<A, B>::Result;

template <typename Part, size_t parts_count, bool top = true>
[[nodiscard]] constexpr auto MakeSequenceForPartsCount()
{
    if constexpr (parts_count == std::dynamic_extent)
    {
        return MakeSequenceForPartsCount<Part, 3>();
    }
    else if constexpr (parts_count == 0)
    {
        if constexpr (top)
        {
            return SizeTSeq<0>{};
        }
        else
        {
            return SizeTSeq<>{};
        }
    }
    else
    {
        constexpr size_t nbits = sizeof(Part) * 8;
        using Base = JoinSequence<
            SizeTSeq<0, 1, nbits / 2, nbits - 2, nbits - 1>,
            ShiftSequence<decltype(MakeSequenceForPartsCount<Part, parts_count - 1, false>()), nbits>>;

        if constexpr (top)
        {
            return JoinSequence<Base, SizeTSeq<nbits * parts_count>>{};
        }
        else
        {
            return Base{};
        }
    }
}

static_assert(std::same_as<decltype(MakeSequenceForPartsCount<uint8_t, 0>()), SizeTSeq<0>>);
static_assert(std::same_as<decltype(MakeSequenceForPartsCount<uint8_t, 1>()), SizeTSeq<0, 1, 4, 6, 7, 8>>);
static_assert(
    std::same_as<decltype(MakeSequenceForPartsCount<uint8_t, 2>()), SizeTSeq<0, 1, 4, 6, 7, 8, 9, 12, 14, 15, 16>>);
static_assert(std::same_as<
              decltype(MakeSequenceForPartsCount<uint8_t, 3>()),
              SizeTSeq<0, 1, 4, 6, 7, 8, 9, 12, 14, 15, 16, 17, 20, 22, 23, 24>>);
static_assert(std::same_as<
              decltype(MakeSequenceForPartsCount<uint8_t, 3>()),
              decltype(MakeSequenceForPartsCount<uint8_t, std::dynamic_extent>())>);

template <typename F>
void ParametrizeBitSpanTest(F&& callable)  // NOLINT
{
    auto with_runtime_extents = [&]<typename PartType, size_t static_parts_count, size_t static_size>()
    {
        constexpr bool is_runtime_parts_count = static_parts_count == std::dynamic_extent;
        constexpr bool is_runtime_size = static_size == std::dynamic_extent;
        constexpr BitSpanStaticExtents static_extents{.parts_count = static_parts_count, .size = static_size};

        constexpr size_t part_bits_count = sizeof(PartType) * 8;
        if constexpr (is_runtime_parts_count && is_runtime_size)
        {
            size_t start_parts_count = 0;
            for (size_t parts_count = start_parts_count; parts_count != start_parts_count + 3; ++parts_count)
            {
                for (size_t size = 0; size != part_bits_count * parts_count + 1; ++size)
                {
                    callable.template operator()<PartType, static_extents>(parts_count, size);
                }
            }
        }

        else if constexpr (is_runtime_parts_count && !is_runtime_size)
        {
            size_t start_parts_count = (static_size / part_bits_count) + (static_size % part_bits_count ? 1 : 0);
            for (size_t parts_count = start_parts_count; parts_count != start_parts_count + 3; ++parts_count)
            {
                callable.template operator()<PartType, static_extents>(parts_count, 0);
            }
        }

        else if constexpr (!is_runtime_parts_count && is_runtime_size)
        {
            for (size_t size = 0; size != part_bits_count * static_parts_count + 1; ++size)
            {
                callable.template operator()<PartType, static_extents>(0, size);
            }
        }

        else
        {
            static_assert(!is_runtime_parts_count && !is_runtime_size);
            callable.template operator()<PartType, static_extents>(0, 0);
        }
    };

    auto with_size = [&]<typename PartType, size_t parts_count, size_t... size>(const SizeTSeq<size...>&)
    {
        ((with_runtime_extents.template operator()<PartType, parts_count, size>()), ...);
    };

    auto with_parts_count = [&]<typename PartType, size_t... parts_count>(const SizeTSeq<parts_count...>&)
    {
        auto parts_count_to_max_bits = []<size_t n>(std::integer_sequence<size_t, n>)
        {
            // if parts count is dynamic value - try all variants of size value but at most for 3 parts
            size_t r = sizeof(PartType) * 8;
            r *= (n == std::dynamic_extent ? 3 : n);
            return r;
        };

        ((with_size.template operator()<PartType, parts_count>(MakeSequenceForPartsCount<PartType, parts_count>())),
         ...);
        ((with_size.template operator()<PartType, parts_count>(std::integer_sequence<size_t, std::dynamic_extent>())),
         ...);
    };

    [&]<typename... PartType>(const std::tuple<PartType...>&)
    {
        (with_parts_count.template operator()<PartType>(std::make_integer_sequence<size_t, 4>()), ...);
        (with_parts_count.template operator()<PartType>(SizeTSeq<std::dynamic_extent>{}), ...);
    }(AllPartTypes{});
}

TEST(BitSpanTest, StaticallySizeCtor)
{
    size_t bs{};
    BitSpan<size_t, {.parts_count = 1, .size = 5}> bit_span{&bs};
    ASSERT_EQ(bit_span.GetSize(), 5);
    ASSERT_EQ(bit_span.GetPartsCount(), 1);
    ASSERT_EQ(bit_span.GetCapacity(), 64);
}

TEST(BitSpanTest, DynamicPartsCountCtor)
{
    size_t bs{};
    BitSpan<size_t, {.size = 5}> bit_span{&bs, {.parts_count = 1}};
    ASSERT_EQ(bit_span.GetSize(), 5);
    ASSERT_EQ(bit_span.GetPartsCount(), 1);
    ASSERT_EQ(bit_span.GetCapacity(), 64);
}

TEST(BitSpanTest, DynamicSizeCtor)
{
    size_t bs{};
    BitSpan<size_t, {.parts_count = 1}> bit_span{&bs, {.size = 5}};
    ASSERT_EQ(bit_span.GetSize(), 5);
    ASSERT_EQ(bit_span.GetPartsCount(), 1);
    ASSERT_EQ(bit_span.GetCapacity(), 64);
}

TEST(BitSpanTest, DynamicPartsCountAndSizeCtor)
{
    size_t bs{};
    BitSpan<size_t> bit_span{&bs, {.parts_count = 1, .size = 5}};
    ASSERT_EQ(bit_span.GetSize(), 5);
    ASSERT_EQ(bit_span.GetPartsCount(), 1);
    ASSERT_EQ(bit_span.GetCapacity(), 64);
}

template <typename T>
void PrintSpanInfo(const T& bit_span)
{
    std::cout << "Part size: " << bit_span.BitsPerPart() << ". ";
    std::cout << "Capacity: " << bit_span.GetCapacity() << ". ";
    std::cout << "Size: " << bit_span.GetSize() << ". ";
    std::cout << '\n';
}

template <typename Part, BitSpanStaticExtents static_extents>
[[nodiscard]] constexpr auto AdaptBufferForBitSpan(std::vector<uint8_t>& buffer, size_t dyn_parts_count)
{
    buffer.clear();
    if constexpr (static_extents.parts_count == std::dynamic_extent)
    {
        buffer.resize(dyn_parts_count * sizeof(Part), 0);
        return std::span{reinterpret_cast<Part*>(buffer.data()), dyn_parts_count};
    }
    else
    {
        buffer.resize(static_extents.parts_count * sizeof(Part), 0);
        return std::span<Part, static_extents.parts_count>{
            reinterpret_cast<Part*>(buffer.data()),  // NOLINT
            static_extents.parts_count};
    }
}

TEST(BitSpanTest, CountOnes)
{
    std::vector<uint8_t> buffer;
    ParametrizeBitSpanTest(
        [&]<typename Part, BitSpanStaticExtents static_extents>(size_t parts_count, size_t size)
        {
            auto parts_view = AdaptBufferForBitSpan<Part, static_extents>(buffer, parts_count);
            auto bit_span = MakeBitSpan<Part, static_extents>(parts_view.data(), parts_count, size);
            assert(bit_span.GetPartsCount() * sizeof(Part) <= buffer.size());
            PrintSpanInfo(bit_span);

            for (size_t i = 0; i < buffer.size() * 8; i += 2)
            {
                auto& part = buffer[i / 8];
                part |= 1 << (i % 8);
            }

            const size_t even_count = (bit_span.GetSize() + 1) / 2;
            const size_t odd_count = bit_span.GetSize() / 2;

            ASSERT_EQ(bit_span.CountOnes(), even_count);

            for (auto& v : buffer)
            {
                v = ~v;
            }

            ASSERT_EQ(bit_span.CountOnes(), odd_count);
            for (size_t i = 0; i != bit_span.GetSize(); ++i)
            {
                bool bit_value = bit_span.Get(i);
                bool is_odd = (i % 2) != 0;
                ASSERT_EQ(bit_value, is_odd);
            }
        });
}

TEST(BitSpanTest, Flip)
{
    std::array<uint8_t, 2> data{0b0101'0101, 0b0101'0101};
    auto bit_span = BitSpan<uint8_t>(data.data(), {.parts_count = 2, .size = 4});
    bit_span.Flip();
    ASSERT_EQ(data[0], 0b0101'1010);
    ASSERT_EQ(data[1], 0b0101'0101);

    std::vector<uint8_t> buffer;
    ParametrizeBitSpanTest(
        [&]<typename Part, BitSpanStaticExtents static_extents>(size_t parts_count, size_t size)
        {
            auto parts_view = AdaptBufferForBitSpan<Part, static_extents>(buffer, parts_count);
            auto bit_span = MakeBitSpan<Part, static_extents>(parts_view.data(), parts_count, size);
            assert(bit_span.GetPartsCount() * sizeof(Part) <= buffer.size());
            PrintSpanInfo(bit_span);

            ASSERT_EQ(bit_span.CountOnes(), 0);
            bit_span.Flip();
            ASSERT_EQ(bit_span.CountOnes(), bit_span.GetSize());

            for (size_t i = 0; i != buffer.size() * 8; ++i)
            {
                auto& part = buffer[i / 8];
                bool bit_value = part & (1 << (i % 8));
                if (i < bit_span.GetSize())
                {
                    ASSERT_TRUE(bit_value);
                }
                else
                {
                    ASSERT_FALSE(bit_value);
                }
            }
        });
}

TEST(BitSpanTest, ToBitSpan)
{
    static constexpr std::array<uint8_t, 2> data{0b0011'1000, 0b1000'1110};
    constexpr auto static_extent_span = std::span{data};
    constexpr auto dynamic_extent_span = std::span<const uint8_t>{data.data(), data.size()};

    // from std::array and static size
    {
        constexpr auto bit_span = ToBitSpan<{.size = 16}>(data);
        static_assert(bit_span.HasStaticCapacity());
        static_assert(bit_span.HasStaticSize());
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
    }

    // from std::array and dynamic size
    {
        constexpr auto bit_span = ToBitSpan(data, {.size = 16});
        static_assert(bit_span.HasStaticCapacity());
        static_assert(!bit_span.HasStaticSize());
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
    }

    // static extent of parts span and static size value
    {
        constexpr auto bit_span = ToBitSpan<{.size = 16}>(static_extent_span);
        static_assert(bit_span.HasStaticCapacity());
        static_assert(bit_span.HasStaticSize());
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
    }

    // static extent of parts span and dynamic size value
    {
        constexpr auto bit_span = ToBitSpan(static_extent_span, {.size = 16});
        static_assert(bit_span.HasStaticCapacity());
        static_assert(!bit_span.HasStaticSize());
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
    }

    // dynamic extent of parts span and static size value
    {
        constexpr auto bit_span = ToBitSpan<{.size = 16}>(dynamic_extent_span);
        static_assert(!bit_span.HasStaticCapacity());
        static_assert(bit_span.HasStaticSize());
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
    }

    // dynamic extent of parts span and dynamic size value
    {
        constexpr auto bit_span = ToBitSpan(dynamic_extent_span, {.size = 16});
        static_assert(!bit_span.HasStaticCapacity());
        static_assert(!bit_span.HasStaticSize());
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
    }
}
}  // namespace ass
