#pragma once

#include <array>
#include <optional>

#include "ass/enum/enum_as_index.hpp"
#include "ass/enum_set.hpp"

namespace ass::enum_map_detail::trivially_destructible
{
template <typename Key, typename Value, typename Converter>
class EnumMap
{
public:
    constexpr EnumMap() = default;

    constexpr Value& GetOrAdd(const Key key, std::optional<Value> opt_value = std::nullopt)
    {
        const size_t index = Index(key);

        if (keys_.Contains(key))
        {
            if (opt_value)
            {
                values_[index] = std::move(*opt_value);
            }
        }
        else
        {
            keys_.Add(key);
            if (opt_value)
            {
                values_[index] = std::move(*opt_value);
            }
            else
            {
                values_[index] = Value{};
            }
        }

        return values_[index];
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
        if (Contains(key))
        {
            const size_t index = Index(key);
            keys_.Remove(key);
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

    constexpr size_t Capacity() const
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

private:
    std::array<Value, kCapacity> values_{};
    EnumSet<Key, Converter> keys_{};
};
}  // namespace ass::enum_map_detail::trivially_destructible