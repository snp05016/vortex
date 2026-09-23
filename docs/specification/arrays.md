# 7. Arrays and shapes

An array is a fixed-size, homogeneous value. Its element type, rank, and
dimensions are part of its type and are known before storage layout is
generated.

## 7.1 Array types

```ebnf
array_type ::=
    "[", type, ";",
    expression, { ",", expression },
    "]" ;
```

Examples:

```vortex
[f32; 4]
[f32; 4, 4]
[[i32; 2]; 3]
&mut [f32; rows * columns]
```

The last type is grammatically valid. It is semantically valid only when
`rows * columns` can be evaluated as a compile-time integer expression.

## 7.2 Dimension rules

Every dimension is parsed as a complete expression and preserved in the AST.
Before type equality, layout, or code generation, constant evaluation must
prove that each dimension:

- has an integer type;
- is known at compile time;
- is representable as a supported `usize` extent;
- does not cause the complete layout size to overflow.

The grammar deliberately does not restrict dimensions to literal tokens:

```vortex
let row: [f32; 2 + 2] = [0.0; 2 + 2];
let tile: [f32; 2 * 2, 8 / 2] = [0.0; 2 * 2, 8 / 2];
```

Runtime parameters do not make an array runtime-sized:

```vortex
fn make(size: usize) {
    let values: [f32; size] = [0.0; size];
    // semantic error: size is not a compile-time value
}
```

Whether a zero extent is legal remains an open semantic decision. A conforming
implementation should diagnose it as unsupported until the specification
chooses a rule.

## 7.3 Element-list construction

```ebnf
array_element_list ::=
    expression, { ",", expression }, [ "," ] ;
```

An element-list array contains one or more expressions in source order:

```vortex
[1.0, 2.0, 3.0]
[first(), second(), third()]
```

All elements must have a compatible common type. Nested arrays must have
compatible element types and shapes. The trailing comma is optional.

An empty array expression `[]` is not accepted in v0.1 because it supplies no
element from which local type inference can determine an element type.

## 7.4 Repeat-array construction

```ebnf
repeat_array_body ::=
    expression, ";",
    expression, { ",", expression } ;
```

The expression before `;` supplies the repeated value. The expressions after
`;` supply one or more dimensions:

```vortex
[0.0; 16]
[0.0; 4, 4]
[false; 2 + 2]
```

The repeated value expression is evaluated once, and its resulting value fills
the array. Each dimension follows the same constant-evaluation rules as an
array type.

A trailing comma after repeat dimensions is not accepted.

## 7.5 Rank and shape

The **rank** of an array is its number of dimensions. Its **shape** is the
ordered list of evaluated extents.

| Type | Rank | Shape |
| --- | ---: | --- |
| `[f32; 4]` | 1 | `[4]` |
| `[f32; 2, 3]` | 2 | `[2, 3]` |
| `[f32; 2 + 2]` | 1 | `[4]` after constant evaluation |

Array type equality requires equal element types, ranks, and evaluated shapes.

## 7.6 Indexing

```ebnf
index_suffix ::=
    "[", expression, { ",", expression }, "]" ;
```

Vortex array indexing is zero-based. Each index must have a supported integer
type and must be within its dimension's bounds.

```vortex
let first = values[0];
let cell = matrix[row, column];
```

The number of indices must match the number of dimensions selected by the
operation. Partial indexing and slice results are not specified in v0.1.

When an out-of-bounds index is provable, the compiler rejects the program. When
the index is known only at runtime, generated code retains a bounds check.

## 7.7 Assignment

An entire array value may initialize compatible array storage. An element may
be assigned when the selected storage is mutable:

```vortex
let mut values = [1.0, 2.0, 3.0];
values[0] = 10.0;
```

Vortex v0.1 has fixed shapes. Assignment never resizes an array. Shape mismatch is a
type error rather than a request to truncate, pad, or reallocate.

## 7.8 Memory and layout

Arrays are intended to use contiguous, predictable storage suitable for
numerical work. Exact ABI layout, alignment, and ordering rules are not yet
part of the public v0.1 specification. A backend must apply one documented
layout consistently and preserve observable indexing behavior.

## 7.9 Excluded array behavior

Vortex v0.1 has no runtime-sized array types, vectors, slices, array views, open-ended
index ranges, shape broadcasting, implicit reshaping, or built-in matrix
operators. These require separate future specifications.
