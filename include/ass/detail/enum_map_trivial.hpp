#pragma once

#include <array>
#include <optional>

#include "../enum/enum_as_index.hpp"
#include "../enum_set.hpp"
#include "enum_map_iterator_trivial.hpp"

namespace ass::enum_map_detail::trivially_destructible
{
template <typename Key, typename Value, typename Converter>
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

    constexpr Value& GetOrAdd(const Key key, std::optional<Value> opt_value = std::nullopt)
    {
        Value& value = values_[Index(key)];

        if (keys_.Add(key))
        {
            if (opt_value)
            {
                value = std::move(*opt_value);
            }
            else
            {
                value = Value{};
            }
        }
        else if (opt_value)
        {
            value = std::move(*opt_value);
        }

        return value;
    }

    constexpr const Value& Get(const Key key) const
    {
        const size_t index = Index(key);
        return values_[index];
    }

    constexpr Value& Get(const Key key)
    {
        const size_t index = Index(key);
        return values_[index];
    }

    constexpr std::optional<Value> Remove(const Key key)
    {
        if (keys_.Remove(key))
        {
            const size_t index = Index(key);
            return std::move(values_[index]);
        }

        return std::nullopt;
    }

    constexpr bool Contains(const Key key) const
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
    static constexpr size_t kBytesCountForValues = sizeof(Value) * kCapacity;

    static constexpr size_t Index(const Key key)
    {
        return Converter::ConvertEnumToIndex(key);
    }

    constexpr Value& ValueRef(const size_t index)
    {
        return values_[index];
    }

    constexpr const Value& ValueRef(const size_t index) const
    {
        return values_[index];
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
    std::array<Value, kCapacity> values_{};
    EnumSet<Key, Converter> keys_{};
};
}  // namespace ass::enum_map_detail::trivially_destructible
