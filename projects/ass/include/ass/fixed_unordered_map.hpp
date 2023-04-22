#pragma once

#include <array>
#include <cassert>
#include <functional>
#include <optional>
#include <type_traits>

#include "ass/detail/fixed_unordered_map_non_trivial.hpp"
#include "ass/detail/fixed_unordered_map_trivial.hpp"

namespace ass
{

template <size_t Capacity, typename Key, typename Value, typename Hasher = std::hash<Key>>
using FixedUnorderedMap = std::conditional_t<
    std::is_trivially_destructible_v<Key> && std::is_trivially_destructible_v<Value>,
    fixed_unordered_map_detail::FixedMapTriviallyDestructible<Capacity, Key, Value, Hasher>,
    fixed_unordered_map_detail::FixedMapNonTriviallyDestructible<Capacity, Key, Value, Hasher>>;

}  // namespace ass
