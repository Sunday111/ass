#pragma once

#include <cstddef>
#include <type_traits>

#include "../enum/enum_as_index.hpp"

namespace ass::enum_map_detail::trivially_destructible
{

template <typename Map>
class EnumMapIterator
{
    using CleanMap = std::remove_const_t<Map>;
    using KeyType = typename CleanMap::KeyType;
    using ValueType = std::conditional_t<
        std::is_const_v<Map>,
        std::add_const_t<typename CleanMap::ValueType>,
        typename CleanMap::ValueType>;
    using KeyConverter = typename CleanMap::KeyConverter;

public:
    struct KeyValue
    {
        const KeyType key;
        ValueType& value;
    };

public:
    constexpr EnumMapIterator(Map& map, size_t index) : map_(map), index_(index) {}

    constexpr KeyValue operator*() const
    {
        return KeyValue{KeyConverter::ConvertIndexToEnum(index_), map_.ValueRef(index_)};
    }

    constexpr EnumMapIterator& operator++() noexcept
    {
        index_ = map_.keys_.GetBitset().CountContinuousZeroBits(index_ + 1);
        return *this;
    }

    constexpr bool operator==(const EnumMapIterator& another) const
    {
        return index_ == another.index_ && (&map_ == &another.map_);
    }

    constexpr bool operator!=(const EnumMapIterator& another) const
    {
        return !(*this == another);
    }

private:
    Map& map_;
    size_t index_;
};
}  // namespace ass::enum_map_detail::trivially_destructible
