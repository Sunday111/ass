#pragma once

#include <cstddef>
#include <type_traits>

namespace ass::enum_map_detail::non_trivially_destructible
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
        const KeyType key;  // NOLINT
        ValueType& value;   // NOLINT
    };

public:
    EnumMapIterator(Map& map, size_t index) : map_(&map), index_(index) {}

    KeyValue operator*() const
    {
        return KeyValue{KeyConverter::ConvertIndexToEnum(index_), map_->ValueRef(index_)};
    }

    EnumMapIterator& operator++() noexcept
    {
        index_ = map_->keys_.GetBitset().CountContinuousZeroBits(index_ + 1);
        return *this;
    }

    bool operator==(const EnumMapIterator& another) const
    {
        return index_ == another.index_ && (&map_ == &another.map_);
    }

    bool operator!=(const EnumMapIterator& another) const
    {
        return !(*this == another);
    }

private:
    Map* map_ = nullptr;
    size_t index_;
};
}  // namespace ass::enum_map_detail::non_trivially_destructible
