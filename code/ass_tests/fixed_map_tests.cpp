#include <array>
#include <climits>
#include <limits>
#include <type_traits>
#include <vector>

#include "ass/fixed_unordered_map.hpp"
#include "gtest/gtest.h"

struct TrivialKeyValueTraits
{
    using Key = int;
    using Value = int;
    using Hasher = std::hash<int>;

    static constexpr Key MakeKey(int index)
    {
        return static_cast<Key>((index + 3) * 7);
    }

    static constexpr Value MakeValue(int index)
    {
        return static_cast<Value>((index - 5) * 7);
    }
};

template <typename T>
struct NonTrivialInteger
{
    constexpr NonTrivialInteger() = default;
    constexpr explicit NonTrivialInteger(int x) : value(x) {}
    constexpr NonTrivialInteger(NonTrivialInteger&&) noexcept = default;
    constexpr NonTrivialInteger(const NonTrivialInteger&) = default;
    constexpr NonTrivialInteger& operator=(const NonTrivialInteger&) = default;
    constexpr NonTrivialInteger& operator=(NonTrivialInteger&&) noexcept = default;

    ~NonTrivialInteger() {}

    explicit constexpr operator T() const
    {
        return value;
    }

    constexpr bool operator==(const NonTrivialInteger& another) const
    {
        return value == another.value;
    }

    T value{};
};

struct ConstexprHasher
{
    constexpr ConstexprHasher() = default;

    template <typename T>
    constexpr size_t operator()(const NonTrivialInteger<T>& v) const
    {
        int64_t l = std::numeric_limits<int>::lowest();
        int64_t v64 = v.value;
        return static_cast<size_t>(v64 - l);
    }

    constexpr size_t operator()(const int v) const
    {
        int64_t l = std::numeric_limits<int>::lowest();
        int64_t v64 = v;
        return static_cast<size_t>(v64 - l);
    }
};

static_assert(!std::is_trivially_destructible_v<NonTrivialInteger<int>>);

struct NonTrivialKeyValueTraits
{
    using Key = NonTrivialInteger<int>;
    using Value = NonTrivialInteger<int>;
    using Hasher = ConstexprHasher;

    static Key MakeKey(int index)
    {
        return Key((index + 3) * 7);
    }

    static Value MakeValue(int index)
    {
        return Value((index - 5) * 7);
    }
};

template <typename T>
class FixedUnorderedMapTest : public testing::Test, public T
{
public:
    using Self = FixedUnorderedMapTest<T>;
};

using Implementations = testing::Types<TrivialKeyValueTraits, NonTrivialKeyValueTraits>;

class FixedUnorderedMapTestNames
{
public:
    template <typename T>
    static std::string GetName(int)
    {
        if (std::is_same<T, TrivialKeyValueTraits>()) return "Trivial";
        if (std::is_same<T, NonTrivialKeyValueTraits>()) return "NonTrivial";
        return "";
    }
};
TYPED_TEST_SUITE(FixedUnorderedMapTest, Implementations, FixedUnorderedMapTestNames);

TYPED_TEST(FixedUnorderedMapTest, Full)
{
    constexpr size_t Capacity = 10;
    using Self = typename std::decay_t<decltype(*this)>::Self;
    using Key = typename Self::Key;
    using Value = typename Self::Value;
    using Hasher = typename Self::Hasher;
    auto make_key = Self::MakeKey;
    auto make_value = Self::MakeValue;

    ass::FixedUnorderedMap<Capacity, Key, Value, Hasher> m;

    for (int i = 0; i != Capacity; ++i)
    {
        ASSERT_EQ(m.Size(), i);
        m.Add(make_key(i), make_value(i));

        for (int j = 0; j <= i; ++j)
        {
            ASSERT_TRUE(m.Contains(make_key(j)));
        }

        for (int j = i + 1; j != Capacity; ++j)
        {
            ASSERT_FALSE(m.Contains(make_key(j)));
        }
    }

    for (int i = 0; i != Capacity; ++i)
    {
        ASSERT_EQ(m.Get(make_key(i)), make_value(i));
    }

    ASSERT_EQ(m.TryAdd(make_key(10)), nullptr);
    ASSERT_NE(m.TryAdd(make_key(9)), nullptr);
    ASSERT_EQ(*m.TryAdd(make_key(9)), make_value(9));

    for (int i = 0; i != Capacity; ++i)
    {
        ASSERT_EQ(m.Size(), Capacity - static_cast<size_t>(i));
        auto opt = m.Remove(make_key(i));
        ASSERT_TRUE(opt.has_value());
        ASSERT_EQ(*opt, make_value(i));
    }

    for (int i = 0; i != Capacity; ++i)
    {
        auto opt = m.Remove(make_key(i));
        ASSERT_FALSE(opt.has_value());
    }
}

TYPED_TEST(FixedUnorderedMapTest, Iteration)
{
    constexpr size_t Capacity = 10;

    using Self = typename std::decay_t<decltype(*this)>::Self;
    using Key = typename Self::Key;
    using Value = typename Self::Value;
    using Hasher = typename Self::Hasher;
    auto make_key = Self::MakeKey;
    auto make_value = Self::MakeValue;

    using Converted = std::array<std::pair<Key, Value>, Capacity>;
    using Map = ass::FixedUnorderedMap<Capacity, Key, Value, Hasher>;

    auto convert = [](auto& map) -> std::pair<Converted, size_t>
    {
        Converted r{};
        size_t index = 0;
        for (auto kv : map)
        {
            r[index].first = kv.key;
            r[index].second = kv.value;
            ++index;
        }
        return {r, index};
    };
    auto converted_has_kv = [](const Converted& map, const Key key, const Value value, size_t count) -> bool
    {
        for (size_t index = 0; index != count; ++index)
        {
            auto& [existing_key, existing_value] = map[index];
            if (key == existing_key)
            {
                return value == existing_value;
            }
        }
        return false;
    };

    Map map{};
    for (int i = 0; i != Capacity; ++i)
    {
        map.Add(make_key(i), make_value(i));

        auto [converted, count] = convert(map);
        ASSERT_EQ(static_cast<int>(count), i + 1);

        for (int j = 0; j <= i; ++j)
        {
            const Key key = make_key(j);
            const Value value = make_value(j);
            ASSERT_TRUE(converted_has_kv(converted, key, value, count)) << "i = " << i << ", j = " << j;
        }
    }

    // Now modify values of created map
    {
        int offset = 0;
        for (auto kv : map)
        {
            kv.value = Value(42 + offset);
            ++offset;
        }
    }

    // And ensure values were actually written
    {
        int offset = 0;
        for (auto kv : map)
        {
            ASSERT_EQ(kv.value, Value(42 + offset));
            ++offset;
        }
    }
}

static constexpr bool ConstexprTest()
{
    constexpr size_t Capacity = 10;
    ass::FixedUnorderedMap<Capacity, int, int, ConstexprHasher> m{};

    auto make_key = TrivialKeyValueTraits::MakeKey;
    auto make_value = TrivialKeyValueTraits::MakeValue;

    for (int i = 0; i != Capacity; ++i)
    {
        assert(m.Size() == static_cast<size_t>(i));
        m.Add(make_key(i), make_value(i));

        for (int j = 0; j <= i; ++j)
        {
            assert(m.Contains(make_key(j)));
        }

        for (int j = i + 1; j != Capacity; ++j)
        {
            assert(!m.Contains(make_key(j)));
        }
    }

    for (int i = 0; i != Capacity; ++i)
    {
        assert(m.Get(make_key(i)) == make_value(i));
    }

    assert(m.TryAdd(make_key(10)) == nullptr);
    assert(m.TryAdd(make_key(9)) != nullptr);
    assert(*m.TryAdd(make_key(9)) == make_value(9));

    for (int i = 0; i != Capacity; ++i)
    {
        assert(m.Size() == Capacity - static_cast<size_t>(i));
        auto opt = m.Remove(make_key(i));
        assert(opt.has_value());
        assert(*opt == make_value(i));
    }

    for (int i = 0; i != Capacity; ++i)
    {
        auto opt = m.Remove(make_key(i));
        assert(!opt.has_value());
    }

    return true;
}

static constexpr bool ConstexprIteratorTest()
{
    constexpr size_t Capacity = 10;

    auto make_key = TrivialKeyValueTraits::MakeKey;
    auto make_value = TrivialKeyValueTraits::MakeValue;
    using Key = decltype(make_key(0));
    using Value = decltype(make_value(0));

    using Converted = std::array<std::pair<Key, Value>, Capacity>;
    using Map = ass::FixedUnorderedMap<Capacity, int, int, ConstexprHasher>;

    auto convert = [](auto& map) -> std::pair<Converted, size_t>
    {
        Converted r{};
        size_t index = 0;
        for (auto kv : map)
        {
            r[index].first = kv.key;
            r[index].second = kv.value;
            ++index;
        }
        return {r, index};
    };
    auto converted_has_kv = [](const Converted& map, const Key key, const Value value, size_t count) -> bool
    {
        for (size_t index = 0; index != count; ++index)
        {
            auto& [existing_key, existing_value] = map[index];
            if (key == existing_key)
            {
                return value == existing_value;
            }
        }
        return false;
    };

    Map map{};
    for (int i = 0; i != Capacity; ++i)
    {
        map.Add(make_key(i), make_value(i));

        auto [converted, count] = convert(map);
        if (static_cast<int>(count) != i + 1) return false;

        for (int j = 0; j <= i; ++j)
        {
            const Key key = make_key(j);
            const Value value = make_value(j);
            if (!converted_has_kv(converted, key, value, count))
            {
                return false;
            }
        }
    }

    return true;
}

static_assert(ConstexprTest());
static_assert(ConstexprIteratorTest());

// TEST(FixedUnorderedMapConstexpr, Iterator)
// {
//     ASSERT_TRUE(ConstexprIteratorTest());
// }
