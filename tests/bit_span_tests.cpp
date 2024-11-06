#include "ass/bit_span.hpp"
#include "gtest/gtest.h"

namespace ass
{
static_assert(sizeof(BitSpan<size_t, {.parts_count = 10, .size = 10}>) == sizeof(void*));
static_assert(sizeof(BitSpan<size_t, {.parts_count = 10}>) == sizeof(size_t) + sizeof(void*));
static_assert(sizeof(BitSpan<size_t, {.size = 10}>) == sizeof(size_t) + sizeof(void*));
static_assert(sizeof(BitSpan<size_t>) == sizeof(size_t) * 2 + sizeof(void*));

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

TEST(BitSpanTest, CountOnes_FullStatic_)
{
    uint8_t bs = 0;
    BitSpan<const uint8_t> bit_span{&bs, {.parts_count = 1, .size = 5}};
    ASSERT_EQ(bit_span.GetSize(), 5);
    ASSERT_EQ(bit_span.GetPartsCount(), 1);
    ASSERT_EQ(bit_span.GetCapacity(), 8);
    ASSERT_EQ(bit_span.CountOnes(), 0);

    bs |= 0b0000'0010;
    ASSERT_EQ(bit_span.CountOnes(), 1);

    bs |= 0b0010'1010;
    ASSERT_EQ(bit_span.CountOnes(), 2);
}
}  // namespace ass
