#pragma once

#include "ass/enum_set.hpp"

namespace ass::enum_map_detail::non_trivially_destructible
{
template <typename Key, typename Value, typename Converter>
class EnumMap
{
public:
    EnumMap() = default;

    EnumMap(const EnumMap& another) : keys_(another.keys_)
    {
        // Call copy constructor for each value present in the source
        for (const Key key : keys_)
        {
            const size_t index = Index(key);
            Value& value = ValueRef(index);
            new (&value) Value(another.ValueRef(index));
        }
    }

    EnumMap& operator=(const EnumMap& another)
    {
        // Self assignment - do nothing
        if (this == &another)
        {
            return *this;
        }

        constexpr auto all_keys = EnumSet<Key, Converter>().GetComplement();

        // Delete all keys that will not be needed
        for (const Key key : keys_ - another.keys_)
        {
            const size_t index = Index(key);
            ValueRef(index).~Value();
        }

        // Call copy constructor for keys that present in source but not locally
        for (const Key key : another.keys_ - keys_)
        {
            const size_t index = Index(key);
            new (&ValueRef(index)) Value(another.ValueRef(index));
        }

        // Call assign operator for keys that present in both collections
        for (const Key key : another.keys_.GetIntersectionWith(another.keys_))
        {
            const size_t index = Index(key);
            ValueRef(index) = another.ValueRef(index);
        }

        // Finally copy keys set
        keys_ = another.keys_;

        return *this;
    }

    EnumMap(EnumMap&& another) noexcept : keys_(another.keys_)
    {
        for (const Key key : keys_)
        {
            const size_t index = Index(key);
            new (&ValueRef(index)) Value(std::move(another.ValueRef(index)));
        }
    }

    EnumMap& operator=(EnumMap&& another) noexcept
    {
        // Self assignment - do nothing
        if (this == &another)
        {
            return *this;
        }

        constexpr auto all_keys = EnumSet<Key, Converter>().GetComplement();

        // Delete all keys that will not be needed
        for (const Key key : keys_ - another.keys_)
        {
            const size_t index = Index(key);
            ValueRef(index).~Value();
        }

        // Call move constructor for keys that present in source but not locally
        for (const Key key : another.keys_ - keys_)
        {
            const size_t index = Index(key);
            new (&ValueRef(index)) Value(move(another.ValueRef(index)));
        }

        // Call assign operator for keys that present in both collections
        for (const Key key : another.keys_.GetIntersectionWith(another.keys_))
        {
            const size_t index = Index(key);
            ValueRef(index) = std::move(another.ValueRef(index));
        }

        // Finally copy keys set
        keys_ = another.keys_;

        return *this;
    }

    ~EnumMap()
    {
        for (const Key key : keys_)
        {
            const size_t index = Index(key);
            ValueRef(index).~Value();
        }
    }

    template <typename... Args>
    Value& Emplace(const Key key, Args&&... args)
    {
        Value& value = ValueRef(Index(key));

        if (keys_.Add(key))
        {
            new (&value) Value(std::forward<Args>(args)...);
        }
        else
        {
            value = Value(std::forward<Args>(args)...);
        }

        return value;
    }

    Value& GetOrAdd(const Key key, std::optional<Value> opt_value = std::nullopt)
    {
        const size_t index = Index(key);
        Value& value = ValueRef(index);

        if (keys_.Add(key))
        {
            if (opt_value)
            {
                new (&value) Value(std::move(*opt_value));
            }
            else
            {
                new (&value) Value;
            }
        }
        else if (opt_value)
        {
            value = std::move(*opt_value);
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
        if (keys_.Remove(key))
        {
            const size_t index = Index(key);
            Value& value = ValueRef(index);
            std::optional<Value> result = std::move(value);
            value.~Value();
            return result;
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
    alignas(Value) std::array<uint8_t, kValuesBytesCount> values_{};
    EnumSet<Key, Converter> keys_{};
};
}  // namespace ass::enum_map_detail::non_trivially_destructible