#pragma once

#include <cstddef>
#include <optional>

#include "ass/fixed_bitset.hpp"
#include "fixed_unordered_map_iterator_trivial.hpp"

namespace ass::fixed_unordered_map_detail
{
template <size_t Capacity, typename Key_, typename Value_, typename Hasher_>
class FixedMapTriviallyDestructible
{
public:
    using Key = Key_;
    using Value = Value_;
    using Hasher = Hasher_;
    using Self = FixedMapTriviallyDestructible<Capacity, Key, Value, Hasher>;
    using Iterator = FixedMapIteratorTrivial<Self>;
    using ConstIterator = FixedMapIteratorTrivial<std::add_const_t<Self>>;
    friend Iterator;
    friend ConstIterator;

    constexpr FixedMapTriviallyDestructible() = default;

    constexpr bool Contains(const Key key) const
    {
        const size_t index = FindIndexForKey(key);
        return index != Capacity && has_index_.Get(index);
    }

    constexpr Value& Get(const Key key)
    {
        const size_t index = FindIndexForKey(key);
        assert(index != Capacity && has_index_.Get(index));
        return values_[index];
    }

    constexpr const Value& Get(const Key key) const
    {
        const size_t index = FindIndexForKey(key);
        assert(index != Capacity && has_index_.Get(index));
        return values_[index];
    }

    constexpr Value* TryAdd(const Key key, std::optional<Value> value = std::nullopt)
    {
        const size_t index = FindIndexForKey(key);
        if (index == Capacity)
        {
            return nullptr;
        }

        if (has_index_.Get(index))
        {
            if (value != std::nullopt)
            {
                values_[index] = std::move(*value);
            }
        }
        else
        {
            has_index_.Set(index, true);
            keys_[index] = key;
            if (value == std::nullopt)
            {
                values_[index] = Value{};
            }
            else
            {
                values_[index] = std::move(*value);
            }
        }

        return &values_[index];
    }

    constexpr Value& Add(const Key key, Value value = {})
    {
        auto ptr = TryAdd(key, std::move(value));
        assert(ptr);
        return *ptr;
    }

    constexpr std::optional<Value> Remove(const Key key)
    {
        const size_t index = FindIndexForKey(key);
        if (index != Capacity && has_index_.Get(index))
        {
            has_index_.Set(index, false);
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
        if (this_->HasValueAtIndex(0))
        {
            return It(*this_, 0);
        }
        else
        {
            return It(*this_, this_->GetNextIndexWithValue(0));
        }
    }

    template <typename It, typename This>
    static constexpr It MakeEnd(This this_) noexcept
    {
        return It(*this_, Capacity);
    }

    constexpr size_t FindIndexForKey(const Key key) const
    {
        const size_t start_index = ToIndex(Hasher{}(key));
        for (size_t collision_index = 0; collision_index != Capacity; ++collision_index)
        {
            const size_t index = ToIndex(start_index + collision_index);
            if (!has_index_.Get(index) || keys_[index] == key)
            {
                return index;
            }
        }

        return Capacity;
    }

    static constexpr size_t ToIndex(const size_t value)
    {
        if constexpr (Capacity == 0)
        {
            return 0;
        }
        else
        {
            return value % Capacity;
        }
    }

    constexpr bool HasValueAtIndex(size_t index) const noexcept
    {
        if (index < Capacity)
        {
            return has_index_.Get(index);
        }

        return false;
    }

    constexpr size_t GetNextIndexWithValue(size_t prev_index) const noexcept
    {
        assert(prev_index <= Capacity);
        size_t index = prev_index;
        while (index != Capacity)
        {
            ++index;
            if (index == Capacity || has_index_.Get(index))
            {
                break;
            }
        }

        return index;
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
    std::array<Key, Capacity> keys_{};
    std::array<Value, Capacity> values_{};
    FixedBitset<Capacity> has_index_{};
};
}  // namespace ass::fixed_unordered_map_detail
