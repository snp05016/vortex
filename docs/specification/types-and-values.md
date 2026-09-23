# 4. Types and values

Every Vortex expression has a type known before executable code is produced.
Types determine valid operations, storage layout, calling behavior, and the
checks required by later compiler stages.

## 4.1 Type grammar

```ebnf
type ::=
      primitive_type
    | array_type
    | reference_type
    | identifier ;

primitive_type ::=
      "void"
    | "bool"
    | "char"
    | "i32"
    | "u32"
    | "usize"
    | "f32"
    | "f64"
    | "String" ;

array_type ::=
    "[", type, ";",
    expression, { ",", expression },
    "]" ;

reference_type ::=
    "&", [ "mut" ], type ;
```

Named types are preserved as identifiers until name resolution. The parser
must not assume that an identifier names a struct merely because structs are
the only user-defined type form in v0.1.

## 4.2 Primitive types

| Type | Meaning | Default literal source |
| --- | --- | --- |
| `void` | No returned value | None |
| `bool` | `true` or `false` | Boolean literal |
| `char` | One Unicode character | Character literal |
| `i32` | Signed 32-bit integer | Unconstrained integer literal |
| `u32` | Unsigned 32-bit integer | Context only |
| `usize` | Target-sized unsigned size or index | Context only |
| `f32` | 32-bit floating-point number | Unconstrained floating literal |
| `f64` | 64-bit floating-point number | Context only |
| `String` | UTF-8 text value | String literal |

`void` is valid as a function return type. It is not a storable value type and
cannot be used for a local variable, parameter value, array element, or struct
field.

## 4.3 Integers

`i32` represents signed values from `-2^31` through `2^31 - 1`. `u32`
represents values from `0` through `2^32 - 1`. `usize` represents nonnegative
sizes and indices supported by the target architecture.

An unconstrained integer literal defaults to `i32`. Context may require
another integer type when the value is representable. Unary `-` is separate
from the literal, so the positive literal is checked together with its unary
context.

Integer arithmetic, bitwise operations, shifts, remainder, and comparisons are
available only where the operand types support them. Implicit conversion rules
beyond contextual literal fitting are not yet generalized in v0.1.

## 4.4 Floating-point values

An unconstrained floating literal defaults to `f32`. `f64` is selected through
an explicit type context or cast.

Floating-point values support arithmetic and compatible comparisons. Remainder,
bitwise, and shift operations are not defined for floating-point values in
v0.1.

Floating-point behavior follows the target's documented IEEE 754 support. The
compiler must not silently enable transformations that change specified
floating-point results unless a future language option permits them.

## 4.5 Boolean values

`bool` has exactly two values, `true` and `false`. Vortex does not implicitly
interpret integers, pointers, strings, or collections as booleans. Conditions
for `if` and `while` must have type `bool`.

Logical `&&` and `||` short-circuit from left to right. `!` computes logical
negation.

## 4.6 Characters and strings

A `char` stores one Unicode character. Its storage representation is an
implementation detail, but it is not defined as a one-byte value.

A `String` stores UTF-8 text. Vortex v0.1 specifies string literals and basic use with
supported functions such as `print`. String mutation, indexing,
interpolation, concatenation, searching, and numeric parsing are not yet
specified.

## 4.7 Arrays

An array type has one element type and one or more compile-time dimensions.
Dimensions are part of the type.

```vortex
[f32; 4]
[f32; 4, 4]
[[i32; 2]; 3]
```

See [Arrays and shapes](arrays.md) for construction, indexing, and dimension
rules.

## 4.8 Struct values

A struct declaration introduces a named value type with ordered named fields.
Struct values are constructed by naming each field. Vortex v0.1 defines no
inheritance or object identity model.

See [Structs](structs.md) for full rules.

## 4.9 References

`&T` is a shared reference to an existing `T`. `&mut T` is a mutable reference
that permits mutation when its source storage is mutable and no conflicting
access exists.

References are not raw integer addresses and do not permit pointer arithmetic.
See [References and mutability](references.md).

## 4.10 Type inference

Local variable type inference uses the initializer:

```vortex
let count = 10;       // i32
let weight = 0.5;     // f32
let ready = false;    // bool
let name = "Vortex"; // String
```

Inference does not make the language dynamically typed. Once inferred, the
variable has one fixed type. Function parameters, struct fields, and explicit
function return types are written in source.

An empty array expression is excluded from v0.1 because local inference has no
element value from which to determine its element type.

## 4.11 Type equality

Primitive types are equal when their primitive kinds match. Named types are
equal when name resolution identifies the same declaration. Reference types
include mutability and referenced type. Array types include element type, rank,
and every constant-evaluated dimension.

For example, `[f32; 2 + 2]` and `[f32; 4]` have equal shapes after constant
evaluation, while `[f32; 4]` and `[f32; 2, 2]` do not.

## 4.12 Casts and conversions

Explicit casts use call syntax with a type name in callee position:

```vortex
let value: f32 = f32(count);
```

The parser records ordinary call syntax. Name and type resolution determine
whether the callee is a function or type conversion.

A complete numeric conversion matrix is not yet fixed. Implementations must
not invent silent lossy conversions and present them as standard v0.1
behavior.

## 4.13 Types outside v0.1

Tuples, unions, enums, vectors, slices, runtime-sized arrays, function types,
raw pointers, generic types, aliases, `i8`, `i16`, `i64`, `u8`, `u16`, `u64`,
`f16`, and `bf16` are not v0.1 types.
