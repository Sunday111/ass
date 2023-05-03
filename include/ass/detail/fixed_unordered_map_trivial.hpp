#pragma once

#include <cstddef>
#include <optional>

#include "../fixed_bitset.hpp"
#include "fixed_unordered_map_iterator_trivial.hpp"

namespace ass::fixed_unordered_map_detail::trivially_destructible
{
template <size_t capacity, typename Key_, typename Value_, typename Hasher_>
class FixedUnorderedMap
{
public:
    using Key = Key_;
    using Value = Value_;
    using Hasher = Hasher_;
    using Self = FixedUnorderedMap<capacity, Key, Value, Hasher>;
    using Iterator = FixedUnorderedMapIterator<Self>;
    using ConstIterator = FixedUnorderedMapIterator<std::add_const_t<Self>>;
    friend Iterator;
    friend ConstIterator;

    constexpr FixedUnorderedMap() = default;

    constexpr bool Contains(const Key key) const
    {
        const size_t index = FindKeyIndex(key);
        return index != capacity && has_index_.Get(index);
    }

    static constexpr size_t Capacity() noexcept
    {
        return capacity;
    }

    constexpr Value& Get(const Key key)
    {
        const size_t index = FindKeyIndex(key);
        assert(index != capacity && has_index_.Get(index));
        return values_[index];
    }

    constexpr const Value& Get(const Key key) const
    {
        const size_t index = FindKeyIndex(key);
        assert(index != capacity && has_index_.Get(index));
        return values_[index];
    }

    template <typename... Args>
    constexpr Value* TryEmplace(const Key key, Args&&... args)
    {
        const size_t index = FindFreeIndexForKey(key);
        if (index == capacity)
        {
            return nullptr;
        }

        Value& value = values_[index];
        has_index_.Set(index, true);
        was_deleted_.Set(index, false);
        value = Value(std::forward<Args>(args)...);
        return &value;
    }

    template <typename... Args>
    constexpr Value& Emplace(const Key key, Args&&... args)
    {
        auto ptr = TryEmplace(key, std::forward<Args>(args)...);
        assert(ptr);
        return *ptr;
    }

    constexpr Value* TryAdd(const Key key, std::optional<Value> value = std::nullopt)
    {
        const size_t index = FindFreeIndexForKey(key);
        if (index == capacity)
        {
            return nullptr;
        }

        const bool must_init = has_index_.Set(index, true);

        if (must_init)
        {
            keys_[index] = key;
            was_deleted_.Set(index, false);
        }

        Value& value_ref = values_[index];
        if (value != std::nullopt)
        {
            value_ref = std::move(*value);
        }
        else if (must_init)
        {
            value_ref = Value{};
        }

        return &value_ref;
    }

    constexpr Value& Add(const Key key, std::optional<Value> value = std::nullopt)
    {
        auto ptr = TryAdd(key, std::move(value));
        assert(ptr);
        return *ptr;
    }

    constexpr std::optional<Value> Remove(const Key key)
    {
        const size_t index = FindKeyIndex(key);
        if (index != capacity && has_index_.Set(index, false))
        {
            was_deleted_.Set(index, true);
            return std::move(values_[index]);
        }

        return std::nullopt;
    }

    constexpr size_t Size() const
    {
        return has_index_.CountOnes();
    }

    // STL conformance
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

protected:
    template <typename It, typename This>
    static constexpr It MakeBegin(This this_) noexcept
    {
        return It(*this_, this_->GetFirstIndexWithValue());
    }

    template <typename It, typename This>
    static constexpr It MakeEnd(This this_) noexcept
    {
        return It(*this_, capacity);
    }

    constexpr size_t FindKeyIndex(const Key key) const
    {
        constexpr bool stop_at_deleted = false;
        return FindIndexForKey<stop_at_deleted>(key);
    }

    constexpr size_t FindFreeIndexForKey(const Key key) const
    {
        constexpr bool stop_at_deleted = true;
        return FindIndexForKey<stop_at_deleted>(key);
    }

    template <bool kStopAtDeleted>
    constexpr size_t FindIndexForKey(const Key key) const
    {
        const size_t start_index = ToIndex(Hasher{}(key));
        for (size_t collision_index = 0; collision_index != capacity; ++collision_index)
        {
            const size_t index = ToIndex(start_index + collision_index);
            if constexpr (kStopAtDeleted)
            {
                if (!has_index_.Get(index) || keys_[index] == key)
                {
                    return index;
                }
            }
            else
            {
                if (has_index_.Get(index))
                {
                    if (keys_[index] == key)
                    {
                        return index;
                    }
                }
                else if (!was_deleted_.Get(index))
                {
                    return index;
                }
            }
        }

        return capacity;
    }

    static constexpr size_t ToIndex(const size_t value)
    {
        if constexpr (capacity == 0)
        {
            return 0;
        }
        else
        {
            return value % capacity;
        }
    }

    constexpr bool HasValueAtIndex(size_t index) const noexcept
    {
        if (index < capacity)
        {
            return has_index_.Get(index);
        }

        return false;
    }

    constexpr size_t GetFirstIndexWithValue() const noexcept
    {
        return has_index_.CountContinuousZeroBits();
    }

    constexpr size_t GetNextIndexWithValue(size_t prev_index) const noexcept
    {
        assert(prev_index <= capacity);
        return has_index_.CountContinuousZeroBits(prev_index + 1);
    }

    constexpr const Key& GetKeyAt(size_t index) const noexcept
    {
        return keys_[index];
    }

    constexpr Value& GetValueAt(size_t index) noexcept
    {
        return values_[index];
    }

    constexpr const Value& GetValueAt(size_t index) const noexcept
    {
        return values_[index];
    }

private:
    std::array<Key, capacity> keys_{};
    std::array<Value, capacity> values_{};
    FixedBitset<capacity> has_index_{};
    FixedBitset<capacity> was_deleted_{};
};
}  // namespace ass::fixed_unordered_map_detail::trivially_destructible
