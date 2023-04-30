#include <algorithm>
#include <random>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "ass/enum/enum_as_index.hpp"
#include "ass/enum_map.hpp"
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
}  // namespace enum_map_tests

struct TestKeyParams_Continuous
{
    using KeyType = enum_map_tests::ContinuousEnum;
    using KeyConverter = ass::EnumIndexConverter_Continuous<KeyType, KeyType::A, KeyType::kMax>;
    static constexpr std::string_view GetName()
    {
        return "Continuous";
    }
};

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

namespace enum_map_tests
{

template <typename KeyAndKeyConverter, typename Value>
using EnumMapAlias =
    ass::EnumMap<std::tuple_element_t<0, KeyAndKeyConverter>, Value, std::tuple_element_t<1, KeyAndKeyConverter>>;
using ContinuousEnumProps = std::
    tuple<ContinuousEnum, ass::EnumIndexConverter_Continuous<ContinuousEnum, ContinuousEnum::A, ContinuousEnum::kMax>>;
using SparseEnumProps = std::tuple<SparseEnum, ass::EnumIndexConverter_Sparse<SparseEnum, GetSparseEnumValues>>;
using Implementations = test_helpers::TupleToGoogleTestTypes<test_helpers::ParametrizeWithCombinations<
    EnumMapAlias,
    std::tuple<ContinuousEnumProps, SparseEnumProps>,
    std::tuple<size_t, NonTrivialInteger<size_t>>>>;

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
}  // namespace enum_map_tests

TYPED_TEST_SUITE(EnumMapTest, enum_map_tests::Implementations, enum_map_tests::TestsNames);

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

class ValueWithDestructor
{
public:
    ValueWithDestructor(size_t* counter, size_t value) : destructor_calls_counter_(counter), value_(value) {}
    ValueWithDestructor(const ValueWithDestructor&) = delete;
    ValueWithDestructor(ValueWithDestructor&& another) noexcept
        : destructor_calls_counter_(another.destructor_calls_counter_),
          value_(another.value_)
    {
        another.destructor_calls_counter_ = nullptr;
    }
    ValueWithDestructor& operator=(const ValueWithDestructor&) = delete;
    ValueWithDestructor& operator=(ValueWithDestructor&& another) noexcept
    {
        destructor_calls_counter_ = another.destructor_calls_counter_;
        value_ = another.value_;

        another.destructor_calls_counter_ = nullptr;
        return *this;
    }
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
    using KeyParams = TestKeyParams_Continuous;
    using KeyType = typename KeyParams::KeyType;
    using KeyConverter = typename KeyParams::KeyConverter;
    using ValueType = ValueWithDestructor;
    ass::EnumMap<KeyType, ValueType, KeyConverter> map{};

    size_t dtor_counter = 0;
    map.Emplace(KeyType::A, &dtor_counter, 42u);
    ASSERT_EQ(dtor_counter, 0);
    map.Remove(KeyType::A);
    ASSERT_EQ(dtor_counter, 1);
}