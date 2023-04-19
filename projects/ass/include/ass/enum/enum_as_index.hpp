#pragma once

#include <cassert>
#include <cstddef>
#include <type_traits>

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

}  // namespace ass