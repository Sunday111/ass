# EnumMap

Fixed map with enumeration keys. Can be used in constexpr context (if your value type is constexpr-capable).

## Template parameters
- `Key` - enumeration type used as a key. You have to implement [`EnumIndexConverter`](./enum_index_converter.md).
- `Value` - value which will be associated with a key.
- `Converter` - enumeration <-> index converter. By default uses `ass::EnumIndexConverter<T>`.

## Methods
- `Value& Get(const Key key)` - gets values associated with key. Consider calling `Contains` beforehand.
- `const Value& Get(const Key key) const` - const overload.
- `std::optional<Value> Remove(const Key key)` - removes key and associated value. Returns removed value if it was present in the map.
- `bool Contains(const Key key) const` - returns true if the key is present.
- `size_t Size() const` - number of present keys.
- `static size_t Capacity() const` - maximum number of keys.
