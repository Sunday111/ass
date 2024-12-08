# Advanced and Standard Structures

![Badge](https://github.com/sunday111/ass/actions/workflows/linux.yml/badge.svg)

Small header-only library with fixed data structures and utilities. Uses C++ 20 and depends on STL only. There is also a freezed branch that supports c++17: `cpp17`.

In a few words
- `ass::FixedBitset` - same as `std::bitset` but can be used in constexpr context.
- [`ass::FixedUnorderedMap`](doc/fixed_unordered_map.md) - fixed unordered map.
- [`ass::EnumSet`](doc/enum_set.md) - set of enumeration values.
- [`ass::EnumMap`](doc/enum_map.md) - fixed map with enum keys.
