#pragma once

#include <array>
#include <cassert>
#include <functional>
#include <optional>
#include <type_traits>

#include "ass/fixed_bitset.hpp"

namespace ass::detail
{

template <size_t Capacity, typename Key, typename Value, typename Hasher>
class FixedMapTriviallyDestructible;

template <size_t Capacity, typename Key, typename Value, typename Hasher>
class FixedMapNonTriviallyDestructible;
}  // namespace ass::detail

namespace ass
{

template <size_t Capacity, typename Key, typename Value, typename Hasher = std::hash<Key>>
using FixedUnorderedMap = std::conditional_t<
    std::is_trivially_destructible_v<Key> && std::is_trivially_destructible_v<Value>,
    detail::FixedMapTriviallyDestructible<Capacity, Key, Value, Hasher>,
    detail::FixedMapNonTriviallyDestructible<Capacity, Key, Value, Hasher>>;

}  // namespace ass

namespace ass::detail
{

template <size_t Capacity, typename Key, typename Value, typename Hasher>
class FixedMapTriviallyDestructible
{
public:
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

protected:
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

private:
    std::array<Key, Capacity> keys_{};
    std::array<Value, Capacity> values_{};
    FixedBitset<Capacity> has_index_{};
};

template <size_t Capacity, typename Key, typename Value, typename Hasher>
class FixedMapNonTriviallyDestructible
{
public:
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

protected:
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
}  // namespace ass::detail