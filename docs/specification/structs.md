# 8. Structs

A struct is a named value type containing an ordered set of named fields. It is
Vortex v0.1's user-defined aggregate type.

## 8.1 Declaration

```vortex
struct Point {
    x: f32,
    y: f32,
}
```

Field declarations use `name: type`. Fields are comma-separated, and the final
comma is optional. Each field name must be unique within its struct.

A struct declaration introduces the struct name into program type scope. It
does not create a runtime value.

## 8.2 Empty structs

The grammar accepts an empty struct:

```vortex
struct Marker {}
```

Its storage representation is implementation-defined. It remains a distinct
named type.

## 8.3 Struct construction

```ebnf
struct_expression ::=
    identifier, "{",
    field_initializer,
    { ",", field_initializer },
    [ "," ],
    "}" ;

field_initializer ::=
    identifier, ":", expression ;
```

A struct expression names the target struct and provides one initializer for
every declared field:

```vortex
let origin = Point {
    x: 0.0,
    y: 0.0,
};
```

The parser preserves the struct name and every initializer's field name, value,
order, and location. Name and type checking must then require:

- the target name resolves to a struct;
- every required field appears exactly once;
- no unknown field appears;
- every initializer value is compatible with its declared field type.

Empty, positional, shorthand, anonymous, and update-style struct expressions
are not accepted:

```vortex
Point {}                  // invalid: required fields are missing
Point(0.0, 0.0)           // a call, not struct construction
Point { x, y }            // invalid shorthand
```

An empty struct currently has no constructible empty expression under the
struct-expression grammar. This mismatch is an open v0.1 design issue and must
be resolved before empty structs are relied upon in programs.

## 8.4 Field access

Use `.` to select a field from a struct value:

```vortex
let horizontal = point.x;
let area = size.width * size.height;
```

Field access can participate in postfix chains:

```vortex
points[index].position.x
```

Type checking resolves the object type and rejects a field that does not exist.

## 8.5 Field assignment

A field is assignable when the complete access path identifies mutable
storage:

```vortex
let mut point = Point { x: 0.0, y: 0.0 };
point.x = 4.0;
```

A field of an immutable binding cannot be modified. The field declaration does
not independently carry mutability in v0.1.

## 8.6 Value semantics

Structs are value types. A struct expression creates a value rather than an
object with independent identity. The exact copy, move, and parameter-passing
ABI is an implementation detail, subject to preserving Vortex value behavior.

References allow a function to access an existing struct without treating the
struct as a raw pointer.

## 8.7 Type identity

Struct types are nominal: two separately declared structs are distinct types
even when they have the same field names and field types.

```vortex
struct Position { x: f32, y: f32 }
struct Velocity { x: f32, y: f32 }
```

`Position` and `Velocity` are not interchangeable without an explicitly
specified conversion, and v0.1 defines no implicit structural conversion.

## 8.8 Layout

Field order is preserved by the AST. Exact physical field offsets, padding,
alignment, and ABI compatibility are not yet specified. The backend must use a
consistent documented layout and ensure field access observes the declared
field values.

## 8.9 Excluded struct features

Vortex v0.1 has no field defaults, private fields, methods, constructors,
destructors, inheritance, anonymous structs, generics, extension declarations,
or update syntax.
