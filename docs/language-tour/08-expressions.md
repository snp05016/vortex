# Expressions

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

## Name expressions

Writing the name of a variable reads its value:

```vortex
let width = 128;
let area = width * width;
```

Here, both uses of `width` are expressions.

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
let valid = index < values.len() && values[index] > 0.0;
```

If the index is outside the array, the second part is not evaluated.

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

## Array expressions

An array literal creates an array:

```vortex
let values = [1.0, 2.0, 3.0, 4.0];
```

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

## Range expressions

Ranges are mainly used by loops:

```vortex
0..10   // includes 0, but does not include 10
0..=10  // includes both 0 and 10
```

For example:

```vortex
for index in 0..values.len() {
    print(values[index]);
}
```

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
- compile-time expressions;
- tensor expressions that operate on complete tensors at once.
