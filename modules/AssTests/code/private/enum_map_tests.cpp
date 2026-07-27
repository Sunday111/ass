#include <algorithm>
#include <array>
#include <memory>
#include <random>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ass/enum/enum_as_index.hpp"
#include "ass/enum_map.hpp"
#include "gtest/gtest.h"
#include "test_helpers.hpp"

namespace enum_map_tests
{
enum class ContinuousEnum
{
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    J,
    K,
    kMax
};

enum class SparseEnum
{
    A = 1,
    B = 2,
    C = 3,
    D = 5,
    E = 8,
    F = 13,
    G = 21,
    H = 34,
    J = 55,
    K = 88
};

static constexpr auto GetSparseEnumValues()
{
    return std::array<SparseEnum, 10>{
        SparseEnum::A,
        SparseEnum::B,
        SparseEnum::C,
        SparseEnum::D,
        SparseEnum::E,
        SparseEnum::F,
        SparseEnum::G,
        SparseEnum::H,
        SparseEnum::J,
        SparseEnum::K};
}

template <typename MapType_>
class EnumMapTest : public testing::Test
{
public:
    using MapType = MapType_;

    static constexpr size_t MakeValue(size_t index)
    {
        return (index + 1) * 3;
    }
};
template <typename KeyAndKeyConverter, typename Value>
using EnumMapAlias =
    ass::EnumMap<std::tuple_element_t<0, KeyAndKeyConverter>, Value, std::tuple_element_t<1, KeyAndKeyConverter>>;
using ContinuousConverter = ass::EnumIndexConverter_Continuous<ContinuousEnum, ContinuousEnum::A, ContinuousEnum::kMax>;
using ContinuousEnumProps = std::tuple<ContinuousEnum, ContinuousConverter>;
using SparseConverter = ass::EnumIndexConverter_Sparse<SparseEnum, GetSparseEnumValues>;
using SparseEnumProps = std::tuple<SparseEnum, SparseConverter>;
using Implementations = test_helpers::GParametrizeWithCombinations<
    EnumMapAlias,
    std::tuple<ContinuousEnumProps, SparseEnumProps>,
    std::tuple<size_t, NonTrivialInteger<size_t>>>;

struct TestsNames
{
    template <typename T>
    static std::string GetName(int)
    {
        constexpr bool kIsContinuous = (std::is_same_v<typename T::KeyType, ContinuousEnum>);
        constexpr bool kIsTrivial = std::is_trivially_destructible_v<typename T::ValueType>;
        std::string name = kIsContinuous ? "Continuous" : "Sparse";
        name += kIsTrivial ? "Trivial" : "NonTrivial";
        return name;
    }
};

TYPED_TEST_SUITE(EnumMapTest, Implementations, TestsNames);

TYPED_TEST(EnumMapTest, AddRemove)
{
    using Self = std::decay_t<decltype(*this)>;
    using Map = typename Self::MapType;
    using KeyType = typename Map::KeyType;
    using ValueType = typename Map::ValueType;
    using KeyConverter = typename Map::KeyConverter;
    constexpr size_t keys_count = Map::Capacity();

    std::array<KeyType, keys_count> shuffled_keys{};
    for (size_t i = 0; i != keys_count; ++i)
    {
        shuffled_keys[i] = KeyConverter::ConvertIndexToEnum(i);
    }

    auto make_value = [&](const KeyType key)
    {
        return ValueType(Self::MakeValue(KeyConverter::ConvertEnumToIndex(key)));
    };

    constexpr unsigned kSeed = 1234;
    std::mt19937 rnd(kSeed);

    std::shuffle(shuffled_keys.begin(), shuffled_keys.end(), rnd);

    Map map;
    for (size_t i = 0; i != keys_count; ++i)
    {
        {
            const KeyType key = shuffled_keys[i];
            ASSERT_FALSE(map.Contains(key));
            map.GetOrAdd(key, make_value(key));
            ASSERT_TRUE(map.Contains(key));
        }

        // Ensure that values for previous keys remain the same
        for (size_t j = 0; j != i; ++j)
        {
            const KeyType key = shuffled_keys[j];
            ASSERT_TRUE(map.Contains(key));
            ASSERT_EQ(map.Get(key), make_value(key));
        }

        // Ensure that remaining keys are not in the map yet
        for (size_t j = i + 1; j != keys_count; ++j)
        {
            ASSERT_FALSE(map.Contains(shuffled_keys[j]));
        }
    }

    std::shuffle(shuffled_keys.begin(), shuffled_keys.end(), rnd);

    for (size_t i = 0; i != keys_count; ++i)
    {
        {
            const KeyType key = shuffled_keys[i];
            ASSERT_TRUE(map.Contains(key));
            ASSERT_EQ(map.Get(key), make_value(key));
            auto removed_value = map.Remove(key);
            ASSERT_TRUE(removed_value.has_value());
        }

        // Ensure that previously removed values did not magically appeared
        for (size_t j = 0; j != i; ++j)
        {
            const KeyType key = shuffled_keys[j];
            ASSERT_FALSE(map.Contains(key));
        }

        // Ensure that other keys are still there
        for (size_t j = i + 1; j != keys_count; ++j)
        {
            const KeyType key = shuffled_keys[j];
            ASSERT_TRUE(map.Contains(key));
            ASSERT_EQ(map.Get(key), make_value(key));
        }
    }
}

TYPED_TEST(EnumMapTest, Iteration)
{
    using Self = std::decay_t<decltype(*this)>;
    using Map = typename Self::MapType;
    using KeyType = typename Map::KeyType;
    using ValueType = typename Map::ValueType;
    using KeyConverter = typename Map::KeyConverter;
    constexpr size_t keys_count = Map::Capacity();

    std::array<KeyType, keys_count> shuffled_keys{};
    for (size_t i = 0; i != keys_count; ++i)
    {
        shuffled_keys[i] = KeyConverter::ConvertIndexToEnum(i);
    }

    auto make_value = [&](const KeyType key)
    {
        return ValueType(Self::MakeValue(KeyConverter::ConvertEnumToIndex(key)));
    };

    constexpr unsigned kSeed = 1234;
    std::mt19937 rnd(kSeed);

    std::shuffle(shuffled_keys.begin(), shuffled_keys.end(), rnd);

    auto convert_map = [](const Map& m)
    {
        std::array<std::pair<KeyType, ValueType>, keys_count> converted{};
        size_t index = 0;
        for (auto kv : m)
        {
            converted[index].first = kv.key;
            converted[index].second = kv.value;
            index++;
        }

        return std::pair{converted, index};
    };

    auto converted_map_has_kv = [](const std::array<std::pair<KeyType, ValueType>, keys_count>& converted,
                                   size_t count,
                                   KeyType key,
                                   ValueType value)
    {
        for (size_t i = 0; i != count; ++i)
        {
            for (auto& kv : converted)
            {
                if (kv.first == key && kv.second == value)
                {
                    return true;
                }
            }
        }

        return false;
    };

    Map m;
    for (size_t i = 0; i != shuffled_keys.size(); ++i)
    {
        {
            const KeyType key = shuffled_keys[i];
            m.GetOrAdd(key, make_value(key));
        }

        auto [converted, converted_size] = convert_map(m);

        for (size_t j = 0; j <= i; ++j)
        {
            const KeyType key = shuffled_keys[j];
            const ValueType value = make_value(key);
            ASSERT_TRUE(converted_map_has_kv(converted, converted_size, key, value));
        }

        for (size_t j = i + 1; j != shuffled_keys.size(); ++j)
        {
            const KeyType key = shuffled_keys[j];
            const ValueType value = make_value(key);
            ASSERT_FALSE(converted_map_has_kv(converted, converted_size, key, value));
        }
    }

    for (auto kv : m)
    {
        kv.value *= ValueType(2);
    }

    {
        auto [converted, converted_size] = convert_map(m);
        ASSERT_EQ(converted.size(), shuffled_keys.size());
        for (size_t j = 0; j != shuffled_keys.size(); ++j)
        {
            const KeyType key = shuffled_keys[j];
            const ValueType value = make_value(key) * ValueType(2);
            ASSERT_TRUE(converted_map_has_kv(converted, converted_size, key, value));
        }
    }
}

class ValueWithDestructor
{
public:
    ValueWithDestructor(size_t* counter, size_t value) : destructor_calls_counter_(counter), value_(value) {}
    ValueWithDestructor(const ValueWithDestructor&) = delete;
    ValueWithDestructor(ValueWithDestructor&& another) noexcept = delete;
    ValueWithDestructor& operator=(const ValueWithDestructor&) = delete;
    ValueWithDestructor& operator=(ValueWithDestructor&& another) noexcept = delete;
    ~ValueWithDestructor()
    {
        if (destructor_calls_counter_)
        {
            *destructor_calls_counter_ += 1;
        }
    }

    size_t* destructor_calls_counter_ = nullptr;
    size_t value_;
};

TEST(EnumMap, ObjectDestructionTest)
{
    using KeyType = ContinuousEnum;
    using KeyConverter = ContinuousConverter;
    using ValueType = std::unique_ptr<ValueWithDestructor>;
    ass::EnumMap<KeyType, ValueType, KeyConverter> map{};

    size_t dtor_counter = 0;
    map.Emplace(KeyType::A, std::make_unique<ValueWithDestructor>(&dtor_counter, 42u));
    ASSERT_EQ(dtor_counter, 0);
    {
        auto removed_object = map.Remove(KeyType::A);
        ASSERT_TRUE(removed_object.has_value());
        ASSERT_NE(removed_object->get(), nullptr);
        ASSERT_EQ(removed_object->get()->value_, 42);
        ASSERT_EQ(dtor_counter, 0);
    }
    ASSERT_EQ(dtor_counter, 1);
}

}  // namespace enum_map_tests
