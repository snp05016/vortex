# Statements

A statement tells Vortex to do something. Unlike an expression, a statement is
usually used for its effect: creating a variable, changing a value, choosing a
path, repeating work, or leaving a function.

Statements in v0.1 end with a semicolon when they are written on one line.
Blocks such as `if` and `for` use braces and do not need a semicolon after the
closing brace.

## Variable declarations

Use `let` to create a variable:

```vortex
let width = 128;
let mut total: f32 = 0.0;
```

The first variable cannot change. The second can change because it uses `mut`.

## Assignment statements

An assignment changes a mutable variable or a mutable array element:

```vortex
let mut total = 0;
total = 10;

let mut values = [1.0, 2.0, 3.0];
values[0] = 10.0;
```

Vortex does not allow assignment to a variable created without `mut`.

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

## Expression statements

An expression can be used as a statement when you only care about its effect.
Function calls that print information are the most common example:

```vortex
print("starting calculation");
save_results(values);
```

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

## While loops

A `while` loop repeats as long as its condition stays `true`:

```vortex
let mut index = 0;

while index < 4 {
    print(index);
    index += 1;
}
```

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
