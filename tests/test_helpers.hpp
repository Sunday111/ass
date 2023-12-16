#pragma once

#include <type_traits>

#include "gtest/gtest.h"

template <typename T>
struct NonTrivialInteger
{
    constexpr NonTrivialInteger() = default;
    constexpr explicit NonTrivialInteger(T x) : value(x) {}
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

    constexpr NonTrivialInteger& operator*=(const NonTrivialInteger& arg)
    {
        value *= arg.value;
        return *this;
    }

    constexpr NonTrivialInteger operator*(const NonTrivialInteger& arg) const
    {
        auto copy = *this;
        copy *= arg;
        return copy;
    }

    constexpr NonTrivialInteger operator/=(const NonTrivialInteger& arg)
    {
        value /= arg.value;
        return *this;
    }

    constexpr NonTrivialInteger operator/(const NonTrivialInteger& arg)
    {
        auto copy = *this;
        copy /= arg;
        return *this;
    }

    T value{};
};

static_assert(!std::is_trivially_destructible_v<NonTrivialInteger<int>>);

namespace test_helpers
{
namespace self_test
{
struct A
{
};
struct B
{
};
struct C
{
};
template <typename, typename>
struct MyTemplate2
{
};
template <typename, typename, typename>
struct MyTemplate3
{
};
}  // namespace self_test

template <typename T, typename TupleOfTuples>
struct AppendOneForEachElement;

template <typename T, typename... Tuples>
struct AppendOneForEachElement<T, std::tuple<Tuples...>>
{
    template <typename Tuple>
    using AppendToTuple = decltype(std::tuple_cat(std::tuple<T>{}, Tuple{}));
    using Result = std::tuple<AppendToTuple<Tuples>...>;
};

namespace self_test
{
// clang-format off
static_assert(std::is_same_v<
    typename AppendOneForEachElement<A, std::tuple<
        std::tuple<A, B, C>,
        std::tuple<B, A, C>
    >>::Result,
    std::tuple<
        std::tuple<A, A, B, C>,
        std::tuple<A, B, A, C>
    >
>);
// clang-format on
}  // namespace self_test

template <typename AppendValues, typename TargetTuples>
struct AppendManyForEachElement;

template <typename... AppendValues, typename TargetTuples>
struct AppendManyForEachElement<std::tuple<AppendValues...>, TargetTuples>
{
    template <typename AppendValue>
    using AppendOne = typename AppendOneForEachElement<AppendValue, TargetTuples>::Result;
    using Result = decltype(std::tuple_cat(AppendOne<AppendValues>{}...));
};

namespace self_test
{
// clang-format off
static_assert(std::is_same_v<
    typename AppendManyForEachElement<
        std::tuple<C, A>,
        std::tuple<
            std::tuple<A, B, C>,
            std::tuple<B, A, C>
        >
    >::Result,
    std::tuple<
        std::tuple<C, A, B, C>,
        std::tuple<C, B, A, C>,
        std::tuple<A, A, B, C>,
        std::tuple<A, B, A, C>
    >
>);
// clang-format on
}  // namespace self_test

template <typename Enable = void, typename... ArgsVariants>
struct MakeCombinationsImpl;

template <typename... FirstArgs, typename... SecondArgs>
struct MakeCombinationsImpl<void, std::tuple<FirstArgs...>, std::tuple<SecondArgs...>>
{
    template <typename FirstArg>
    using CombineWithSecond = std::tuple<std::tuple<FirstArg, SecondArgs>...>;
    using Result = decltype(std::tuple_cat(CombineWithSecond<FirstArgs>{}...));
};

template <typename Head, typename... Tail>
struct MakeCombinationsImpl<std::enable_if_t<(sizeof...(Tail) > 1)>, Head, Tail...>
{
    using TailCombinations = typename MakeCombinationsImpl<void, Tail...>::Result;
    using Result = typename AppendManyForEachElement<Head, TailCombinations>::Result;
};

// Makes all combinations of elements from first and second list
template <typename... ArgsVariants>
using MakeCombinations = typename MakeCombinationsImpl<void, ArgsVariants...>::Result;

namespace self_test
{
// clang-format off
static_assert(std::is_same_v<
    MakeCombinations<
        std::tuple<A, B>,
        std::tuple<B, C>
    >,
    std::tuple<
        std::tuple<A, B>,
        std::tuple<A, C>,
        std::tuple<B, B>,
        std::tuple<B, C>
    >
>);

static_assert(std::is_same_v<
    MakeCombinations<
        std::tuple<A, B>,
        std::tuple<B, C>,
        std::tuple<C, A>
    >,
    std::tuple<
        std::tuple<A, B, C>,
        std::tuple<A, B, A>,
        std::tuple<A, C, C>,
        std::tuple<A, C, A>,
        std::tuple<B, B, C>,
        std::tuple<B, B, A>,
        std::tuple<B, C, C>,
        std::tuple<B, C, A>
    >
>);
// clang-format on
}  // namespace self_test

template <template <typename...> typename Template, typename ArgsList>
struct ParametrizeOneCombination;

template <template <typename...> typename Template, typename... Args>
struct ParametrizeOneCombination<Template, std::tuple<Args...>>
{
    using Result = Template<Args...>;
};

namespace self_test
{
// clang-format off
static_assert(std::is_same_v<
    typename ParametrizeOneCombination<
        MyTemplate2,
        std::tuple<A, B>
    >::Result,
    MyTemplate2<A, B>
>);
static_assert(std::is_same_v<
    typename ParametrizeOneCombination<
        MyTemplate3,
        std::tuple<A, B, C>
    >::Result,
    MyTemplate3<A, B, C>
>);
// clang-format on
}  // namespace self_test

template <template <typename...> typename Template, typename CombinationsList>
struct ParametrizeManyCombinations;

template <template <typename...> typename Template, typename... Combinations>
struct ParametrizeManyCombinations<Template, std::tuple<Combinations...>>
{
    using Result = std::tuple<typename ParametrizeOneCombination<Template, Combinations>::Result...>;
};

namespace self_test
{
// clang-format off
static_assert(std::is_same_v<
    typename ParametrizeManyCombinations<
        MyTemplate2,
        std::tuple<
            std::tuple<A, B>,
            std::tuple<B, C>,
            std::tuple<C, A>
        >
    >::Result,
    std::tuple<
        MyTemplate2<A, B>,
        MyTemplate2<B, C>,
        MyTemplate2<C, A>
    >
>);
static_assert(std::is_same_v<
    typename ParametrizeManyCombinations<
        MyTemplate3,
        std::tuple<
            std::tuple<A, B, C>,
            std::tuple<B, C, A>,
            std::tuple<C, A, B>
        >
    >::Result,
    std::tuple<
        MyTemplate3<A, B, C>,
        MyTemplate3<B, C, A>,
        MyTemplate3<C, A, B>
    >
>);
// clang-format on
}  // namespace self_test

template <template <typename...> typename Template, typename... ArgsVariants>
struct ParametrizeTemplateWithCombinationsImpl
{
    using Combinations = MakeCombinations<ArgsVariants...>;
    using Result = typename ParametrizeManyCombinations<Template, Combinations>::Result;
};

template <template <typename...> typename Template, typename... ArgsVariants>
using ParametrizeWithCombinations = typename ParametrizeTemplateWithCombinationsImpl<Template, ArgsVariants...>::Result;

namespace self_test
{
// clang-format off
static_assert(
    std::is_same_v<
        ParametrizeWithCombinations<
            MyTemplate3,
            std::tuple<A, B, C>,
            std::tuple<A, B, C>,
            std::tuple<A, B, C>
        >,
        std::tuple<
            MyTemplate3<A, A, A>,
            MyTemplate3<A, A, B>,
            MyTemplate3<A, A, C>,
            MyTemplate3<A, B, A>,
            MyTemplate3<A, B, B>,
            MyTemplate3<A, B, C>,
            MyTemplate3<A, C, A>,
            MyTemplate3<A, C, B>,
            MyTemplate3<A, C, C>,

            MyTemplate3<B, A, A>,
            MyTemplate3<B, A, B>,
            MyTemplate3<B, A, C>,
            MyTemplate3<B, B, A>,
            MyTemplate3<B, B, B>,
            MyTemplate3<B, B, C>,
            MyTemplate3<B, C, A>,
            MyTemplate3<B, C, B>,
            MyTemplate3<B, C, C>,

            MyTemplate3<C, A, A>,
            MyTemplate3<C, A, B>,
            MyTemplate3<C, A, C>,
            MyTemplate3<C, B, A>,
            MyTemplate3<C, B, B>,
            MyTemplate3<C, B, C>,
            MyTemplate3<C, C, A>,
            MyTemplate3<C, C, B>,
            MyTemplate3<C, C, C>
        >>);
// clang-format on
}  // namespace self_test

template <typename TypesTuple>
struct TupleToGoogleTestTypesImpl;

template <typename... Types>
struct TupleToGoogleTestTypesImpl<std::tuple<Types...>>
{
    using Result = testing::Types<Types...>;
};

template <typename TypesTuple>
using TupleToGoogleTestTypes = typename TupleToGoogleTestTypesImpl<TypesTuple>::Result;
template <template <typename...> typename Template, typename... ArgsVariants>
using GParametrizeWithCombinations = TupleToGoogleTestTypes<ParametrizeWithCombinations<Template, ArgsVariants...>>;

template <auto value>
struct TypedConstant
{
    static constexpr auto kValue = value;
};

}  // namespace test_helpers
