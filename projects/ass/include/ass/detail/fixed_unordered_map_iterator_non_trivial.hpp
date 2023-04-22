#pragma once

#include <cstddef>
#include <type_traits>

namespace ass::fixed_unordered_map_detail
{

template <typename Collection>
class FixedMapIteratorNonTrivial
{
public:
    using Self = FixedMapIteratorNonTrivial;
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

    explicit FixedMapIteratorNonTrivial(Collection& collection, size_t index) : collection_(collection), index_(index)
    {
    }

    FixedMapIteratorNonTrivial(const FixedMapIteratorNonTrivial&) noexcept = default;
    FixedMapIteratorNonTrivial(FixedMapIteratorNonTrivial&&) noexcept = default;
    FixedMapIteratorNonTrivial& operator=(const FixedMapIteratorNonTrivial&) noexcept = default;
    FixedMapIteratorNonTrivial& operator=(FixedMapIteratorNonTrivial&&) noexcept = default;
    ~FixedMapIteratorNonTrivial() noexcept = default;

    bool operator==(const FixedMapIteratorNonTrivial& another) const noexcept
    {
        return &collection_ == &another.collection_ && index_ == another.index_;
    }

    bool operator!=(const FixedMapIteratorNonTrivial& another) const noexcept
    {
        return !(*this == another);
    }

    FixedMapIteratorNonTrivial& operator++() noexcept
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
}  // namespace ass::fixed_unordered_map_detail
