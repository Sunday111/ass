#pragma once

#include "ass/enum_set.hpp"

namespace ass::enum_map_detail::non_trivially_destructible
{
template <typename Key, typename Value, typename Converter>
class EnumMap
{
public:
    EnumMap() = default;

    Value& GetOrAdd(const Key key, std::optional<Value> opt_value = std::nullopt)
    {
        const size_t index = Index(key);
        Value& value = ValueRef(index);

        if (keys_.Contains(key))
        {
            if (opt_value)
            {
                value = std::move(*opt_value);
            }
        }
        else
        {
            keys_.Add(key);
            if (opt_value)
            {
                value = std::move(*opt_value);
            }
            else
            {
                value = Value{};
            }
        }

        return value;
    }

    const Value& Get(const Key key) const
    {
        assert(Contains(key));
        return ValueRef(Index(key));
    }

    Value& Get(const Key key)
    {
        assert(Contains(key));
        return ValueRef(Index(key));
    }

    std::optional<Value> Remove(const Key key)
    {
        if (Contains(key))
        {
            const size_t index = Index(key);
            keys_.Remove(key);
            return std::move(ValueRef(index));
        }

        return std::nullopt;
    }

    bool Contains(const Key key) const
    {
        return keys_.Contains(key);
    }

    size_t Size() const
    {
        return keys_.Size();
    }

    size_t Capacity() const
    {
        return kCapacity;
    }

private:
    static constexpr size_t kCapacity = Converter::GetElementsCount();
    static constexpr size_t kBytesCountForValues = sizeof(Value) * kCapacity;

    static constexpr size_t Index(const Key key)
    {
        return Converter::ConvertEnumToIndex(key);
    }

    Value& ValueRef(size_t index)
    {
        const size_t offset = sizeof(Value) * index;
        return *reinterpret_cast<Value*>(&values_[offset]);
    }

    const Value& ValueRef(size_t index) const
    {
        const size_t offset = sizeof(Value) * index;
        return *reinterpret_cast<Value*>(&values_[offset]);
    }

private:
    static constexpr size_t kValuesBytesCount = sizeof(Value) * kCapacity;
    alignas(Value) std::array<Value, kValuesBytesCount> values_{};
    EnumSet<Key, Converter> keys_{};
};
}  // namespace ass::enum_map_detail::non_trivially_destructible