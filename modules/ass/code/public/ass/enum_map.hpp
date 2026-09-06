#pragma once

#include <array>
#include <cassert>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "enum/enum_as_index.hpp"
#include "enum_set.hpp"

namespace ass
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
    constexpr EnumMapIterator(Map& map, size_t index) : map_(&map), index_(index) {}

    constexpr KeyValue operator*() const
    {
        return KeyValue{KeyConverter::ConvertIndexToEnum(index_), map_->ValueRef(index_)};
    }

    constexpr EnumMapIterator& operator++() noexcept
    {
        index_ = map_->keys_.GetBitset().CountContinuousZeroBits(index_ + 1);
        return *this;
    }

    constexpr bool operator==(const EnumMapIterator& another) const
    {
        return index_ == another.index_ && (map_ == another.map_);
    }

    constexpr bool operator!=(const EnumMapIterator& another) const
    {
        return !(*this == another);
    }

private:
    Map* map_;
    size_t index_;
};

template <typename Key, typename Value, typename Converter = EnumIndexConverter<Key>>
class EnumMap
{
public:
    using KeyType = Key;
    using ValueType = Value;
    using KeyConverter = Converter;
    using Iterator = EnumMapIterator<EnumMap>;
    using ConstIterator = EnumMapIterator<const EnumMap>;

    friend Iterator;
    friend ConstIterator;

    constexpr EnumMap() = default;

    constexpr EnumMap(const EnumMap& another)
        requires std::is_copy_constructible_v<Value>
        : EnumMap()
    {
        *this = another;
    }

    constexpr EnumMap(EnumMap&& another) noexcept(std::is_nothrow_move_constructible_v<Value>)
        requires std::is_move_constructible_v<Value>
        : EnumMap()
    {
        *this = std::move(another);
    }

    constexpr EnumMap& operator=(const EnumMap& another)
        requires std::is_copy_constructible_v<Value>
    {
        if (this != &another)
        {
            Clear();
            for (auto [key, value] : another) Emplace(key, value);
        }
        return *this;
    }

    constexpr EnumMap& operator=(EnumMap&& another) noexcept(std::is_nothrow_move_constructible_v<Value>)
        requires std::is_move_constructible_v<Value>
    {
        if (this != &another)
        {
            Clear();
            for (auto [key, value] : another) Emplace(key, std::move(value));
            another.Clear();
        }
        return *this;
    }

    constexpr ~EnumMap()
    {
        Clear();
    }

    template <typename... Args>
        requires std::is_constructible_v<Value, Args...>
    constexpr Value& Emplace(Key key, Args&&... args)
    {
        if (Contains(key))
        {
            if constexpr (std::is_assignable_v<Value&, Value>)
            {
                Value& value = Get(key);
                value = Value(std::forward<Args>(args)...);
                return value;
            }
            else
            {
                DestroyValue(key);
            }
        }

        Value* value = std::construct_at(std::addressof(values_[Index(key)].value), std::forward<Args>(args)...);
        keys_.Add(key);
        return *value;
    }

    constexpr Value& GetOrAdd(Key key)
        requires std::is_default_constructible_v<Value>
    {
        return Contains(key) ? Get(key) : Emplace(key);
    }

    constexpr Value& GetOrAdd(Key key, Value value)
        requires(std::is_move_constructible_v<Value> || std::is_copy_constructible_v<Value>)
    {
        return Emplace(key, std::move_if_noexcept(value));
    }

    constexpr const Value& Get(Key key) const
    {
        assert(Contains(key));
        return ValueRef(Index(key));
    }

    constexpr Value& Get(Key key)
    {
        assert(Contains(key));
        return ValueRef(Index(key));
    }

    constexpr std::optional<Value> Remove(Key key)
        requires(std::is_move_constructible_v<Value> || std::is_copy_constructible_v<Value>)
    {
        if (!Contains(key)) return std::nullopt;
        std::optional<Value> removed(std::in_place, std::move_if_noexcept(Get(key)));
        DestroyValue(key);
        return removed;
    }

    constexpr bool Contains(Key key) const
    {
        return keys_.Contains(key);
    }

    constexpr size_t Size() const
    {
        return keys_.Size();
    }

    static constexpr size_t Capacity()
    {
        return kCapacity;
    }

    // STL
    constexpr Iterator begin() noexcept
    {
        return MakeBegin<Iterator>(this);
    }

    constexpr ConstIterator begin() const noexcept
    {
        return cbegin();
    }

    constexpr ConstIterator cbegin() const noexcept
    {
        return MakeBegin<ConstIterator>(this);
    }

    constexpr Iterator end() noexcept
    {
        return MakeEnd<Iterator>(this);
    }

    constexpr ConstIterator end() const noexcept
    {
        return cend();
    }

    constexpr ConstIterator cend() const noexcept
    {
        return MakeEnd<ConstIterator>(this);
    }

private:
    static constexpr size_t kCapacity = Converter::GetElementsCount();

    static constexpr size_t Index(Key key)
    {
        return Converter::ConvertEnumToIndex(key);
    }

    constexpr Value& ValueRef(size_t index)
    {
        return values_[index].value;
    }

    constexpr const Value& ValueRef(size_t index) const
    {
        return values_[index].value;
    }

    template <typename It, typename This>
    static constexpr It MakeBegin(This this_) noexcept
    {
        return It(*this_, this_->keys_.GetBitset().CountContinuousZeroBits());
    }

    template <typename It, typename This>
    static constexpr It MakeEnd(This this_) noexcept
    {
        return It(*this_, kCapacity);
    }

private:
    union Slot
    {
        constexpr Slot() noexcept {}
        constexpr ~Slot() noexcept {}
        Value value;
    };

    constexpr void DestroyValue(Key key)
    {
        std::destroy_at(std::addressof(Get(key)));
        keys_.Remove(key);
    }

    constexpr void Clear()
    {
        for (auto [key, value] : *this) DestroyValue(key);
    }

    std::array<Slot, kCapacity> values_{};
    EnumSet<Key, Converter> keys_{};
};
}  // namespace ass
