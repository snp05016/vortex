# 5. Expressions

An expression computes, constructs, or refers to a value. Expression nesting
forms the value-producing part of the Vortex abstract syntax tree.

## 5.1 Expression forms

Vortex v0.1 includes:

- scalar literals and identifiers;
- array and struct construction;
- grouping;
- prefix unary operations;
- infix binary operations;
- bounded ranges;
- calls and cast-shaped calls;
- indexing and field access.

Assignment and control flow are not expressions.

## 5.2 Precedence and associativity

From lowest to highest precedence:

| Level | Form | Operators | Associativity |
| ---: | --- | --- | --- |
| 1 | Range | `..`, `..=` | Non-chaining |
| 2 | Logical OR | `||` | Left |
| 3 | Logical AND | `&&` | Left |
| 4 | Bitwise OR | `\|` | Left |
| 5 | Bitwise XOR | `^` | Left |
| 6 | Bitwise AND | `&` | Left |
| 7 | Equality | `==`, `!=` | Left by grammar |
| 8 | Comparison | `<`, `<=`, `>`, `>=` | Left by grammar |
| 9 | Shift | `<<`, `>>` | Left |
| 10 | Additive | `+`, `-` | Left |
| 11 | Multiplicative | `*`, `/`, `%` | Left |
| 12 | Unary | `+`, `-`, `!`, `~`, `&`, `&mut` | Right |
| 13 | Postfix | call, index, field | Left and chainable |
| 14 | Primary | literal, name, array, struct, grouping | Not applicable |

Thus `2 + 3 * 4` groups as `2 + (3 * 4)`, and `- -value` groups from the
right.

## 5.3 Primary expressions

### Literals and names

A scalar literal produces its decoded value. An identifier expression reads
the value of the declaration selected by name resolution.

### Grouping

Parentheses contain one expression and override ordinary precedence:

```vortex
(left + right) * scale
```

Empty parentheses do not form a value in v0.1.

### Array construction

An element-list array contains one or more compatible expressions:

```vortex
[1, 2, 3]
[first(), second(), third()]
```

A repeat array contains one value followed by one or more dimensions:

```vortex
[0.0; 4]
[0.0; rows, columns]
```

See [Arrays and shapes](arrays.md).

### Struct construction

A struct expression names a struct and supplies ordered field initializers:

```vortex
Point { x: 0.0, y: 0.0 }
```

See [Structs](structs.md).

## 5.4 Unary expressions

| Operator | Required category |
| --- | --- |
| `+` | Numeric operand |
| `-` | Signed integer or floating-point operand |
| `!` | `bool` operand |
| `~` | Integer operand |
| `&` | Addressable operand |
| `&mut` | Mutable addressable operand |

Unary expressions group right to left. The parser preserves the exact
operator. Type and reference checking validate the operand.

## 5.5 Arithmetic and bitwise expressions

The arithmetic operators are `+`, `-`, `*`, `/`, and `%`. Remainder requires
integer operands. Arithmetic operand types must be compatible under the
specified type rules.

The integer bitwise operators are `&`, `|`, `^`, `~`, `<<`, and `>>`. Prefix
`&` is a reference operator, while infix `&` is bitwise AND; their grammatical
positions distinguish them.

Integer overflow, invalid shift counts, division by zero, and remainder by
zero are checked according to the [diagnostic rules](diagnostics.md).

## 5.6 Comparison and equality

`==` and `!=` compare compatible values for equality where their type defines
equality. `<`, `<=`, `>`, and `>=` require compatible ordered operands.

All comparison and equality expressions produce `bool`. Vortex v0.1 does not define
ordered comparison for strings or structs.

## 5.7 Logical expressions

`!`, `&&`, and `||` require boolean operands. `&&` and `||` evaluate the left
operand first and evaluate the right operand only when required to determine
the result.

```vortex
let safe = index < 4 && values[index] > 0.0;
```

The required short-circuit behavior is observable and must be preserved by
lowering and optimization.

## 5.8 Ranges

A range has two endpoints and one of two operators:

```vortex
start..end   // excludes end
start..=end  // includes end
```

Open-ended and chained ranges are invalid in v0.1. When used by a `for` loop,
endpoints must have compatible integer types.

## 5.9 Postfix expressions

Postfix suffixes can be chained in source order:

```vortex
factory().items[row, column].value
```

### Calls

A call contains a callee expression followed by zero or more ordered argument
expressions. A trailing argument comma is not accepted.

Name and type checking determine whether the callee is callable, whether it is
a type conversion, and whether argument count and types are valid.

### Indexing

An index suffix contains one or more comma-separated index expressions. A
trailing comma is not accepted. The AST preserves every index in source order.

### Field access

Field access uses `.` followed by an identifier. Type checking verifies that
the selected field exists on the resolved object type.

## 5.10 Evaluation order

Vortex v0.1 requires left-to-right evaluation for ordered child lists such as call
arguments, array elements, and struct field initializer values. Short-circuit
operators apply their more specific rules.

Optimization may remove an evaluation only when doing so preserves all
specified observable behavior and required diagnostics.

## 5.11 Invalid forms

Vortex v0.1 has no assignment expressions, ternary expressions, value-producing
`if`, `match`, lambdas, optional chaining, null coalescing, increment,
decrement, open-ended ranges, or chained ranges.

The [formal grammar](grammar.md) gives the complete productions.
