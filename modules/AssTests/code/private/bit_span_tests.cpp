#include <algorithm>
#include <array>
#include <functional>
#include <vector>

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
        [[maybe_unused]] auto parts_count_to_max_bits = []<size_t n>(std::integer_sequence<size_t, n>)
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
                v = static_cast<uint8_t>(~v);
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
            auto span = MakeBitSpan<Part, static_extents>(parts_view.data(), parts_count, size);
            assert(span.GetPartsCount() * sizeof(Part) <= buffer.size());

            ASSERT_EQ(span.CountOnes(), 0);
            span.Flip();
            ASSERT_EQ(span.CountOnes(), span.GetSize());

            for (size_t i = 0; i != buffer.size() * 8; ++i)
            {
                auto& part = buffer[i / 8];
                bool bit_value = part & (1 << (i % 8));
                if (i < span.GetSize())
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
    std::array<uint8_t, 2> non_const_data{0b0011'1000, 0b1000'1110};
    static constexpr std::array<uint8_t, 2> data{0b0011'1000, 0b1000'1110};
    constexpr auto static_extent_span = std::span{data};
    constexpr auto dynamic_extent_span = std::span<const uint8_t>{data.data(), data.size()};

    auto test_bits = [&](const auto& bit_span) -> bool
    {
        for (size_t i = 0; i != bit_span.GetSize(); ++i)
        {
            bool actual = bit_span.Get(i);
            bool expected = (data[i / 8] & (1 << (i % 8))) != 0;
            if (actual != expected) return false;
        }

        return true;
    };

    // from const std::array and static size
    {
        constexpr auto bit_span = ToBitSpan<{.size = 16}>(data);
        static_assert(bit_span.HasStaticCapacity());
        static_assert(bit_span.HasStaticSize());
        static_assert(!bit_span.kCanModifyData);
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
        static_assert(test_bits(bit_span));
    }

    // from non const std::array and static size
    {
        const auto bit_span = ToBitSpan<{.size = 16}>(non_const_data);
        static_assert(bit_span.HasStaticCapacity());
        static_assert(bit_span.HasStaticSize());
        static_assert(bit_span.kCanModifyData);
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
    }

    // from const std::array and dynamic size
    {
        constexpr auto bit_span = ToBitSpan(data, {.size = 16});
        static_assert(bit_span.HasStaticCapacity());
        static_assert(!bit_span.HasStaticSize());
        static_assert(!bit_span.kCanModifyData);
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
        static_assert(test_bits(bit_span));
    }

    // from non-const std::array and dynamic size
    {
        const auto bit_span = ToBitSpan(non_const_data, {.size = 16});
        static_assert(bit_span.HasStaticCapacity());
        static_assert(!bit_span.HasStaticSize());
        static_assert(bit_span.kCanModifyData);
        ASSERT_EQ(bit_span.GetSize(), 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
        ASSERT_TRUE(test_bits(bit_span));
    }

    // static extent of parts span and static size value
    {
        constexpr auto bit_span = ToBitSpan<{.size = 16}>(static_extent_span);
        static_assert(bit_span.HasStaticCapacity());
        static_assert(bit_span.HasStaticSize());
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
        static_assert(test_bits(bit_span));
    }

    // static extent of parts span and dynamic size value
    {
        constexpr auto bit_span = ToBitSpan(static_extent_span, {.size = 16});
        static_assert(bit_span.HasStaticCapacity());
        static_assert(!bit_span.HasStaticSize());
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
        static_assert(test_bits(bit_span));
    }

    // dynamic extent of parts span and static size value
    {
        constexpr auto bit_span = ToBitSpan<{.size = 16}>(dynamic_extent_span);
        static_assert(!bit_span.HasStaticCapacity());
        static_assert(bit_span.HasStaticSize());
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
        static_assert(test_bits(bit_span));
    }

    // dynamic extent of parts span and dynamic size value
    {
        constexpr auto bit_span = ToBitSpan(dynamic_extent_span, {.size = 16});
        static_assert(!bit_span.HasStaticCapacity());
        static_assert(!bit_span.HasStaticSize());
        static_assert(bit_span.GetSize() == 16);
        static_assert(bit_span.GetPartsCount() == 2);
        static_assert(bit_span.GetCapacity() == 16);
        static_assert(test_bits(bit_span));
    }
}

TEST(BitSpanTest, VectorToBitSpan)
{
    std::vector<uint8_t> data{0b0011'1000, 0b1000'1110};
    [[maybe_unused]] const auto& const_data = data;

    auto test_bits = [&](const auto& bit_span) -> bool
    {
        for (size_t i = 0; i != bit_span.GetSize(); ++i)
        {
            bool actual = bit_span.Get(i);
            bool expected = (data[i / 8] & (1 << (i % 8))) != 0;
            if (actual != expected) return false;
        }

        return true;
    };

    // deduced (dynamic) size and capacity
    {
        [[maybe_unused]] const auto bit_span = ToBitSpan(data);
        static_assert(!bit_span.HasStaticCapacity());
        static_assert(!bit_span.HasStaticSize());
        static_assert(bit_span.kCanModifyData);
        ASSERT_EQ(bit_span.GetSize(), 16);
        ASSERT_EQ(bit_span.GetPartsCount(), 2);
        ASSERT_EQ(bit_span.GetCapacity(), 16);
        ASSERT_TRUE(test_bits(bit_span));
    }

    // static size, deduced capacity
    {
        [[maybe_unused]] const auto bit_span = ToBitSpan<{.size = 13}>(data);
        static_assert(bit_span.HasStaticCapacity());
        static_assert(bit_span.HasStaticSize());
        static_assert(bit_span.kCanModifyData);
        static_assert(bit_span.GetSize() == 13);
        ASSERT_EQ(bit_span.GetPartsCount(), 2);
        ASSERT_EQ(bit_span.GetCapacity(), 16);
        ASSERT_TRUE(test_bits(bit_span));
    }
}

TEST(BitSpanTest, BinaryOperationsAcrossWordWidths)
{
    auto make_parts = []<typename Part>(size_t count, uint64_t seed)
    {
        std::vector<Part> parts(count);
        for (size_t index = 0; index != count; ++index)
        {
            parts[index] = static_cast<Part>(seed ^ (index * 0x9E3779B97F4A7C15ULL));
        }
        return parts;
    };
    auto check = [&]<typename DestinationPart, typename SourcePart>()
    {
        constexpr size_t destination_width = sizeof(DestinationPart) * 8;
        constexpr size_t source_width = sizeof(SourcePart) * 8;
        for (size_t size : {0U, 1U, 7U, 8U, 9U, 15U, 16U, 17U, 31U, 32U, 33U, 63U, 64U, 65U, 127U, 128U, 129U})
        {
            for (size_t destination_spare : {0U, 2U})
            {
                for (size_t source_spare : {0U, 2U})
                {
                    SCOPED_TRACE(
                        ::testing::Message() << destination_width << " <- " << source_width << ", size=" << size
                                             << ", spare=" << destination_spare << '/' << source_spare);
                    auto destination = make_parts.template operator()<DestinationPart>(
                        (size + destination_width - 1) / destination_width + destination_spare,
                        0xA591D368C247BE0FULL);
                    const auto initial = destination;
                    const auto source = make_parts.template operator()<SourcePart>(
                        (size + source_width - 1) / source_width + source_spare,
                        0xC63A87D15BE924F0ULL);
                    auto check_views = [&](auto destination_span, const auto& source_span)
                    {
                        auto check_operation = [&](auto apply, auto reference)
                        {
                            std::copy(initial.begin(), initial.end(), destination.begin());
                            auto expected = initial;
                            auto expected_span = ToBitSpan(std::span(expected), {.size = size});
                            for (size_t bit = 0; bit != size; ++bit)
                            {
                                expected_span.Set(bit, reference(expected_span.Get(bit), source_span.Get(bit)));
                            }
                            apply(destination_span, source_span);
                            EXPECT_EQ(destination, expected);
                        };
                        check_operation(
                            [](auto target, const auto& operand)
                            {
                                target.AndAssign(operand);
                            },
                            std::bit_and<bool>{});
                        check_operation(
                            [](auto target, const auto& operand)
                            {
                                target.OrAssign(operand);
                            },
                            std::bit_or<bool>{});
                        check_operation(
                            [](auto target, const auto& operand)
                            {
                                target.XorAssign(operand);
                            },
                            std::bit_xor<bool>{});
                    };
                    check_views(
                        ToBitSpan(std::span(destination), {.size = size}),
                        ToBitSpan(std::span(source), {.size = size}));
                    if (size == 65)
                    {
                        auto static_destination =
                            MakeBitSpan<DestinationPart, {.size = 65}>(destination.data(), destination.size(), size);
                        auto static_source =
                            MakeBitSpan<const SourcePart, {.size = 65}>(source.data(), source.size(), size);
                        check_views(static_destination, static_source);
                        check_views(static_destination, ToBitSpan(std::span(source), {.size = size}));
                        check_views(ToBitSpan(std::span(destination), {.size = size}), static_source);
                    }
                }
            }
        }
    };
    auto check_sources = [&]<typename DestinationPart, typename... SourceParts>(const std::tuple<SourceParts...>&)
    {
        (check.template operator()<DestinationPart, SourceParts>(), ...);
    };
    [&]<typename... Parts>(const std::tuple<Parts...>&)
    {
        (check_sources.template operator()<Parts>(AllPartTypes{}), ...);
    }(AllPartTypes{});
}

TEST(BitSpanTest, BinaryOperationsWithConstSourceAndMixedExtents)
{
    std::array<uint16_t, 2> destination{0xFFFF, 0xAAAA};
    constexpr std::array<uint16_t, 2> source{0x0100, 0xFFFF};
    auto destination_span = ToBitSpan(destination, {.size = 17});
    auto source_span = ToBitSpan<{.size = 17}>(source);

    destination_span.AndAssign(source_span);
    EXPECT_EQ(destination, (std::array<uint16_t, 2>{0x0100, 0xAAAA}));
    destination_span.OrAssign(source_span);
    EXPECT_EQ(destination, (std::array<uint16_t, 2>{0x0100, 0xAAAB}));
    destination_span.XorAssign(source_span);
    EXPECT_EQ(destination, (std::array<uint16_t, 2>{0, 0xAAAA}));
}

TEST(BitSpanTest, AndAssign)
{
    std::array<uint8_t, 3> a{0b1101'1011, 0b1101'1011, 0b1101'1011};
    constexpr std::array<uint8_t, 3> b{0b1011'0110, 0b1011'0110, 0b0000'0000};

    auto bit_span_a = ToBitSpan<{.size = 17}>(a);
    auto bit_span_b = ToBitSpan<{.size = 17}>(b);
    bit_span_a.AndAssign(bit_span_b);

    ASSERT_EQ(a[0], 0b1001'0010);
    ASSERT_EQ(a[1], 0b1001'0010);
    ASSERT_EQ(a[2], 0b1101'1010);
}

TEST(BitSpanTest, OrAssign)
{
    std::array<uint8_t, 3> a{0b1101'1011, 0b1101'1011, 0b1101'1011};
    std::array<uint8_t, 3> b{0b1011'0110, 0b1011'0110, 0b0000'0000};

    auto bit_span_a = ToBitSpan<{.size = 17}>(a);
    auto bit_span_b = ToBitSpan<{.size = 17}>(b);
    bit_span_a.OrAssign(bit_span_b);

    ASSERT_EQ(a[0], 0b1111'1111);
    ASSERT_EQ(a[1], 0b1111'1111);
    ASSERT_EQ(a[2], 0b1101'1011);
}

TEST(BitSpanTest, XorAssign)
{
    std::array<uint8_t, 3> a{0b1101'1011, 0b1101'1011, 0b1101'1011};
    std::array<uint8_t, 3> b{0b1011'0110, 0b1011'0110, 0b0000'0000};

    auto bit_span_a = ToBitSpan<{.size = 17}>(a);
    auto bit_span_b = ToBitSpan<{.size = 17}>(b);
    bit_span_a.XorAssign(bit_span_b);

    ASSERT_EQ(a[0], 0b0110'1101);
    ASSERT_EQ(a[1], 0b0110'1101);
    ASSERT_EQ(a[2], 0b1101'1011);
}
}  // namespace ass
