#pragma once

#include <cstddef>
#include <type_traits>

namespace ass::fixed_unordered_map_detail::non_trivially_destructible
{

template <typename Collection>
class FixedUnorderedMapIterator
{
public:
    using Self = FixedUnorderedMapIterator;
    using CleanCollection = std::remove_const_t<Collection>;
    using Key = typename CleanCollection::Key;
    using Value = std::conditional_t<
        std::is_const_v<Collection>,
        std::add_const_t<typename CleanCollection::Value>,
        typename CleanCollection::Value>;

    struct KeyValue
    {
        const Key& key;
        Value& value;
    };

    explicit FixedUnorderedMapIterator(Collection& collection, size_t index) : collection_(collection), index_(index) {}

    FixedUnorderedMapIterator(const FixedUnorderedMapIterator&) noexcept = default;
    FixedUnorderedMapIterator(FixedUnorderedMapIterator&&) noexcept = default;
    FixedUnorderedMapIterator& operator=(const FixedUnorderedMapIterator&) noexcept = default;
    FixedUnorderedMapIterator& operator=(FixedUnorderedMapIterator&&) noexcept = default;
    ~FixedUnorderedMapIterator() noexcept = default;

    bool operator==(const FixedUnorderedMapIterator& another) const noexcept
    {
        return &collection_ == &another.collection_ && index_ == another.index_;
    }

    bool operator!=(const FixedUnorderedMapIterator& another) const noexcept
    {
        return !(*this == another);
    }

    FixedUnorderedMapIterator& operator++() noexcept
    {
        index_ = collection_.GetNextIndexWithValue(index_);
        return *this;
    }

    KeyValue operator*() const noexcept
    {
        auto& key = collection_.GetKeyAt(index_);
        auto& value = collection_.GetValueAt(index_);
        return KeyValue{key, value};
    }

private:
    Collection& collection_;
    size_t index_ = 0;
};
}  // namespace ass::fixed_unordered_map_detail::non_trivially_destructible
