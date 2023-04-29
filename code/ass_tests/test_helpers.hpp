#pragma once

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

    T value{};
};
