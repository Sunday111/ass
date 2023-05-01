#pragma once

#include <type_traits>

#include "detail/enum_map_non_trivial.hpp"
#include "detail/enum_map_trivial.hpp"
#include "enum/enum_as_index.hpp"

namespace ass
{
template <typename Key, typename Value, typename Converter = EnumIndexConverter<Key>>
using EnumMap = std::conditional_t<
    std::is_trivially_destructible_v<Value>,
    enum_map_detail::trivially_destructible::EnumMap<Key, Value, Converter>,
    enum_map_detail::non_trivially_destructible::EnumMap<Key, Value, Converter>>;
}
