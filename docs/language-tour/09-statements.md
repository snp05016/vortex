# Statements

## Learning goals

After this chapter, you should be able to recognize every v0.1 statement,
identify where semicolons are required, and explain the contextual checks for
assignment, loops, and return.

A statement tells Vortex to do something. Unlike an expression, a statement is
usually used for its effect: creating a variable, changing a value, choosing a
path, repeating work, or leaving a function.

Statements in v0.1 end with a semicolon when they are written on one line.
Blocks such as `if` and `for` use braces and do not need a semicolon after the
closing brace.

## Statement quick reference

| Statement | Example | Semicolon after it? |
| --- | --- | --- |
| Variable declaration | `let count = 0;` | Yes |
| Assignment | `count += 1;` | Yes |
| Expression statement | `print(count);` | Yes |
| Return | `return count;` | Yes |
| Break/continue | `break;` | Yes |
| Block | `{ ... }` | No |
| `if`/`else` | `if ready { ... }` | No |
| `while` | `while ready { ... }` | No |
| `for` | `for i in 0..4 { ... }` | No |

Vortex v0.1 does not include declaration-only locals, `switch`, `match`, `do while`,
exceptions, `defer`, or parallel-loop statements.

## Variable declarations

Use `let` to create a variable:

```vortex
let width = 128;
let mut total: f32 = 0.0;
```

The first variable cannot change. The second can change because it uses `mut`.

Every variable declaration needs an initializer. The optional type annotation
must agree with that expression. [Parsing](../compiler/guide/index.md) (the compiler stage that reads the structure of the code)
checks the syntax; type checking and
local-scope creation validate the declaration.

## Assignment statements

An assignment changes a mutable variable or a mutable array element:

```vortex
let mut total = 0;
total = 10;

let mut values = [1.0, 2.0, 3.0];
values[0] = 10.0;
```

Vortex does not allow assignment to a variable created without `mut`.

The target may be a mutable variable, mutable field, or mutable array element.
A literal, calculation, call result, or immutable location is not assignable:

```vortex
10 = value;            // invalid: literal is not a location
(left + right) = 0;    // invalid: calculation is not a location
let fixed = 1;
fixed = 2;             // invalid: fixed is immutable
```

The parser recognizes the restricted assignment-target grammar. [Semantic
analysis](../specification/glossary.md) (the checks on meaning that run after parsing) checks that
the resolved storage is mutable and the value type is
compatible.

## Compound assignment statements

Compound assignments combine an operation with assignment:

| Statement | Same as |
| --- | --- |
| `total += value;` | `total = total + value;` |
| `total -= value;` | `total = total - value;` |
| `total *= value;` | `total = total * value;` |
| `total /= value;` | `total = total / value;` |
| `index %= size;` | `index = index % size;` |

For example:

```vortex
let mut total: f32 = 0.0;
total += 2.5;
```

The underlying operator must be valid for the target type. For example,
`name %= 2;` is invalid when `name` is a `String`. The target is evaluated once;
compound assignment is not permission to duplicate side effects.

## Expression statements

An expression can be used as a statement when you only care about its effect.
Function calls that print information are the most common example:

```vortex
print("starting calculation");
save_results(values);
```

Any grammatical expression can appear before `;`, but using a pure value and
discarding it has no useful effect. A call returning `void` cannot be used as a
stored value, but it is valid as an expression statement.

## Blocks and scope

Braces create a block. A variable created inside a block exists only inside that
block:

```vortex
{
    let temporary = 42;
    print(temporary);
}

// `temporary` cannot be used here.
```

Name resolution opens a scope at `{` and closes it at `}`. A block may contain
zero or more statements. Vortex v0.1 does not use a block itself as a value.

## If and else statements

Use `if` to run code only when a condition is `true`. Use `else` when the
condition is `false`:

```vortex
if score >= 50 {
    print("passed");
} else {
    print("try again");
}
```

You can chain conditions with `else if`:

```vortex
if temperature < 0 {
    print("freezing");
} else if temperature > 30 {
    print("hot");
} else {
    print("comfortable");
}
```

In v0.1, `if` is a statement. It chooses which code runs but does not directly
produce a value. Value-producing `if` expressions can be added later.

Every condition must have type `bool`; Vortex does not treat numbers or strings
as truth values. Each branch is a block, while `else if` is an `else` followed
by another `if` statement.

```vortex
if 1 {
    print("invalid");
}
// invalid: condition is i32, not bool
```

## While loops

A `while` loop repeats as long as its condition stays `true`:

```vortex
let mut index = 0;

while index < 4 {
    print(index);
    index += 1;
}
```

The condition must be `bool`. The body may execute zero times. A C-style loop
such as `for (let i = 0; i < 4; i += 1)` is not part of Vortex.

## For loops

A `for` loop repeats over a range. It is the normal choice when you know the
set of indexes you want to visit:

```vortex
for index in 0..4 {
    print(index);
}
```

This loop visits `0`, `1`, `2`, and `3`. The end value is not included. Use
`..=` when you want to include the end value:

```vortex
for index in 0..=4 {
    print(index);
}
```

For loops are especially important in Vortex because matrix and tensor code is
mostly made from clear, nested loops.

The loop variable is introduced by the loop and is visible only inside its
body. The expression after `in` must be iterable; v0.1 uses integer ranges.
`for index = 0..4` is invalid because the required keyword is `in`.

## Break and continue

Use `break` to leave a loop immediately:

```vortex
for index in 0..100 {
    if values[index] == target {
        break;
    }
}
```

Use `continue` to skip the rest of the current loop iteration:

```vortex
for value in 0..10 {
    if value % 2 == 0 {
        continue;
    }

    print(value);
}
```

`break` and `continue` are valid only inside the nearest enclosing loop. They
are invalid in an ordinary block or directly inside a function with no loop.
This is a semantic context check rather than a parsing decision.

## Return statements

Use `return` to end a function and provide its result:

```vortex
fn larger(left: i32, right: i32) -> i32 {
    if left > right {
        return left;
    }

    return right;
}
```

A `void` function can use `return;` when it needs to finish early:

```vortex
fn print_positive(value: i32) -> void {
    if value <= 0 {
        return;
    }

    print(value);
}
```

- A non-`void` function must return a compatible value on every reachable
  path.
- A `void` function may use `return;` but cannot return a value.
- A non-`void` function cannot use an empty `return;`.

```vortex
fn bad() -> i32 {
    return; // invalid: i32 result is missing
}
```

## Statements that can wait

The first version does not need these yet:

- `match` statements and pattern matching;
- `switch` statements;
- `do while` loops;
- exception handling;
- `defer` cleanup statements;
- `unsafe` blocks;
- parallel-loop statements;
- GPU-kernel launch statements.

## Compiler handling summary

<details markdown="1">
<summary>Which compiler stage enforces each rule (optional reading)</summary>

The parser identifies statement boundaries and builds statement [AST](../specification/glossary.md) (abstract syntax tree) nodes.
Name resolution manages block and loop-variable scopes. Type checking validates
conditions, assignments, expressions, and returned values.

Control-flow analysis checks loop-only statements and verifies required return paths. Code
generation emits the selected branches, loop edges, and early exits.

</details>

## Practice and self-check

Identify the invalid lines and explain each failure:

```vortex
let value;
let fixed = 1;
fixed += 1;
break;
if 42 { print("answer"); }
```

Answers:

1. A local declaration requires an initializer.
2. `let fixed = 1;` is valid.
3. `fixed` is immutable.
4. `break` is outside a loop.
5. An `if` condition must be `bool`.
