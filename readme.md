# Advanced and Standard Structures

![Badge](https://github.com/sunday111/ass/actions/workflows/linux.yml/badge.svg)

Small header-only library with fixed data structures and utilities. Uses C++ 17 for convenience and depends on STL only.

In a few words
- `ass::FixedUnorderedMap` - line `std::unordered_map` but is allocated on the stack and may be used in constexpr context (if both key and value types are trivially destructible).
- `ass::FixedBitset` - literally same as `std::bitset` but can be used in constexpr context.
- [`ass::FixedUnorderedMap`](doc/fixed_unordered_map.md) - fixed unordered map.
- [`ass::EnumSet`](doc/enum_set.md) - fixed set of enumeration values.
- [`ass::EnumMap`](doc/enum_map.md) - fixed map with enum keys.
