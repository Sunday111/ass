# FixedUnorderedMap

Like `std::unordered_map` but on the stack.

## Template parameters
- `Capacity` - maximum number of keys. Not that this values should usually be bigger than the number of key value pairs you actually gonna use to avoid collisions (except your hasher can solve that perfectly).
- `Key` - key type.
- `Value` - value type
- `Hasher` - defaults to `std::hash`. If you need to use this map in constexpr context you have to pass hasher with constexpr `operator(const KeyType)`.

## Methods
- `bool Contains(const Key key) const` - returns true if key is present.
- `size_t Size() const` - number of keys stored in the map.
- `static size_t Capacity()` - maximum number of keys.
- `Value& Get(const Key key)` - gets value by key. Consider calling `Contains` beforehand.
- `const Value& Get(const Key key) const` - constant overload of `Get`.
- `Value* TryEmplace(const Key key, Args&&... args)` - `TryAdd` version with perfect forwarding.
- `Value& Value(const Key key, Args&&... args)` - `Add` version with perfect forwarding.
- `Value* TryAdd(const Key key, std::optional<Value> value = std::nullopt)` - associates value with specified key. Returns pointer to stored value. If map is already full return nullptr.
- `Value& Add(const Key key, std::optional<Value> value = std::nullopt)` - same as `TryAdd` but returns reference (i.e. unsafe version).
- `std::optional<Value> Remove(const Key key)` - removes key and value from the map and returns value if it was present in the map.
