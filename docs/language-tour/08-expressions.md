# Expressions

## Learning goals

After this chapter, you should be able to identify every v0.1 expression form,
separate unary from binary operators, read precedence, and explain which stage
checks an expression.

An expression produces a value. A statement performs an action. Vortex programs
use both of them together.

An expression is any piece of code that produces a value. The value can be a
number, a boolean, a string, an array, or another Vortex type.

For example, every line below contains an expression:

```vortex
42
width
2 + 3
square(4.0)
values[index]
x > y
```

## Expression quick reference

| Form | Syntax example | Result |
| --- | --- | --- |
| Literal | `42`, `true`, `"text"` | The literal's value. |
| Name | `width` | The value bound to a visible name. |
| Unary | `-value`, `!ready`, `&mut item` | One operation applied to one operand. |
| Binary | `left + right`, `a && b` | One operation applied to two operands. |
| Call/cast | `square(4.0)`, `f32(count)` | A function result or converted value. |
| Index/field | `values[index]`, `point.x` | One part of a compound value. |
| Array/struct | `[1, 2]`, `Point { x: 1.0 }` | A newly constructed value. |
| Range | `0..10`, `0..=10` | A bounded sequence used by iteration. |
| Group | `(left + right)` | The enclosed expression with explicit precedence. |

Assignment (`=`) is a statement operator, not a binary expression. A complete
expression also does not end in `;`; the containing statement supplies it.

## Literal expressions

A literal is a value written directly in the program:

```vortex
42                  // i32
3.14                // f32
true                // bool
'V'                 // char
"hello from Vortex" // String
[1, 2, 3, 4]        // array
```

The scalar literal forms are integer, floating-point, boolean, character, and
string literals. An array is a constructed expression, even though programmers
often call it an array literal informally.

Malformed delimiters, unsupported escapes, empty character literals, and
unsupported literal spellings are invalid:

```vortex
"unterminated
'AB'
0xFF // hexadecimal integer literals are not in v0.1
```

The lexer (the [compiler stage](../compiler/guide/index.md) that splits source text into tokens) recognizes scalar literals.
The parser (the stage that checks how tokens fit together) wraps them in
expression nodes, and type checking determines or verifies their Vortex types.

## Name expressions

Writing the name of a variable reads its value:

```vortex
let width = 128;
let area = width * width;
```

Here, both uses of `width` are expressions.

Using an undeclared or out-of-scope name is invalid. The parser can still build
a name expression; name resolution is the stage that reports the missing
declaration.

## Unary expressions

A unary expression applies one prefix operator to one operand:

| Operator | Allowed operand | Example |
| --- | --- | --- |
| `+` | Numeric | `+value` |
| `-` | Signed integer or floating point | `-value` |
| `!` | Boolean | `!finished` |
| `~` | Integer | `~flags` |
| `&` | Addressable value | `&value` |
| `&mut` | Mutable addressable value | `&mut value` |

The operand is itself an expression, so nesting is allowed:

```vortex
let negative = -(left + right);
let not_ready = !is_ready;
```

Invalid examples:

```vortex
let bad = !42;       // ! requires bool
let bits = ~3.14;    // ~ requires an integer
let value = 10;
let bad_ref = &mut value; // value is not mutable
```

The parser records the operator and operand. Type checking validates the
operand. Reference analysis additionally checks addressability and mutability.

## Arithmetic expressions

Vortex supports the usual arithmetic operators:

| Operator | Meaning | Example |
| --- | --- | --- |
| `+` | addition | `a + b` |
| `-` | subtraction | `a - b` |
| `*` | multiplication | `a * b` |
| `/` | division | `a / b` |
| `%` | remainder | `a % b` |

The `+` and `-` operators can also be placed before one value:

```vortex
let positive = +value;
let negative = -value;
```

Arithmetic normally requires compatible number types. Vortex should not quietly
convert between unrelated types when that could lose information.

`%` requires integer operands. Division by zero and integer overflow follow the
[runtime and numerical rules](06-runtime-and-numerical-rules.md). The type
checker rejects mismatched or nonnumeric operands; it does not treat strings as
numbers.

## Comparison expressions

Comparisons produce a `bool`:

| Operator | Meaning |
| --- | --- |
| `==` | equal |
| `!=` | not equal |
| `<` | less than |
| `<=` | less than or equal |
| `>` | greater than |
| `>=` | greater than or equal |

```vortex
let same = left == right;
let in_bounds = index < length;
```

Both operands must be compatible. Ordered comparison is for ordered values such
as numbers; v0.1 does not define ordering for structs or strings. The type
checker rejects incompatible comparisons.

## Logical expressions

Logical operators work with boolean values:

| Operator | Meaning | Example |
| --- | --- | --- |
| `!` | not | `!finished` |
| `&&` | and | `ready && valid` |
| `\|\|` | or | `failed \|\| cancelled` |

```vortex
let ready = is_ready && has_capacity;
let can_continue = ready || failed;
```

`&&` and `||` use short-circuit evaluation. This means Vortex stops as soon as
it knows the answer:

```vortex
let valid = index < 4 && values[index] > 0.0;
```

If the index is outside the array, the second part is not evaluated.

Both operands must be `bool`. `1 && 2` is invalid because Vortex has no implicit
integer-to-boolean conversion. Code generation preserves short-circuit order.

## Bitwise expressions

Bitwise operators work with the individual bits of integer values. They are
useful for low-level CPU and GPU work:

| Operator | Meaning |
| --- | --- |
| `&` | bitwise and |
| `\|` | bitwise or |
| `^` | bitwise exclusive or |
| `~` | bitwise not |
| `<<` | shift bits left |
| `>>` | shift bits right |

```vortex
let masked = flags & 0b1111;
let next_bit = value << 1;
```

Bitwise operators are different from the logical operators `&&`, `||`, and `!`.

Their operands must be compatible integers. Floating-point, boolean, string,
and struct operands are invalid. The parser uses precedence to distinguish
prefix reference `&value` from infix bitwise `left & right`.

## Grouped expressions

Parentheses make the order of a calculation clear:

```vortex
let first = 2 + 3 * 4;    // 14
let second = (2 + 3) * 4; // 20
```

## Function-call expressions

A function call is an expression when the function returns a value:

```vortex
let result = square(4.0);
let larger = max(left, right);
```

A function returning `void` can still be called, but its call does not produce
a value that can be stored:

```vortex
print("starting");
```

The number and types of arguments must match the function parameters. These
are invalid:

- calling an undeclared name;
- passing the wrong number of arguments;
- storing a `void` result.

The parser builds the call; name resolution finds the
callee; type checking validates arguments and the result.

## Array expressions

An array literal creates an array:

```vortex
let values = [1.0, 2.0, 3.0, 4.0];
```

A repeat-array expression accepts one value expression followed by one or more
dimension expressions:

```vortex
let row = [0.0; 2 + 2];
let matrix = [0.0; 2 * 2, 8 / 2];
```

Each dimension is parsed as a full expression. It must evaluate during
compilation to an integer that fits `usize`; runtime-dependent and noninteger
dimensions are invalid:

```vortex
fn make(size: usize) {
    let values = [0.0; size];
    // invalid: size is not known during compilation
}

let bad = [0.0; 2.5]; // invalid: f32 is not an array dimension type
```

An explicit element list must contain at least one element, and all elements
must have a compatible type. Empty `[]` is not supported because it provides no
element type for local inference.

An indexing expression reads one element:

```vortex
let first = values[0];
let current = values[index];
let cell = matrix[row, column];
```

An index must have an integer type. Vortex checks that the index is inside the
array along every dimension. It does this during compilation when the answer is
already known and during execution otherwise. When the compiler can prove an
index is safe, it may remove the unnecessary runtime check in optimized code.

The number of indices must match the array [rank](../specification/glossary.md) (its number of
dimensions). Each index must be an integer.
The compiler performs known bounds checks during compilation and retains
runtime checks for indices whose values are not known yet.

## Struct expressions and field access

A struct expression creates a value, and a field-access expression reads part
of it:

```vortex
let point = Point {
    x: 10.0,
    y: 20.0,
};

let horizontal = point.x;
```

A struct expression must provide every declared field exactly once with a
compatible value. Unknown, duplicate, or missing fields are invalid. Field
access requires a field that exists on the value's resolved struct type.

## Range expressions

Ranges are mainly used by loops:

```vortex
0..10   // includes 0, but does not include 10
0..=10  // includes both 0 and 10
```

For example:

```vortex
for index in 0..4 {
    print(values[index]);
}
```

Vortex v0.1 ranges have both endpoints; open-ended ranges such as `..10`, `0..`, and
`..` are not supported. Range endpoints must have compatible integer types when
the range is used for iteration.

## Cast expressions

Vortex uses Python-style casting. Write the new type like a function and pass it
the value you want to convert:

```vortex
let count: i32 = 10;
let count_as_float = f32(count);

let temperature: f32 = 21.8;
let whole_degrees = i32(temperature);
```

This syntax is simple, but conversion rules still need to be safe and
predictable:

- converting a smaller integer into a larger integer keeps the same value;
- converting an integer into a floating-point number may lose precision for
  very large values;
- converting a floating-point number into an integer removes the decimal part
  by rounding toward zero;
- converting a value that is outside the new type's range causes a checked
  runtime error;
- when an invalid conversion is visible during compilation, the compiler
  reports it before the program runs.

For example, this conversion is invalid because a negative value cannot fit in
an unsigned `u32`:

```vortex
let value = u32(-1); // Compile-time error
```

Vortex should not silently wrap the value into a different number. Explicit
wrapping and saturating conversions may be added later under clearly named
operations.

Converting text into numbers is a different operation because the text may not
contain a valid number. A future version should use an operation such as
`i32::parse(text)` instead of treating text parsing as an ordinary cast.

Only primitive type names can act as the v0.1 cast target. A normal function
call and a cast share parser syntax; name and type checking determine which one
the name denotes.

## Operator precedence

Precedence decides which operation happens first when parentheses are not used.
The operators near the top of this table run before operators near the bottom:

| Order | Operators | Purpose |
| --- | --- | --- |
| 1 | `()` `[]` `.` | calls, indexing, and field access |
| 2 | unary `+` `-` `!` `~` | operations on one value |
| 3 | `*` `/` `%` | multiplication, division, and remainder |
| 4 | `+` `-` | addition and subtraction |
| 5 | `<<` `>>` | bit shifts |
| 6 | `<` `<=` `>` `>=` | ordered comparisons |
| 7 | `==` `!=` | equality comparisons |
| 8 | `&` | bitwise and |
| 9 | `^` | bitwise exclusive or |
| 10 | `\|` | bitwise or |
| 11 | `&&` | logical and |
| 12 | `\|\|` | logical or |
| 13 | `..` `..=` | ranges |

When the intended order is not obvious, use parentheses. Clear code is more
important than memorizing this table.

## Expressions that can wait

The first version of Vortex does not need every possible expression. These can
be considered later:

- conditional expressions such as `condition ? yes : no`;
- `if` blocks that directly produce values;
- anonymous functions and closures;
- pattern matching;
- overloaded operators for user-created types;
- general compile-time expressions outside the array-dimension positions that
  specifically require compile-time evaluation;
- tensor expressions that operate on complete tensors at once.

## Compiler handling summary

<details markdown="1">
<summary>Which compiler stage enforces each rule (optional reading)</summary>

1. The lexer recognizes literal, identifier, delimiter, and operator tokens.
2. The parser applies precedence and builds expression [AST](../specification/glossary.md)
   (abstract syntax tree) nodes.
3. Name resolution binds names, calls, fields, and named types.
4. Type checking validates operands and assigns every expression a result type.
5. Constant evaluation reduces required compile-time expressions, including
   array dimensions.
6. Code generation preserves evaluation order, short circuiting, and required
   runtime checks.

</details>

## Practice and self-check

Classify each line by its outermost expression form and decide whether it is
valid:

```vortex
-(left + right)
values[index + 1]
[0.0; 2 + 2, rows]
ready && count
Point { x: 1.0, y: 2.0 }
```

Answers:

1. Valid unary expression if `left` and `right` have compatible signed numeric
   types.
2. Valid index expression if `index + 1` is an integer and the access is in
   bounds.
3. A repeat-array expression, but invalid when `rows` is runtime-dependent.
4. Invalid unless `count` is `bool`.
5. Valid struct expression when `Point` declares exactly those compatible
   fields.
