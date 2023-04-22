#pragma once

#include <cstddef>
#include <optional>

#include "ass/fixed_bitset.hpp"
#include "fixed_unordered_map_iterator_non_trivial.hpp"

namespace ass::fixed_unordered_map_detail
{

template <size_t Capacity, typename Key, typename Value, typename Hasher>
class FixedMapNonTriviallyDestructible
{
public:
    using Self = FixedMapNonTriviallyDestructible<Capacity, Key, Value, Hasher>;
    using Iterator = FixedMapIteratorNonTrivial<Self>;
    using ConstIterator = FixedMapIteratorNonTrivial<std::add_const_t<Self>>;
    friend Iterator;
    friend ConstIterator;

    constexpr FixedMapNonTriviallyDestructible() = default;

    constexpr bool Contains(const Key key) const
    {
        const size_t index = FindIndexForKey(key);
        return index != Capacity && elements_[index].has_value();
    }

    Value& Get(const Key key)
    {
        const size_t index = FindIndexForKey(key);
        assert(index != Capacity && elements_[index].has_value());
        return elements_[index]->second;
    }

    const Value& Get(const Key key) const
    {
        const size_t index = FindIndexForKey(key);
        assert(index != Capacity && elements_[index].has_value());
        return elements_[index]->second;
    }

    Value* TryAdd(const Key key, std::optional<Value> value = std::nullopt)
    {
        const size_t index = FindIndexForKey(key);
        if (index == Capacity)
        {
            return nullptr;
        }

        auto& maybe_kv = elements_[index];
        if (maybe_kv.has_value())
        {
            if (value != std::nullopt)
            {
                maybe_kv->second = std::move(*value);
            }
        }
        else
        {
            if (value == std::nullopt)
            {
                maybe_kv = std::pair<Key, Value>{key, Value{}};
            }
            else
            {
                maybe_kv = std::pair<Key, Value>{key, std::move(*value)};
            }

            size_ += 1;
        }

        return &maybe_kv->second;
    }

    Value& Add(const Key key, Value value = {})
    {
        auto ptr = TryAdd(key, std::move(value));
        assert(ptr);
        return *ptr;
    }

    std::optional<Value> Remove(const Key key)
    {
        const size_t index = FindIndexForKey(key);
        if (index == Capacity)
        {
            return std::nullopt;
        }

        auto& maybe_kv = elements_[index];
        if (maybe_kv.has_value())
        {
            size_ -= 1;
            std::optional<std::pair<Key, Value>> prev;
            std::swap(prev, maybe_kv);
            return prev->second;
        }

        return std::nullopt;
    }

    size_t Size() const
    {
        return size_;
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
            const auto& maybe_kv = elements_[index];
            if (!maybe_kv.has_value() || maybe_kv->first == key)
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

private:
    size_t size_ = 0;
    std::array<std::optional<std::pair<Key, Value>>, Capacity> elements_{};
};

}  // namespace ass::fixed_unordered_map_detail
