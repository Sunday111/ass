#include "ass/enum/enum_as_index.hpp"
#include "ass/enum_set.hpp"
#include "gtest/gtest.h"

#define DEFINE_CONTINUOUS_ENUM_INDEX_CONVERTER(type, first_value)                                             \
    namespace ass                                                                                             \
    {                                                                                                         \
    template <>                                                                                               \
    struct EnumIndexConverter<type> : ass::EnumIndexConverter_Continuous<type, type::first_value, type::kMax> \
    {                                                                                                         \
    };                                                                                                        \
    }                                                                                                         \
    static_assert(true)

namespace ass::enum_set_tests
{

enum class MyEnum
{
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    kMax
};

}  // namespace ass::enum_set_tests

DEFINE_CONTINUOUS_ENUM_INDEX_CONVERTER(ass::enum_set_tests::MyEnum, A);

namespace ass::enum_set_tests
{
TEST(EnumSetTests, AddRemove)
{
    EnumSet<MyEnum> set;

    set.Add(MyEnum::A);
    ASSERT_TRUE(set.Contains(MyEnum::A));
    ASSERT_FALSE(set.Contains(MyEnum::B));
    ASSERT_FALSE(set.Contains(MyEnum::C));
    ASSERT_FALSE(set.Contains(MyEnum::D));
    ASSERT_FALSE(set.Contains(MyEnum::E));
    ASSERT_FALSE(set.Contains(MyEnum::F));
    ASSERT_FALSE(set.Contains(MyEnum::G));

    set.Add(MyEnum::C);
    ASSERT_TRUE(set.Contains(MyEnum::A));
    ASSERT_FALSE(set.Contains(MyEnum::B));
    ASSERT_TRUE(set.Contains(MyEnum::C));
    ASSERT_FALSE(set.Contains(MyEnum::D));
    ASSERT_FALSE(set.Contains(MyEnum::E));
    ASSERT_FALSE(set.Contains(MyEnum::F));
    ASSERT_FALSE(set.Contains(MyEnum::G));

    set.Add(MyEnum::E);
    ASSERT_TRUE(set.Contains(MyEnum::A));
    ASSERT_FALSE(set.Contains(MyEnum::B));
    ASSERT_TRUE(set.Contains(MyEnum::C));
    ASSERT_FALSE(set.Contains(MyEnum::D));
    ASSERT_TRUE(set.Contains(MyEnum::E));
    ASSERT_FALSE(set.Contains(MyEnum::F));
    ASSERT_FALSE(set.Contains(MyEnum::G));

    set.Add(MyEnum::G);
    ASSERT_TRUE(set.Contains(MyEnum::A));
    ASSERT_FALSE(set.Contains(MyEnum::B));
    ASSERT_TRUE(set.Contains(MyEnum::C));
    ASSERT_FALSE(set.Contains(MyEnum::D));
    ASSERT_TRUE(set.Contains(MyEnum::E));
    ASSERT_FALSE(set.Contains(MyEnum::F));
    ASSERT_TRUE(set.Contains(MyEnum::G));

    set.Add(MyEnum::B);
    ASSERT_TRUE(set.Contains(MyEnum::A));
    ASSERT_TRUE(set.Contains(MyEnum::B));
    ASSERT_TRUE(set.Contains(MyEnum::C));
    ASSERT_FALSE(set.Contains(MyEnum::D));
    ASSERT_TRUE(set.Contains(MyEnum::E));
    ASSERT_FALSE(set.Contains(MyEnum::F));
    ASSERT_TRUE(set.Contains(MyEnum::G));

    set.Add(MyEnum::D);
    ASSERT_TRUE(set.Contains(MyEnum::A));
    ASSERT_TRUE(set.Contains(MyEnum::B));
    ASSERT_TRUE(set.Contains(MyEnum::C));
    ASSERT_TRUE(set.Contains(MyEnum::D));
    ASSERT_TRUE(set.Contains(MyEnum::E));
    ASSERT_FALSE(set.Contains(MyEnum::F));
    ASSERT_TRUE(set.Contains(MyEnum::G));

    set.Add(MyEnum::F);
    ASSERT_TRUE(set.Contains(MyEnum::A));
    ASSERT_TRUE(set.Contains(MyEnum::B));
    ASSERT_TRUE(set.Contains(MyEnum::C));
    ASSERT_TRUE(set.Contains(MyEnum::D));
    ASSERT_TRUE(set.Contains(MyEnum::E));
    ASSERT_TRUE(set.Contains(MyEnum::F));
    ASSERT_TRUE(set.Contains(MyEnum::G));

    set.Remove(MyEnum::A);
    ASSERT_FALSE(set.Contains(MyEnum::A));
    ASSERT_TRUE(set.Contains(MyEnum::B));
    ASSERT_TRUE(set.Contains(MyEnum::C));
    ASSERT_TRUE(set.Contains(MyEnum::D));
    ASSERT_TRUE(set.Contains(MyEnum::E));
    ASSERT_TRUE(set.Contains(MyEnum::F));
    ASSERT_TRUE(set.Contains(MyEnum::G));

    set.Remove(MyEnum::B);
    ASSERT_FALSE(set.Contains(MyEnum::A));
    ASSERT_FALSE(set.Contains(MyEnum::B));
    ASSERT_TRUE(set.Contains(MyEnum::C));
    ASSERT_TRUE(set.Contains(MyEnum::D));
    ASSERT_TRUE(set.Contains(MyEnum::E));
    ASSERT_TRUE(set.Contains(MyEnum::F));
    ASSERT_TRUE(set.Contains(MyEnum::G));

    set.Remove(MyEnum::C);
    ASSERT_FALSE(set.Contains(MyEnum::A));
    ASSERT_FALSE(set.Contains(MyEnum::B));
    ASSERT_FALSE(set.Contains(MyEnum::C));
    ASSERT_TRUE(set.Contains(MyEnum::D));
    ASSERT_TRUE(set.Contains(MyEnum::E));
    ASSERT_TRUE(set.Contains(MyEnum::F));
    ASSERT_TRUE(set.Contains(MyEnum::G));

    set.Remove(MyEnum::D);
    ASSERT_FALSE(set.Contains(MyEnum::A));
    ASSERT_FALSE(set.Contains(MyEnum::B));
    ASSERT_FALSE(set.Contains(MyEnum::C));
    ASSERT_FALSE(set.Contains(MyEnum::D));
    ASSERT_TRUE(set.Contains(MyEnum::E));
    ASSERT_TRUE(set.Contains(MyEnum::F));
    ASSERT_TRUE(set.Contains(MyEnum::G));

    set.Remove(MyEnum::E);
    ASSERT_FALSE(set.Contains(MyEnum::A));
    ASSERT_FALSE(set.Contains(MyEnum::B));
    ASSERT_FALSE(set.Contains(MyEnum::C));
    ASSERT_FALSE(set.Contains(MyEnum::D));
    ASSERT_FALSE(set.Contains(MyEnum::E));
    ASSERT_TRUE(set.Contains(MyEnum::F));
    ASSERT_TRUE(set.Contains(MyEnum::G));

    set.Remove(MyEnum::F);
    ASSERT_FALSE(set.Contains(MyEnum::A));
    ASSERT_FALSE(set.Contains(MyEnum::B));
    ASSERT_FALSE(set.Contains(MyEnum::C));
    ASSERT_FALSE(set.Contains(MyEnum::D));
    ASSERT_FALSE(set.Contains(MyEnum::E));
    ASSERT_FALSE(set.Contains(MyEnum::F));
    ASSERT_TRUE(set.Contains(MyEnum::G));

    set.Remove(MyEnum::G);
    ASSERT_FALSE(set.Contains(MyEnum::A));
    ASSERT_FALSE(set.Contains(MyEnum::B));
    ASSERT_FALSE(set.Contains(MyEnum::C));
    ASSERT_FALSE(set.Contains(MyEnum::D));
    ASSERT_FALSE(set.Contains(MyEnum::E));
    ASSERT_FALSE(set.Contains(MyEnum::F));
    ASSERT_FALSE(set.Contains(MyEnum::G));
}

TEST(EnumSetTests, Invert)
{
    EnumSet<MyEnum> set;
    ASSERT_FALSE(set.Contains(MyEnum::A));
    ASSERT_FALSE(set.Contains(MyEnum::B));
    ASSERT_FALSE(set.Contains(MyEnum::C));
    ASSERT_FALSE(set.Contains(MyEnum::D));
    ASSERT_FALSE(set.Contains(MyEnum::E));
    ASSERT_FALSE(set.Contains(MyEnum::F));
    ASSERT_FALSE(set.Contains(MyEnum::G));

    set.Invert();
    ASSERT_TRUE(set.Contains(MyEnum::A));
    ASSERT_TRUE(set.Contains(MyEnum::B));
    ASSERT_TRUE(set.Contains(MyEnum::C));
    ASSERT_TRUE(set.Contains(MyEnum::D));
    ASSERT_TRUE(set.Contains(MyEnum::E));
    ASSERT_TRUE(set.Contains(MyEnum::F));
    ASSERT_TRUE(set.Contains(MyEnum::G));
}

TEST(EnumSetTests, MakeEnumSet)
{
    constexpr auto s = MakeEnumSet(MyEnum::A, MyEnum::D);
    ASSERT_TRUE(s.Contains(MyEnum::A));
    ASSERT_FALSE(s.Contains(MyEnum::B));
    ASSERT_FALSE(s.Contains(MyEnum::C));
    ASSERT_TRUE(s.Contains(MyEnum::D));
    ASSERT_FALSE(s.Contains(MyEnum::E));
    ASSERT_FALSE(s.Contains(MyEnum::F));
    ASSERT_FALSE(s.Contains(MyEnum::G));

    auto make = [](auto... values)
    {
        return MakeEnumSet<MyEnum>(values...);
    };

    static_assert(make().Size() == 0);
    static_assert(make(MyEnum::D).Size() == 1);
    static_assert(make(MyEnum::D, MyEnum::A).Size() == 2);
}

}  // namespace ass::enum_set_tests