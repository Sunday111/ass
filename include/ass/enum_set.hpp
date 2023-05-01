#pragma once

#include <cstddef>
#include <type_traits>

#include "enum/enum_as_index.hpp"
#include "fixed_bitset.hpp"

namespace ass
{

template <typename T>
class EnumSetIterator;

// This is a set that contains enumeration values
// It works for types that implement EnumIndexConverter from bit/enum_as_index.hpp.
// An advantage here is that this set does not require heap allocations because
// it knows the number of possible values and can create an appropriate bitset for them.
template <typename T, typename Converter = EnumIndexConverter<T>>
class EnumSet
{
public:
    using Iterator = EnumSetIterator<EnumSet>;
    using ValueType = T;
    using EnumConverter = Converter;
    friend Iterator;

    // Adds new value to the set.
    // Returns true if new value was actually added.
    // Returns false if values was already there.
    constexpr bool Add(const T value)
    {
        const size_t index = Converter::ConvertEnumToIndex(value);
        return bits_.Set(index, true);
    }

    // Removes value from the set.
    // Returns true if value was in the set.
    constexpr bool Remove(const T value)
    {
        const size_t index = Converter::ConvertEnumToIndex(value);
        return bits_.Set(index, false);
    }

    constexpr bool Contains(const T value) const
    {
        const size_t index = Converter::ConvertEnumToIndex(value);
        return bits_.Get(index);
    }

    constexpr const EnumSet GetComplement() const
    {
        auto copy = *this;
        copy.bits_ = ~copy.bits_;
        return copy;
    }

    // Replaces this set with complement to self
    constexpr EnumSet& Invert()
    {
        bits_.Flip();
        return *this;
    }

    constexpr size_t Size() const
    {
        return bits_.CountOnes();
    }

    constexpr size_t Capacity() const
    {
        return kCapacity;
    }

    constexpr bool IsEmpty() const
    {
        return Size() == 0;
    }

    // {A, B, C} - {A, C} -> {B}.
    // {A, C} - {A, B, C} -> {}.
    constexpr EnumSet GetDifferenceFrom(const EnumSet& another) const
    {
        return GetIntersectionWith(another.GetComplement());
    }

    constexpr EnumSet GetIntersectionWith(const EnumSet& another) const
    {
        auto copy = *this;
        copy.bits_ &= another.bits_;
        return copy;
    }

    constexpr EnumSet operator-(const EnumSet& another) const
    {
        return GetDifferenceFrom(another);
    }

    constexpr const auto& GetBitset() const
    {
        return bits_;
    }

    // clang-format off
    // STL
    auto begin() const { return Iterator(*this, bits_.CountContinuousZeroBits()); }
    auto end() const { return Iterator(*this, kCapacity); }

    // clang-format on

private:
    static constexpr size_t kCapacity = Converter::GetElementsCount();
    FixedBitset<kCapacity> bits_{};
};

template <typename Collection>
class EnumSetIterator
{
public:
    using ValueType = typename Collection::ValueType;
    using Converter = typename Collection::EnumConverter;

    constexpr explicit EnumSetIterator(const Collection& set, size_t index) : set_(set), index_(index) {}

    constexpr EnumSetIterator& operator++()
    {
        index_ = set_.bits_.CountContinuousZeroBits(index_ + 1);
        return *this;
    }

    constexpr bool operator==(const EnumSetIterator& another) const
    {
        return index_ == another.index_ && &set_ == &another.set_;
    }

    constexpr bool operator!=(const EnumSetIterator& another) const
    {
        return !(*this == another);
    }

    constexpr ValueType operator*() const
    {
        return Converter::ConvertIndexToEnum(index_);
    }

private:
    const Collection& set_;
    size_t index_;
};

template <typename Head, typename... Tail>
constexpr EnumSet<Head> MakeEnumSet(const Head head, const Tail... tail)
{
    EnumSet<Head> result{};
    result.Add(head);
    (result.Add(tail), ...);
    return result;
}

template <typename T>
constexpr EnumSet<T> MakeEnumSet()
{
    return EnumSet<T>{};
}
}  // namespace ass
