#pragma once

#include <cstddef>
#include <type_traits>

namespace ass::fixed_unordered_map_detail
{

template <typename Collection>
class FixedMapIteratorTrivial
{
public:
    using Self = FixedMapIteratorTrivial;
    using CleanCollection = std::remove_const_t<Collection>;
    using Key = typename CleanCollection::Key;
    using Value = std::conditional_t<
        std::is_const_v<Collection>,
        typename CleanCollection::Value,
        std::add_const_t<typename CleanCollection::Value>>;

    struct KeyValue
    {
        const Key& key;
        Value& value;
    };

    constexpr explicit FixedMapIteratorTrivial(Collection& collection, size_t index)
        : collection_(collection),
          index_(index)
    {
    }

    constexpr FixedMapIteratorTrivial(const FixedMapIteratorTrivial&) noexcept = default;
    constexpr FixedMapIteratorTrivial(FixedMapIteratorTrivial&&) noexcept = default;
    constexpr FixedMapIteratorTrivial& operator=(const FixedMapIteratorTrivial&) noexcept = default;
    constexpr FixedMapIteratorTrivial& operator=(FixedMapIteratorTrivial&&) noexcept = default;
    ~FixedMapIteratorTrivial() noexcept = default;

    constexpr bool operator==(const FixedMapIteratorTrivial& another) const noexcept
    {
        return &collection_ == &another.collection_ && index_ == another.index_;
    }

    constexpr bool operator!=(const FixedMapIteratorTrivial& another) const noexcept
    {
        return !(*this == another);
    }

    constexpr FixedMapIteratorTrivial& operator++() noexcept
    {
        index_ = collection_.GetNextIndexWithValue(index_);
        return *this;
    }

    constexpr KeyValue operator*() const noexcept
    {
        auto& key = collection_.GetKeyAt(index_);
        auto& value = collection_.GetValueAt(index_);
        return KeyValue{key, value};
    }

private:
    Collection& collection_;
    size_t index_ = 0;
};
}  // namespace ass::fixed_unordered_map_detail
