#include "ass/enum/enum_as_index.hpp"

#include <array>

#include "gtest/gtest.h"

enum class ContinuousUnsigned : unsigned
{
    A,
    B,
    C,
    D,

    kMax
};

template <>
struct ass::EnumIndexConverter<ContinuousUnsigned>
    : ass::EnumIndexConverter_Continuous<ContinuousUnsigned, ContinuousUnsigned::A, ContinuousUnsigned::kMax>
{
};

TEST(EnumIndexConverter, ContinuousUnsigned)
{
    using E = ContinuousUnsigned;
    using CV = ass::EnumIndexConverter<E>;
    ASSERT_EQ(CV::GetElementsCount(), 4);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::A), 0);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::B), 1);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::C), 2);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::D), 3);
    ASSERT_EQ(CV::ConvertIndexToEnum(0), E::A);
    ASSERT_EQ(CV::ConvertIndexToEnum(1), E::B);
    ASSERT_EQ(CV::ConvertIndexToEnum(2), E::C);
    ASSERT_EQ(CV::ConvertIndexToEnum(3), E::D);
}

enum class ContinuousSigned : int
{
    A = -10,
    B,
    C,
    D,

    kMax
};

template <>
struct ass::EnumIndexConverter<ContinuousSigned>
    : ass::EnumIndexConverter_Continuous<ContinuousSigned, ContinuousSigned::A, ContinuousSigned::kMax>
{
};

TEST(EnumIndexConverter, ContinuousSigned)
{
    using E = ContinuousSigned;
    using CV = ass::EnumIndexConverter<E>;
    ASSERT_EQ(CV::GetElementsCount(), 4);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::A), 0);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::B), 1);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::C), 2);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::D), 3);
    ASSERT_EQ(CV::ConvertIndexToEnum(0), E::A);
    ASSERT_EQ(CV::ConvertIndexToEnum(1), E::B);
    ASSERT_EQ(CV::ConvertIndexToEnum(2), E::C);
    ASSERT_EQ(CV::ConvertIndexToEnum(3), E::D);
}

enum class SparseEnumeration : int
{
    A = -22,
    B = -2,
    C = 22,
    D = 23,
    E = 42,
    F = 48,
    G = 59
};

inline constexpr auto get_sparse_enum_values()
{
    std::array<SparseEnumeration, 7> values{
        SparseEnumeration::A,
        SparseEnumeration::B,
        SparseEnumeration::C,
        SparseEnumeration::D,
        SparseEnumeration::E,
        SparseEnumeration::F,
        SparseEnumeration::G};

    return values;
}

template <>
struct ass::EnumIndexConverter<SparseEnumeration>
    : ass::EnumIndexConverter_Sparse<SparseEnumeration, get_sparse_enum_values>
{
};

TEST(EnumIndexConverter, SparseSigned)
{
    using E = SparseEnumeration;
    using CV = ass::EnumIndexConverter<E>;
    ASSERT_EQ(CV::GetElementsCount(), 7);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::A), 0);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::B), 1);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::C), 2);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::D), 3);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::E), 4);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::F), 5);
    ASSERT_EQ(CV::ConvertEnumToIndex(E::G), 6);
    ASSERT_EQ(CV::ConvertIndexToEnum(0), E::A);
    ASSERT_EQ(CV::ConvertIndexToEnum(1), E::B);
    ASSERT_EQ(CV::ConvertIndexToEnum(2), E::C);
    ASSERT_EQ(CV::ConvertIndexToEnum(3), E::D);
    ASSERT_EQ(CV::ConvertIndexToEnum(4), E::E);
    ASSERT_EQ(CV::ConvertIndexToEnum(5), E::F);
    ASSERT_EQ(CV::ConvertIndexToEnum(6), E::G);
}