#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "ass/enum_map.hpp"
#include "gtest/gtest.h"

namespace
{

enum class Key
{
    A,
    B,
    C,
    End,
};

using Converter = ass::EnumIndexConverter_Continuous<Key, Key::A, Key::End>;
template <typename Value>
using Map = ass::EnumMap<Key, Value, Converter>;

struct Lifetimes
{
    int live = 0;
    int constructed = 0;
    int destroyed = 0;
    int copies_before_throw = -1;
    int moves_before_throw = -1;
};

struct Value
{
    explicit Value(Lifetimes& counts, int number) : counts(&counts), number(number)
    {
        ++counts.live;
        ++counts.constructed;
    }

    Value(const Value& other) : counts(other.counts), number(other.number)
    {
        if (counts->copies_before_throw == 0) throw std::runtime_error("copy failed");
        if (counts->copies_before_throw > 0) --counts->copies_before_throw;
        ++counts->live;
        ++counts->constructed;
    }

    Value(Value&& other) : counts(other.counts), number(other.number)
    {
        if (counts->moves_before_throw == 0) throw std::runtime_error("move failed");
        if (counts->moves_before_throw > 0) --counts->moves_before_throw;
        other.number = -1;
        ++counts->live;
        ++counts->constructed;
    }

    Value& operator=(const Value&) = delete;
    Value& operator=(Value&&) = delete;

    ~Value()
    {
        --counts->live;
        ++counts->destroyed;
    }

    Lifetimes* counts;
    int number;
};

static_assert(!std::is_default_constructible_v<Value>);
static_assert(std::is_default_constructible_v<Map<Value>>);
static_assert(!std::is_copy_constructible_v<Map<std::unique_ptr<int>>>);
static_assert(!std::is_copy_assignable_v<Map<std::unique_ptr<int>>>);
static_assert(std::is_nothrow_move_constructible_v<Map<std::unique_ptr<int>>>);
static_assert(std::is_nothrow_move_assignable_v<Map<std::unique_ptr<int>>>);

TEST(EnumMapLifetime, ConstructsOnlyPresentNonDefaultValues)
{
    Lifetimes counts;
    {
        Map<Value> map;
        EXPECT_EQ(counts.live, 0);
        map.Emplace(Key::A, counts, 42);
        EXPECT_EQ(counts.live, 1);
        EXPECT_EQ(map.Get(Key::A).number, 42);
        map.Emplace(Key::C, counts, 84);
        EXPECT_EQ(counts.live, 2);

        map.Emplace(Key::A, counts, 21);
        EXPECT_EQ(counts.live, 2);
        EXPECT_EQ(counts.destroyed, 1);
        EXPECT_EQ(map.Get(Key::A).number, 21);
        {
            auto removed = map.Remove(Key::A);
            ASSERT_TRUE(removed.has_value());
            EXPECT_EQ(removed->number, 21);
            EXPECT_FALSE(map.Contains(Key::A));
            EXPECT_EQ(map.Size(), 1);
            EXPECT_EQ(counts.live, 2);
        }
        EXPECT_EQ(counts.live, 1);
        EXPECT_FALSE(map.Remove(Key::A).has_value());
        map.Emplace(Key::A, counts, 7);
        EXPECT_EQ(counts.live, 2);
        EXPECT_EQ(map.Get(Key::A).number, 7);
    }
    EXPECT_EQ(counts.live, 0);
    EXPECT_EQ(counts.constructed, counts.destroyed);
}

TEST(EnumMapLifetime, DoesNotDefaultConstructUnusedSlots)
{
    struct NoDefaultCall
    {
        NoDefaultCall()
        {
            throw std::runtime_error("default construction failed");
        }
        explicit NoDefaultCall(int number) : number(number) {}
        int number;
    };
    Map<NoDefaultCall> map;
    map.Emplace(Key::A, 42);
    EXPECT_EQ(map.GetOrAdd(Key::A).number, 42);
    EXPECT_THROW(map.GetOrAdd(Key::B), std::runtime_error);
    EXPECT_FALSE(map.Contains(Key::B));
    EXPECT_EQ(map.Size(), 1);
}

TEST(EnumMapLifetime, SupportsImmovableValuesAndFailedReplacement)
{
    struct Immovable
    {
        explicit Immovable(int& live_count, bool fail = false) : live(&live_count)
        {
            if (fail) throw std::runtime_error("construction failed");
            ++live_count;
        }
        Immovable(const Immovable&) = delete;
        Immovable(Immovable&&) = delete;
        Immovable& operator=(const Immovable&) = delete;
        Immovable& operator=(Immovable&&) = delete;
        ~Immovable()
        {
            --*live;
        }
        int* live;
    };
    static_assert(!std::is_copy_constructible_v<Map<Immovable>>);
    static_assert(!std::is_move_constructible_v<Map<Immovable>>);
    int live = 0;
    {
        Map<Immovable> map;
        map.Emplace(Key::A, live);
        map.Emplace(Key::B, live);
        EXPECT_EQ(live, 2);
        EXPECT_THROW(map.Emplace(Key::A, live, true), std::runtime_error);
        EXPECT_FALSE(map.Contains(Key::A));
        EXPECT_TRUE(map.Contains(Key::B));
        EXPECT_EQ(map.Size(), 1);
        EXPECT_EQ(live, 1);
        map.Emplace(Key::A, live);
        EXPECT_EQ(live, 2);
    }
    EXPECT_EQ(live, 0);
}

TEST(EnumMapLifetime, CopiesAndMovesOnlyOccupiedSlots)
{
    Lifetimes counts;
    {
        Map<Value> source;
        source.Emplace(Key::A, counts, 42);
        source.Emplace(Key::C, counts, 84);
        Map<Value> copy(source);
        EXPECT_EQ(counts.live, 4);
        EXPECT_EQ(copy.Size(), 2);
        EXPECT_FALSE(copy.Contains(Key::B));
        EXPECT_EQ(copy.Get(Key::C).number, 84);
        copy.Get(Key::A).number = 21;
        EXPECT_EQ(source.Get(Key::A).number, 42);

        Map<Value> moved(std::move(copy));
        EXPECT_EQ(copy.Size(), 0);
        EXPECT_EQ(copy.begin(), copy.end());
        EXPECT_EQ(counts.live, 4);
        EXPECT_EQ(moved.Get(Key::A).number, 21);
        copy.Emplace(Key::B, counts, 7);
        EXPECT_EQ(counts.live, 5);
        copy = source;
        EXPECT_EQ(counts.live, 6);
        EXPECT_FALSE(copy.Contains(Key::B));
        moved = std::move(copy);
        EXPECT_EQ(counts.live, 4);
        EXPECT_EQ(copy.Size(), 0);
        EXPECT_EQ(moved.Get(Key::A).number, 42);
        auto& alias = moved;
        moved = alias;
        moved = std::move(alias);
        EXPECT_EQ(moved.Size(), 2);
        EXPECT_EQ(counts.live, 4);
    }
    EXPECT_EQ(counts.live, 0);
    EXPECT_EQ(counts.constructed, counts.destroyed);
}

TEST(EnumMapLifetime, CleansPartialCopiesAndPreservesLiveMembership)
{
    Lifetimes counts;
    {
        Map<Value> source;
        source.Emplace(Key::A, counts, 42);
        source.Emplace(Key::B, counts, 84);
        counts.copies_before_throw = 1;
        EXPECT_THROW((void)Map<Value>(source), std::runtime_error);
        EXPECT_EQ(counts.live, 2);
        EXPECT_EQ(source.Size(), 2);

        Map<Value> destination;
        destination.Emplace(Key::C, counts, 7);
        counts.copies_before_throw = 1;
        EXPECT_THROW(destination = source, std::runtime_error);
        EXPECT_EQ(counts.live, 3);
        EXPECT_EQ(destination.Size(), 1);
        EXPECT_TRUE(destination.Contains(Key::A));
        EXPECT_FALSE(destination.Contains(Key::B));
        EXPECT_FALSE(destination.Contains(Key::C));
        EXPECT_EQ(destination.Get(Key::A).number, 42);
    }
    EXPECT_EQ(counts.live, 0);
    EXPECT_EQ(counts.constructed, counts.destroyed);
}

TEST(EnumMapLifetime, CleansPartialMovesAndPreservesLiveMembership)
{
    Lifetimes counts;
    {
        Map<Value> source;
        source.Emplace(Key::A, counts, 42);
        source.Emplace(Key::B, counts, 84);
        counts.moves_before_throw = 1;
        EXPECT_THROW((void)Map<Value>(std::move(source)), std::runtime_error);
        EXPECT_EQ(counts.live, 2);
        EXPECT_EQ(source.Size(), 2);
        EXPECT_EQ(source.Get(Key::A).number, -1);
        EXPECT_EQ(source.Get(Key::B).number, 84);

        Map<Value> destination;
        destination.Emplace(Key::C, counts, 7);
        counts.moves_before_throw = 1;
        EXPECT_THROW(destination = std::move(source), std::runtime_error);
        EXPECT_EQ(counts.live, 3);
        EXPECT_EQ(source.Size(), 2);
        EXPECT_EQ(destination.Size(), 1);
        EXPECT_TRUE(destination.Contains(Key::A));
        EXPECT_FALSE(destination.Contains(Key::B));
        EXPECT_FALSE(destination.Contains(Key::C));
    }
    EXPECT_EQ(counts.live, 0);
    EXPECT_EQ(counts.constructed, counts.destroyed);
}

TEST(EnumMapLifetime, FailedRemovalKeepsTheSlotAlive)
{
    struct MoveOnly : Value
    {
        using Value::Value;
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly(MoveOnly&&) = default;
    };
    Lifetimes counts;
    {
        Map<MoveOnly> map;
        map.Emplace(Key::B, counts, 42);
        counts.moves_before_throw = 0;
        EXPECT_THROW((void)map.Remove(Key::B), std::runtime_error);
        EXPECT_TRUE(map.Contains(Key::B));
        EXPECT_EQ(map.Get(Key::B).number, 42);
        EXPECT_EQ(counts.live, 1);
        counts.moves_before_throw = -1;
        auto removed = map.Remove(Key::B);
        ASSERT_TRUE(removed.has_value());
        EXPECT_EQ(removed->number, 42);
        EXPECT_EQ(map.Size(), 0);
        EXPECT_EQ(counts.live, 1);
    }
    EXPECT_EQ(counts.live, 0);
    EXPECT_EQ(counts.constructed, counts.destroyed);
}

TEST(EnumMapLifetime, GetOrAddPreservesOrReplacesValues)
{
    Map<int> map;
    EXPECT_EQ(map.GetOrAdd(Key::A), 0);
    map.Get(Key::A) = 42;
    EXPECT_EQ(map.GetOrAdd(Key::A), 42);
    EXPECT_EQ(map.GetOrAdd(Key::A, 84), 84);
    EXPECT_EQ(map.GetOrAdd(Key::B), 0);

    Map<std::unique_ptr<int>> move_only;
    EXPECT_EQ(*move_only.GetOrAdd(Key::A, std::make_unique<int>(42)), 42);
    EXPECT_EQ(*move_only.GetOrAdd(Key::A, std::make_unique<int>(84)), 84);
    EXPECT_EQ(move_only.Size(), 1);

    Lifetimes counts;
    {
        Map<Value> non_default;
        non_default.GetOrAdd(Key::C, Value(counts, 21));
        EXPECT_EQ(non_default.Get(Key::C).number, 21);
        non_default.GetOrAdd(Key::C, Value(counts, 7));
        EXPECT_EQ(non_default.Get(Key::C).number, 7);
        EXPECT_EQ(counts.live, 1);
        Value supplied(counts, 42);
        EXPECT_EQ(non_default.GetOrAdd(Key::A, supplied).number, 42);
        EXPECT_EQ(supplied.number, 42);
        EXPECT_EQ(counts.live, 3);
    }
    EXPECT_EQ(counts.live, 0);
}

constexpr bool ConstantEvaluation()
{
    struct NonDefault
    {
        explicit constexpr NonDefault(int number) : number(number) {}
        int number;
    };
    Map<NonDefault> map;
    map.Emplace(Key::A, 42);
    map.Emplace(Key::C, 84);
    Map<NonDefault> copy(map);
    Map<NonDefault> moved(std::move(copy));
    if (copy.Size() != 0) return false;
    copy = map;
    map = std::move(moved);
    auto removed = map.Remove(Key::C);
    if (!removed || removed->number != 84 || map.Contains(Key::C)) return false;
    map.Emplace(Key::C, 7);
    int sum = 0;
    for (auto [key, value] : std::as_const(map)) sum += value.number;
    return sum == 49 && copy.Get(Key::C).number == 84;
}

static_assert(ConstantEvaluation());

TEST(EnumMapLifetime, SupportsCopyOnlyOverAlignedValues)
{
    struct alignas(128) CopyOnly
    {
        explicit CopyOnly(int number) : number(number) {}
        CopyOnly(const CopyOnly&) = default;
        CopyOnly(CopyOnly&&) = delete;
        CopyOnly& operator=(const CopyOnly&) = delete;
        int number;
    };
    static_assert(alignof(Map<CopyOnly>) >= alignof(CopyOnly));
    Map<CopyOnly> map;
    map.Emplace(Key::A, 42);
    map.Emplace(Key::C, 84);
    Map<CopyOnly> copy(map);
    copy = map;
    auto removed = map.Remove(Key::C);
    ASSERT_TRUE(removed.has_value());
    EXPECT_EQ(removed->number, 84);
    EXPECT_FALSE(map.Contains(Key::C));
    EXPECT_EQ(copy.Get(Key::C).number, 84);
    EXPECT_EQ(map.Get(Key::A).number, 42);
}

}  // namespace
