#pragma once

#include <cassert>
#include <functional>
#include <type_traits>

#include "detail/fixed_unordered_map_non_trivial.hpp"
#include "detail/fixed_unordered_map_trivial.hpp"

namespace ass
{

template <size_t Capacity, typename Key, typename Value, typename Hasher = std::hash<Key>>
using FixedUnorderedMap = std::conditional_t<
    std::is_trivially_destructible_v<Key> && std::is_trivially_destructible_v<Value>,
    fixed_unordered_map_detail::trivially_destructible::FixedUnorderedMap<Capacity, Key, Value, Hasher>,
    fixed_unordered_map_detail::non_trivially_destructible::FixedUnorderedMap<Capacity, Key, Value, Hasher>>;

}  // namespace ass
