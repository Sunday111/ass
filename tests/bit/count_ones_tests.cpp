#include <gtest/gtest.h>

#include <bitset>
#include <ios>
#include <limits>
#include <random>

#include "ass/bit/count_ones.hpp"

namespace ass
{

template <typename T>
class CountOnesTest : public testing::Test
{
public:
    using IntegerType = T;
};

using Implementations = testing::Types<uint8_t, uint16_t, uint32_t, uint64_t>;
class CountOnesTypesNames
{
public:
    template <typename T>
    static std::string GetName(int)
    {
        if (std::is_same<T, uint8_t>()) return "uint8_t";
        if (std::is_same<T, uint16_t>()) return "uint16_t";
        if (std::is_same<T, uint32_t>()) return "uint32_t";
        if (std::is_same<T, uint64_t>()) return "uint64_t";
        return "";
    }
};

TYPED_TEST_SUITE(CountOnesTest, Implementations, CountOnesTypesNames);

TYPED_TEST(CountOnesTest, Full)
{
    using IntegerType = typename std::decay_t<decltype(*this)>::IntegerType;

    std::random_device random_device;
    const unsigned seed = random_device();
    std::mt19937_64 generator(seed);
    std::uniform_int_distribution<uint64_t> distribution(
        std::numeric_limits<IntegerType>::lowest(),
        std::numeric_limits<IntegerType>::max());

    for (size_t i = 0; i != 1000000; ++i)
    {
        const IntegerType value = static_cast<IntegerType>(distribution(generator));
        std::bitset<sizeof(IntegerType) * 8> bs(value);
        ASSERT_EQ(bs.count(), CountBits(value)) << "For value: " << std::hex << value << "(" << bs << ")";
    }
}
}  // namespace ass
