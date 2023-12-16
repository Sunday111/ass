#pragma once

#include <cstddef>
#include <type_traits>

namespace ass::fixed_unordered_map_detail::trivially_destructible
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
        const Key& key;  // NOLINT
        Value& value;    // NOLINT
    };

    constexpr explicit FixedUnorderedMapIterator(Collection& collection, size_t index)
        : collection_(&collection),
          index_(index)
    {
    }

    constexpr FixedUnorderedMapIterator(const FixedUnorderedMapIterator&) noexcept = default;
    constexpr FixedUnorderedMapIterator(FixedUnorderedMapIterator&&) noexcept = default;
    constexpr FixedUnorderedMapIterator& operator=(const FixedUnorderedMapIterator&) noexcept = default;
    constexpr FixedUnorderedMapIterator& operator=(FixedUnorderedMapIterator&&) noexcept = default;
    ~FixedUnorderedMapIterator() noexcept = default;

    constexpr bool operator==(const FixedUnorderedMapIterator& another) const noexcept
    {
        return collection_ == another.collection_ && index_ == another.index_;
    }

    constexpr bool operator!=(const FixedUnorderedMapIterator& another) const noexcept
    {
        return !(*this == another);
    }

    constexpr FixedUnorderedMapIterator& operator++() noexcept
    {
        index_ = collection_->GetNextIndexWithValue(index_);
        return *this;
    }

    constexpr KeyValue operator*() const noexcept
    {
        auto& key = collection_->GetKeyAt(index_);
        auto& value = collection_->GetValueAt(index_);
        return KeyValue{key, value};
    }

private:
    Collection* collection_ = nullptr;
    size_t index_ = 0;
};
}  // namespace ass::fixed_unordered_map_detail::trivially_destructible
