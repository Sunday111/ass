#pragma once

#include "enum_as_index.hpp"
#include "magic_enum/magic_enum.hpp"

static_assert(magic_enum::is_magic_enum_supported);

namespace ass
{

template <typename T, typename Hasher = enum_as_index_detail::ConstexprEnumHasher<T>>
struct EnumIndexConverter_MagicEnum
{
    using U = std::underlying_type_t<T>;
    static constexpr auto kEnumValues = magic_enum::enum_values<T>();
    static constexpr size_t kMapCapacity = kEnumValues.size();
    static constexpr auto kEnumToIndexMap =
        enum_as_index_detail::MakeEnumToIndexMap<kEnumValues.size(), Hasher>(kEnumValues);
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
