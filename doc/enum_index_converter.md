# `EnumIndexConverter<Enum>`

`EnumIndexConverter` is a class template that provides a way to convert enumeration values to continuous zero-based indices. This can be useful, for example, when working with arrays or other data structures that require integer indices to access their elements. The library's collections that work with enumerations require this class to be specialized for your types.

To use this class, you must instantiate the template with your enumeration type and implement the functions specified in the template declaration. Most of the time, however, you do not have to write them manually because there are already default implementations that you can use - `EnumIndexConverter_Continuous` and `EnumIndexConverter_Sparse`.

## `EnumIndexConverter_Continuous`
This is an implementation for `EnumIndexConverter` and is used for enumerations with continuous underlying values. For example:

```C++
enum class MyEnum {
    A, B, C, D, E, F, G, kMax
};
```

Template parameters:
- `T` - type of your enumeration
- `T begin` - the first value in the enumeration.
- `T end` - the enumeration value that is one more than the last enumeration value. For example, `kMax` in the example above.

For the `MyEnum` enumeration, you can use `EnumIndexConverter_Continuous` in the following way:

```C++
namespace ass {
    template<>
    struct EnumIndexConverter<MyEnum> :
        public EnumIndexConverterContinuous<MyEnum, MyEnum::A, MyEnum::kMax>
    {};
}
```

## `EnumIndexConverter_Sparse`

This is an implementation for EnumIndexConverter and is used for cases when the enumeration entries have non-continuous values:

```C++
enum class MyEnum {
    A = 42,
    B = 48,
    C = 55
};
```

Template paramaters:
- `T` - enumeration type
- `auto values_fn` - constexpr function that returns a `std::array` with all values of the enumeration.

For the `MyEnum` enumeration, you can use `EnumIndexConverter_Sparse` in the following way:

```C++

inline constexpr auto GetMyEnumValues() {
    return std::array<MyEnum, 3> {
        MyEnum::A, MyEnum::B, MyEnum::C
    };
}

namespace ass {
    template<>
    struct EnumIndexConverter<MyEnum> :
        public EnumIndexConverter_Sparse<MyEnum, GetMyEnumValues>
    {};
}
```
