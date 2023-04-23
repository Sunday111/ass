#include <bitset>
#include <random>

#include "ass/fixed_bitset.hpp"
#include "gtest/gtest.h"

namespace ass
{
TEST(FixedBitsetTest, GetAndSet)
{
    constexpr size_t bitset_capacity = 404;
    constexpr unsigned seed = 2042;
    constexpr size_t iterations_count = 10'000;

    FixedBitset<bitset_capacity> fbs;
    std::bitset<bitset_capacity> sbs;

    std::mt19937 gen(seed);
    std::uniform_int_distribution<size_t> index_distribution(0, bitset_capacity - 1);

    // Compares standard and library bitset values and returns tur if they are the same
    auto same = [&]()
    {
        for (size_t index = 0; index != bitset_capacity; ++index)
        {
            bool a = fbs.Get(index);
            bool b = sbs[index];
            if (a != b)
            {
                return false;
            }
        }
        return true;
    };

    for (size_t iteration_index = 0; iteration_index != iterations_count; ++iteration_index)
    {
        size_t index = index_distribution(gen);
        const bool value = sbs[index] ? false : true;
        fbs.Set(index, value);
        sbs.set(index, value);
        ASSERT_TRUE(same()) << "iteration: " << iteration_index;
    }
}

TEST(FixedBitsetTest, CountContinuousZeroBits)
{
    constexpr size_t bitset_capacity = 404;
    constexpr unsigned seed = 2'042;
    constexpr size_t max_bits = 202;
    constexpr size_t iterations_count = 10'000;

    std::mt19937 gen(seed);
    std::uniform_int_distribution<size_t> index_dirstribution(0, bitset_capacity - 1);

    for (size_t i = 0; i != iterations_count; ++i)
    {
        FixedBitset<bitset_capacity> fbs;
        size_t expected = bitset_capacity;
        for (size_t j = 0; j != max_bits; ++j)
        {
            size_t bitset_index = index_dirstribution(gen);
            fbs.Set(bitset_index, true);
            expected = std::min(expected, bitset_index);
        }

        ASSERT_EQ(expected, fbs.CountContinuousZeroBits());
    }

    for (size_t i = 0; i != bitset_capacity; ++i)
    {
        FixedBitset<bitset_capacity> fbs;
        fbs.Set(i, true);
        ASSERT_EQ(fbs.CountContinuousZeroBits(), i);
    }
}

TEST(FixedBitsetTest, CountContinuousZeroBitsOverload)
{
    constexpr size_t bitset_capacity = 404;
    constexpr unsigned seed = 2'042;
    constexpr size_t max_bits = 202;
    constexpr size_t iterations_count = 10'000;

    std::mt19937 gen(seed);
    std::uniform_int_distribution<size_t> index_dirstribution(0, bitset_capacity - 1);
    std::uniform_int_distribution<size_t> ignored_bits_distribution(1, bitset_capacity - 1);

    for (size_t i = 0; i != iterations_count; ++i)
    {
        const size_t ignored_bits = ignored_bits_distribution(gen);

        FixedBitset<bitset_capacity> fbs;
        size_t expected = bitset_capacity;
        for (size_t j = 0; j != max_bits; ++j)
        {
            size_t bitset_index = index_dirstribution(gen);
            fbs.Set(bitset_index, true);
            if (bitset_index >= ignored_bits)
            {
                expected = std::min(expected, bitset_index);
            }
        }

        ASSERT_EQ(expected, fbs.CountContinuousZeroBits(ignored_bits)) << "iteration: " << i;
    }

    for (size_t i = 0; i != bitset_capacity; ++i)
    {
        FixedBitset<bitset_capacity> fbs;
        fbs.Set(i, true);
        ASSERT_EQ(fbs.CountContinuousZeroBits(), i);
    }
}
}  // namespace ass