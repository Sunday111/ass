#include <gtest/gtest.h>

#include "ass/bit/bit_count_to_type.hpp"

namespace ass
{
static_assert(std::is_same_v<BitsCountToUnsignedIntT<64>, uint64_t>);
static_assert(std::is_same_v<BitsCountToUnsignedIntT<32>, uint32_t>);
static_assert(std::is_same_v<BitsCountToUnsignedIntT<16>, uint16_t>);
static_assert(std::is_same_v<BitsCountToUnsignedIntT<8>, uint8_t>);
}  // namespace ass
