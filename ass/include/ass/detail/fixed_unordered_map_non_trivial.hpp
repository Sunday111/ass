#pragma once

#include <cstddef>
#include <optional>

#include "ass/fixed_bitset.hpp"
#include "fixed_unordered_map_iterator_non_trivial.hpp"

namespace ass::fixed_unordered_map_detail::non_trivially_destructible
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

    static constexpr size_t Capacity() noexcept
    {
        return capacity;
    }

    FixedUnorderedMap() = default;

    FixedUnorderedMap(const FixedUnorderedMap& another)
    {
        for (size_t index = 0; index != capacity; ++index)
        {
            if (another.has_index_.Get(index))
            {
                has_index_.Set(index, true);
                new (&GetKeyAt(index)) Key(another.GetKeyAt(index));
                new (&GetValueAt(index)) Value(another.GetValueAt(index));
            }
        }
    }

    FixedUnorderedMap(FixedUnorderedMap&& another) noexcept
    {
        for (size_t index = 0; index != capacity; ++index)
        {
            if (another.has_index_.Get(index))
            {
                has_index_.Set(index, true);
                new (&GetKeyAt(index)) Key(std::move(another.GetKeyAt(index)));
                new (&GetValueAt(index)) Value(std::move(another.GetValueAt(index)));
            }
        }
    }

    ~FixedUnorderedMap()
    {
        if constexpr (capacity != 0)
        {
            size_t index = GetFirstIndexWithValue();

            while (HasValueAtIndex(index))
            {
                EraseIndex(index);
                index = GetNextIndexWithValue(index);
            }
        }
    }

    FixedUnorderedMap& operator=(const FixedUnorderedMap& another)
    {
        if (this == &another)
        {
            return *this;
        }

        for (size_t index = 0; index != capacity; ++index)
        {
            const bool this_has_index = has_index_.Get(index);

            if (another.has_index_.Get(index))
            {
                if (this_has_index)
                {
                    GetKeyAt(index) = another.GetKeyAt(index);
                    GetValueAt(index) = another.GetValueAt(index);
                }
                else
                {
                    has_index_.Set(index, true);
                    new (&GetKeyAt(index)) Key(another.GetKeyAt(index));
                    new (&GetValueAt(index)) Value(another.GetValueAt(index));
                }
            }
            else if (this_has_index)
            {
                EraseIndex(index);
            }
        }
        return *this;
    }

    FixedUnorderedMap& operator=(FixedUnorderedMap&& another) noexcept
    {
        if (this == &another)
        {
            return *this;
        }

        for (size_t index = 0; index != capacity; ++index)
        {
            const bool this_has_index = has_index_.Get(index);

            if (another.has_index_.Get(index))
            {
                if (this_has_index)
                {
                    GetKeyAt(index) = std::move(another.GetKeyAt(index));
                    GetValueAt(index) = std::move(another.GetValueAt(index));
                }
                else
                {
                    has_index_.Set(index, true);
                    new (&GetKeyAt(index)) Key(std::move(another.GetKeyAt(index)));
                    new (&GetValueAt(index)) Value(std::move(another.GetValueAt(index)));
                }
            }
            else if (this_has_index)
            {
                EraseIndex(index);
            }
        }
        return *this;
    }

    constexpr bool Contains(const Key key) const
    {
        const size_t index = FindIndexForKey(key);
        return HasValueAtIndex(index);
    }

    Value& Get(const Key key)
    {
        const size_t index = FindIndexForKey(key);
        assert(HasValueAtIndex(index));
        return GetValueAt(index);
    }

    const Value& Get(const Key key) const
    {
        const size_t index = FindIndexForKey(key);
        assert(HasValueAtIndex(key));
        return GetValueAt(index);
    }

    Value* TryAdd(const Key key, std::optional<Value> value = std::nullopt)
    {
        const size_t index = FindIndexForKey(key);
        if (index == capacity)
        {
            return nullptr;
        }

        Value& value_ref = GetValueAt(index);
        if (has_index_.Get(index))
        {
            if (value != std::nullopt)
            {
                value_ref = std::move(*value);
            }
        }
        else
        {
            has_index_.Set(index, true);
            Key& key_ref = GetKeyAt(index);
            new (&key_ref) Key(key);
            if (value == std::nullopt)
            {
                new (&value_ref) Value();
            }
            else
            {
                new (&value_ref) Value(std::move(*value));
            }
        }

        return &value_ref;
    }

    Value& Add(const Key key, Value value = {})
    {
        auto ptr = TryAdd(key, std::move(value));
        assert(ptr);
        return *ptr;
    }

    std::optional<Value> Remove(const Key key)
    {
        if (const size_t index = FindIndexForKey(key); HasValueAtIndex(index))
        {
            std::optional<Value> moved = std::move(GetValueAt(index));
            EraseIndex(index);
            return moved;
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

    constexpr size_t FindIndexForKey(const Key key) const
    {
        const size_t start_index = ToIndex(Hasher{}(key));
        for (size_t collision_index = 0; collision_index != capacity; ++collision_index)
        {
            const size_t index = ToIndex(start_index + collision_index);

            if (!has_index_.Get(index) || GetKeyAt(index) == key)
            {
                return index;
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

    constexpr bool HasValueAtIndex(const size_t index) const noexcept
    {
        if (index < capacity)
        {
            return has_index_.Get(index);
        }

        return false;
    }

    size_t GetFirstIndexWithValue() const noexcept
    {
        return has_index_.CountContinuousZeroBits();
    }

    size_t GetNextIndexWithValue(size_t prev_index) const noexcept
    {
        assert(prev_index <= capacity);
        return has_index_.CountContinuousZeroBits(prev_index + 1);
    }

    template <typename TT>
    static constexpr size_t GetOffsetForIndex(size_t index) noexcept
    {
        return sizeof(TT) * index;
    }

    Key& GetKeyAt(size_t index) noexcept
    {
        const size_t offset = GetOffsetForIndex<Key>(index);
        return *reinterpret_cast<Key*>(&keys_data_[offset]);
    }

    const Key& GetKeyAt(size_t index) const noexcept
    {
        const size_t offset = GetOffsetForIndex<Key>(index);
        return *reinterpret_cast<const Key*>(&keys_data_[offset]);
    }

    Value& GetValueAt(size_t index) noexcept
    {
        const size_t offset = GetOffsetForIndex<Value>(index);
        return *reinterpret_cast<Value*>(&values_data_[offset]);
    }

    const Value& GetValueAt(size_t index) const noexcept
    {
        const size_t offset = GetOffsetForIndex<Value>(index);
        return *reinterpret_cast<const Value*>(&values_data_[offset]);
    }

    void EraseIndex(size_t index)
    {
        has_index_.Set(index, false);
        GetKeyAt(index).~Key();
        GetValueAt(index).~Value();
    }

private:
    alignas(std::array<Key, capacity>) std::array<uint8_t, sizeof(Key) * capacity> keys_data_;
    alignas(std::array<Value, capacity>) std::array<uint8_t, sizeof(Value) * capacity> values_data_;
    FixedBitset<capacity> has_index_{};
};

}  // namespace ass::fixed_unordered_map_detail::non_trivially_destructible
