# EnumMap

Fixed-capacity map with enumeration keys. Storage is inline; only present keys have constructed values. The key bitset also tracks which slots need destruction. Values need not be default-constructible, assignable, or movable to use `Emplace` and `Get`. Operations can be evaluated at compile time when the value type supports them.

## Template parameters

- `Key` — enumeration type used as a key.
- `Value` — type associated with each present key.
- `Converter` — enumeration/index converter, defaulting to `ass::EnumIndexConverter<Key>`. See [`EnumIndexConverter`](./enum_index_converter.md).

## Methods

- `Emplace(key, args...)` constructs a missing value directly from the arguments and returns a reference. For an existing key, it assigns a newly constructed value when assignment is supported; otherwise it destroys and reconstructs the slot. In the latter case, arguments must not refer to the value being replaced or its members.
- `Get(key)` returns a reference, with a const overload. The key must be present.
- `GetOrAdd(key)` returns an existing value or default-constructs a missing one. This overload requires a default-constructible value type.
- `GetOrAdd(key, optional_value)` inserts or replaces the value when the optional is engaged. An empty optional behaves like `GetOrAdd(key)` for default-constructible values; otherwise the key must already be present.
- `Remove(key)` returns the removed value in an optional, or an empty optional if absent. It destroys the stored value immediately after extracting it. The value type must be move-constructible or copy-constructible.
- `Contains(key)`, `Size()`, and static `Capacity()` report membership, occupied slots, and maximum slots.
- `begin()` and `end()` iterate over present keys in converter index order, exposing the key and a reference to its value. Const iteration is supported.

Copying constructs only occupied slots and requires copy-constructible values. The move overloads require move-constructible values and empty the source on success, except for self-assignment. Copy-only values use the copy overloads instead. Assignment destroys the destination's existing values first; the values themselves need not support assignment. Destruction visits only occupied slots.

## Exceptions and references

A failed insertion leaves its key absent. During replacement, failure to construct the temporary leaves an assignable value unchanged; a throwing assignment leaves its state as determined by the value type. For nonassignable values, failed reconstruction leaves the key absent.

Failed map construction destroys every completed destination value. Failed map assignment can leave a partially populated destination. A failed move can leave source values moved from, with their keys still present. Failed removal retains the slot if initial extraction throws, although the value may have been modified by its move constructor; an exception while returning the extracted optional can occur after the slot has been removed.

Removing or reconstructing a slot invalidates references and iterators to that value. Assigning or destroying the map invalidates all its value references. Other occupied slots are unaffected by insertion, replacement, or removal of a key.
