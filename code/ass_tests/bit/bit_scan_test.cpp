#include "ass/bit/bit_scan_constexpr.hpp"

static_assert(ass::BitScanConstexpr(0b11111111) == 0);
static_assert(ass::BitScanConstexpr(0b11111100) == 2);
static_assert(ass::BitScanConstexpr(0b00000000) == 8);
static_assert(ass::BitScanConstexpr(static_cast<uint16_t>(0b00000000'00000000)) == 16u);
static_assert(ass::BitScanConstexpr(static_cast<uint16_t>(0b00000000'10000000)) == 7u);
static_assert(ass::BitScanConstexpr(static_cast<uint16_t>(0b00000001'00000000)) == 8u);
static_assert(ass::BitScanConstexpr(static_cast<uint16_t>(0b10000000'00000000)) == 15u);
