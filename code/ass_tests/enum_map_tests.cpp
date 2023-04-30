#include <algorithm>
#include <random>
#include <string_view>
#include <vector>

#include "ass/enum/enum_as_index.hpp"
#include "ass/enum_map.hpp"
#include "test_helpers.hpp"

enum class EnumMapTest_ContinuousEnum
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

enum class EnumMapTest_SparseEnum
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
    return std::array<EnumMapTest_SparseEnum, 10>{
        EnumMapTest_SparseEnum::A,
        EnumMapTest_SparseEnum::B,
        EnumMapTest_SparseEnum::C,
        EnumMapTest_SparseEnum::D,
        EnumMapTest_SparseEnum::E,
        EnumMapTest_SparseEnum::F,
        EnumMapTest_SparseEnum::G,
        EnumMapTest_SparseEnum::H,
        EnumMapTest_SparseEnum::J,
        EnumMapTest_SparseEnum::K};
}

struct TestParams_TrivialContinuous
{
    using KeyType = EnumMapTest_ContinuousEnum;
    using KeyConverter = ass::EnumIndexConverter_Continuous<KeyType, KeyType::A, KeyType::kMax>;
    using ValueType = size_t;
    static constexpr std::string_view GetName()
    {
        return "Trivial_Continuous";
    }
    static constexpr ValueType MakeValue(size_t index)
    {
        return (index + 1) * 3;
    }
};

struct TestParams_NonTrivialContinuous : TestParams_TrivialContinuous
{
    using ValueType = NonTrivialInteger<size_t>;
    static constexpr std::string_view GetName()
    {
        return "NonTrivial_Continuous";
    }
    static ValueType MakeValue(size_t index)
    {
        return ValueType((index + 1) * 4);
    }
};

struct TestParams_TrivialSparse
{
    using KeyType = EnumMapTest_SparseEnum;
    using KeyConverter = ass::EnumIndexConverter_Sparse<KeyType, GetSparseEnumValues>;
    using ValueType = size_t;
    static constexpr std::string_view GetName()
    {
        return "Trivial_Sparse";
    }
    static constexpr ValueType MakeValue(size_t index)
    {
        return (index + 1) * 3;
    }
};

struct TestParams_NonTrivialSparse : TestParams_TrivialSparse
{
    using ValueType = NonTrivialInteger<size_t>;
    static constexpr std::string_view GetName()
    {
        return "NonTrivial_Sparse";
    }
    static ValueType MakeValue(size_t index)
    {
        return ValueType((index + 1) * 4);
    }
};

template <typename TestParameters_>
class EnumMapTest : public testing::Test
{
public:
    using TestParameters = TestParameters_;
};

using EnumMapTest_Implementations = testing::Types<
    TestParams_TrivialContinuous,
    TestParams_TrivialSparse,
    TestParams_NonTrivialContinuous,
    TestParams_NonTrivialSparse>;

struct EnumMapTest_Names
{
    template <typename T>
    static std::string GetName(int)
    {
        return std::string(T::GetName());
    }
};

TYPED_TEST_SUITE(EnumMapTest, EnumMapTest_Implementations, EnumMapTest_Names);

TYPED_TEST(EnumMapTest, AddRemove)
{
    using Self = std::decay_t<decltype(*this)>;
    using Parameters = typename Self::TestParameters;
    using KeyType = typename Parameters::KeyType;
    using ValueType = typename Parameters::ValueType;
    using KeyConverter = typename Parameters::KeyConverter;
    using Map = ass::EnumMap<KeyType, ValueType, KeyConverter>;

    constexpr size_t keys_count = KeyConverter::GetElementsCount();

    std::array<KeyType, keys_count> shuffled_keys{};
    for (size_t i = 0; i != keys_count; ++i)
    {
        shuffled_keys[i] = KeyConverter::ConvertIndexToEnum(i);
    }

    auto make_value = [&](const KeyType key)
    {
        return Parameters::MakeValue(KeyConverter::ConvertEnumToIndex(key));
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
    using Params = TestParams_TrivialContinuous;
    using KeyType = typename Params::KeyType;
    using KeyConverter = typename Params::KeyConverter;
    using ValueType = ValueWithDestructor;
    ass::EnumMap<KeyType, ValueType, KeyConverter> map{};

    size_t dtor_counter = 0;
    map.Emplace(KeyType::A, &dtor_counter, 42u);
    ASSERT_EQ(dtor_counter, 0);
    map.Remove(KeyType::A);
    ASSERT_EQ(dtor_counter, 1);
}