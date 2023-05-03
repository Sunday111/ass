#include <algorithm>
#include <array>
#include <climits>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

#include "ass/fixed_unordered_map.hpp"
#include "test_helpers.hpp"

namespace fixed_map_tests
{

template <typename Key>
constexpr auto MakeKeyMaker()
{
    return [](size_t index)
    {
        return static_cast<Key>((static_cast<int>(index) + 3) * 7);
    };
}

template <typename Value>
constexpr auto MakeValueMaker()
{
    return [](size_t index)
    {
        return static_cast<Value>((static_cast<int>(index) - 5) * 7);
    };
}

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

struct ConstexprHasherCollisions
{
    constexpr ConstexprHasherCollisions() = default;

    template <typename T>
    constexpr size_t operator()(const T&) const
    {
        return 42;
    }
};

template <typename MapType_>
class FixedUnorderedMapTest : public testing::Test
{
public:
    using MapType = MapType_;
};

class FixedUnorderedMapTestNames
{
public:
    template <typename T>
    static std::string GetName(int)
    {
        constexpr bool kTrivialKey = std::is_trivially_destructible_v<typename T::Key>;
        constexpr bool kTrivialValue = std::is_trivially_destructible_v<typename T::Value>;
        std::string result;
        if constexpr (kTrivialKey ^ kTrivialValue)
        {
            if constexpr (kTrivialKey)
            {
                result = "NonTrivialVal";
            }
            else
            {
                result = "NonTrivialKey";
            }
        }
        else if constexpr (kTrivialKey)
        {
            result = "Trivial";
        }
        else
        {
            result = "NonTrivial";
        }

        result += "_n";
        result += std::to_string(T::Capacity());

        if constexpr (std::is_same_v<ConstexprHasherCollisions, typename T::Hasher>)
        {
            result += "_collisions";
        }

        return result;
    }
};

template <typename Capacity, typename Key, typename Value, typename Hasher>
using MapAlias = ass::FixedUnorderedMap<Capacity::kValue, Key, Value, Hasher>;

using test_helpers::TypedConstant;
using Implementations = test_helpers::GParametrizeWithCombinations<
    MapAlias,
    /*Capacity*/ std::tuple<TypedConstant<10>, TypedConstant<20>, TypedConstant<100>>,
    /* Keys */ std::tuple<int, NonTrivialInteger<int>>,
    /* Values */ std::tuple<int, NonTrivialInteger<int>>,
    /*Hashers*/ std::tuple<ConstexprHasher, ConstexprHasherCollisions>>;

TYPED_TEST_SUITE(FixedUnorderedMapTest, Implementations, FixedUnorderedMapTestNames);

TYPED_TEST(FixedUnorderedMapTest, Full)
{
    using Self = typename std::decay_t<decltype(*this)>;
    using Map = typename Self::MapType;
    using Key = typename Map::Key;
    using Value = typename Map::Value;
    auto make_key = MakeKeyMaker<Key>();
    auto make_value = MakeValueMaker<Value>();
    constexpr size_t Capacity = Map::Capacity();

    constexpr unsigned seed = 42;
    std::mt19937 rnd(seed);

    std::vector<size_t> shuffled_indices;
    for (size_t i = 0; i != Capacity; ++i)
    {
        shuffled_indices.push_back(i);
    }

    Map m;
    for (size_t iteration = 0; iteration != 10; ++iteration)
    {
        std::shuffle(shuffled_indices.begin(), shuffled_indices.end(), rnd);

        // Add elements
        for (size_t i = 0; i != Capacity; ++i)
        {
            {
                auto key = make_key(shuffled_indices[i]);
                auto value = make_value(shuffled_indices[i]);
                ASSERT_EQ(m.Size(), i);
                m.Add(key, value);
            }

            for (size_t j = 0; j <= i; ++j)
            {
                ASSERT_TRUE(m.Contains(make_key(shuffled_indices[j])));
            }

            for (size_t j = i + 1; j != Capacity; ++j)
            {
                ASSERT_FALSE(m.Contains(make_key(shuffled_indices[j])));
            }
        }

        // Remove elements
        std::shuffle(shuffled_indices.begin(), shuffled_indices.end(), rnd);
        for (size_t i = 0; i != Capacity; ++i)
        {
            auto key = make_key(shuffled_indices[i]);
            auto value = make_value(shuffled_indices[i]);
            ASSERT_NE(m.TryAdd(key), nullptr);
            ASSERT_EQ(*m.TryAdd(key), value);
            ASSERT_EQ(m.Get(key), value);
        }

        ASSERT_EQ(m.TryAdd(make_key(Capacity)), nullptr);

        for (size_t i = 0; i != Capacity; ++i)
        {
            auto key = make_key(shuffled_indices[i]);
            auto value = make_value(shuffled_indices[i]);
            ASSERT_EQ(m.Size(), Capacity - i);
            auto opt = m.Remove(key);
            ASSERT_TRUE(opt.has_value());
            ASSERT_EQ(*opt, value);
        }

        for (size_t i = 0; i != Capacity; ++i)
        {
            auto opt = m.Remove(make_key(i));
            ASSERT_FALSE(opt.has_value());
        }
    }
}

TYPED_TEST(FixedUnorderedMapTest, Iteration)
{
    using Self = typename std::decay_t<decltype(*this)>;
    using Map = typename Self::MapType;
    using Key = typename Map::Key;
    using Value = typename Map::Value;
    auto make_key = MakeKeyMaker<Key>();
    auto make_value = MakeValueMaker<Value>();
    static constexpr size_t Capacity = Map::Capacity();

    using Converted = std::array<std::pair<Key, Value>, Capacity>;

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
    for (size_t i = 0; i != Capacity; ++i)
    {
        map.Add(make_key(i), make_value(i));

        auto [converted, count] = convert(map);
        ASSERT_EQ(static_cast<int>(count), i + 1);

        for (size_t j = 0; j <= i; ++j)
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
#ifdef NDEBUG
#define ASS_ENSURE(expr) \
    if (!(expr))         \
    {                    \
        return false;    \
    }
#else
#define ASS_ENSURE(expr) assert(expr);
#endif

    constexpr size_t Capacity = 10;
    ass::FixedUnorderedMap<Capacity, int, int, ConstexprHasher> m{};

    auto make_key = MakeKeyMaker<int>();
    auto make_value = MakeValueMaker<int>();

    for (size_t i = 0; i != Capacity; ++i)
    {
        ASS_ENSURE(m.Size() == static_cast<size_t>(i));
        m.Add(make_key(i), make_value(i));

        for (size_t j = 0; j <= i; ++j)
        {
            ASS_ENSURE(m.Contains(make_key(j)));
        }

        for (size_t j = i + 1; j != Capacity; ++j)
        {
            ASS_ENSURE(!m.Contains(make_key(j)));
        }
    }

    for (size_t i = 0; i != Capacity; ++i)
    {
        ASS_ENSURE(m.Get(make_key(i)) == make_value(i));
    }

    ASS_ENSURE(m.TryAdd(make_key(10)) == nullptr);
    ASS_ENSURE(m.TryAdd(make_key(9)) != nullptr);
    ASS_ENSURE(*m.TryAdd(make_key(9)) == make_value(9));

    for (size_t i = 0; i != Capacity; ++i)
    {
        ASS_ENSURE(m.Size() == Capacity - static_cast<size_t>(i));
        auto opt = m.Remove(make_key(i));
        ASS_ENSURE(opt.has_value());
        ASS_ENSURE(*opt == make_value(i));
    }

    for (size_t i = 0; i != Capacity; ++i)
    {
        auto opt = m.Remove(make_key(i));
        ASS_ENSURE(!opt.has_value());
    }

    return true;
}

static constexpr bool ConstexprIteratorTest()
{
    constexpr size_t Capacity = 10;

    auto make_key = MakeKeyMaker<int>();
    auto make_value = MakeValueMaker<int>();
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
    for (size_t i = 0; i != Capacity; ++i)
    {
        map.Add(make_key(i), make_value(i));

        auto [converted, count] = convert(map);
        if (count != i + 1) return false;

        for (size_t j = 0; j <= i; ++j)
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
}  // namespace fixed_map_tests
