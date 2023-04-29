#include <type_traits>

#include "ass/enum/enum_as_index.hpp"
#include "ass/fixed_bitset.hpp"

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
    static constexpr size_t kCapacity = Converter::GetElementsCount();
    using Iterator = EnumSetIterator<T>;
    friend Iterator;

    constexpr void Add(const T value)
    {
        const size_t index = Converter::ConvertEnumToIndex(value);
        bits_.Set(index, true);
    }

    constexpr void Remove(const T value)
    {
        const size_t index = Converter::ConvertEnumToIndex(value);
        bits_.Set(index, false);
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

    constexpr bool IsEmpty() const
    {
        return Size() == 0;
    }

    // clang-format off
    // STL
    auto begin() const { return EnumSetIterator<T>(*this, bits_.CountContinuousZeroBits()); }
    auto end() const { return EnumSetIterator<T>(*this, kCapacity); }

    // clang-format on

private:
    FixedBitset<kCapacity> bits_{};
};

// TODO(sunday): This one must use collection type or also accept enum to index converter type
template <typename T>
class EnumSetIterator
{
public:
    constexpr explicit EnumSetIterator(const EnumSet<T>& set, size_t index) : set_(set), index_(index) {}

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

private:
    const EnumSet<T>& set_;
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