#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>

#include "../fixed_bitset.hpp"
#include "../fixed_unordered_map.hpp"

namespace ass::enum_as_index_detail
{
template <size_t MapCapacity, typename Hasher, typename T, size_t ElementsCount>
constexpr FixedUnorderedMap<MapCapacity, T, size_t, Hasher> MakeEnumToIndexMap(
    const std::array<T, ElementsCount>& elements)
{
    FixedUnorderedMap<MapCapacity, T, size_t, Hasher> result{};
    for (size_t index = 0; index != elements.size(); ++index)
    {
        result.Add(elements[index], index);
    }
    return result;
}
template <typename T>
struct ConstexprEnumHasher
{
    constexpr ConstexprEnumHasher() = default;
    constexpr size_t operator()(const T value) const noexcept
    {
        auto myabs = [](auto v)
        {
            return v < 0 ? -v : v;
        };
        using U = std::underlying_type_t<T>;
        return static_cast<size_t>(myabs(static_cast<U>(value)));
    }
};
}  // namespace ass::enum_as_index_detail

namespace ass
{

template <typename T>
struct EnumIndexConverter
{
    static constexpr bool IsImplemented()
    {
        return false;
    }
    static constexpr size_t ConvertEnumToIndex(const T enum_value);
    static constexpr T ConvertIndexToEnum(const size_t index);
    static constexpr size_t GetElementsCount();
};

template <typename T, T enum_begin, T enum_end>
struct EnumIndexConverter_Continuous
{
    using U = std::underlying_type_t<T>;
    static constexpr U kBegin = static_cast<U>(enum_begin);
    static constexpr U kEnd = static_cast<U>(enum_end);

    static constexpr bool IsImplemented()
    {
        return true;
    }
    static constexpr size_t ConvertEnumToIndex(const T enum_value)
    {
        const U u_value = static_cast<U>(enum_value);
        assert(u_value >= kBegin && u_value < kEnd);
        const size_t index = static_cast<size_t>(u_value - kBegin);
        return index;
    }
    static constexpr T ConvertIndexToEnum(const size_t index)
    {
        assert(index < GetElementsCount());
        const U u_value = static_cast<U>(index) + kBegin;
        const T enum_value = static_cast<T>(u_value);
        return enum_value;
    }
    static constexpr size_t GetElementsCount()
    {
        return static_cast<size_t>(kEnd - kBegin);
    }

    static_assert(std::is_enum_v<T>);
    static_assert(kBegin <= kEnd);
};

template <
    typename T,
    auto values_fn,
    size_t CustomMapCapacity = 0,
    typename Hasher = enum_as_index_detail::ConstexprEnumHasher<T>>
struct EnumIndexConverter_Sparse
{
    using U = std::underlying_type_t<T>;
    static constexpr auto kEnumValues = values_fn();
    static constexpr size_t kMapCapacity = CustomMapCapacity == 0 ? kEnumValues.size() * 2 : CustomMapCapacity;
    static constexpr auto kEnumToIndexMap = enum_as_index_detail::MakeEnumToIndexMap<kMapCapacity, Hasher>(kEnumValues);
    static_assert(kMapCapacity > kEnumValues.size());
    static constexpr bool IsImplemented()
    {
        return true;
    }
    static constexpr size_t ConvertEnumToIndex(const T enum_value)
    {
        return kEnumToIndexMap.Get(enum_value);
    }
    static constexpr T ConvertIndexToEnum(const size_t index)
    {
        return kEnumValues[index];
    }
    static constexpr size_t GetElementsCount()
    {
        return kEnumValues.size();
    }
};

}  // namespace ass
