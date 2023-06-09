# `EnumSet<Enum, Converter>`

Represents a set of enumeration values. Internally stored as bitset. You have to specialize [`ass::EnumIndexConverter`](enum_index_converter.md) in order to use it or pass converter type as the second template parameter.

The EnumSet class contains the following public member functions:
- `Add(const T value) -> bool`: Adds the enumeration value to the set. Returns true if values was added to the set. Returns false if specified value was already present.
- `Remove(const T value) -> bool`: Removes the enumeration value from the set. Returns false if set did not contain that value.
- `Contains(const T value) const`: Returns true if the set contains the enumeration value value, otherwise false.
- `GetComplement() const`: Returns the complement of the set.
- `Invert()`: Replaces the set with its complement.
- `Size() const`: Returns the size of the set.
- `Capacity() const`: Returns the maximum capacity of the set.
- `IsEmpty() const`: Returns true if the set is empty, otherwise false.

The EnumSet class also provides STL-style member functions:
- `begin() const`: Returns an iterator to the beginning of the set.
- `end() const`: Returns an iterator to the end of the set.

The `MakeEnumSet` function is a helper function that creates an EnumSet object and adds the given enumeration values to the set. 