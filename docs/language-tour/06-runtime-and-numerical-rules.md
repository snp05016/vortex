# Runtime and Numerical Rules

[Previous: Types planned for later](05-types-planned-for-later.md) | [Tour index](README.md) | [Next: Kernels and parallel execution](07-kernels-and-parallel-execution.md)

## Learning goals

After this chapter, you should be able to distinguish compile-time errors from
runtime checks and explain Vortex's default integer and floating-point behavior.

## Checked operations

Vortex v0.1 uses checked behavior for common mistakes. A runtime error stops the
program and reports what went wrong.

- Accessing an array outside its bounds is a runtime error.
- Dividing an integer by zero is a runtime error.
- Integer overflow is a runtime error.
- A cast that cannot produce a value in the destination type is a runtime error.
- When the compiler can prove one of these errors will happen, it reports a
  compile-time error instead.

## Allowed examples

```vortex
let values = [10, 20, 30];
let index: usize = 1;
let selected = values[index];

let numerator = 12;
let denominator = 3;
let result = numerator / denominator;
```

These operations are valid. When `index` or `denominator` is not known during
compilation, generated code keeps the required runtime check.

## Rejected or checked examples

```vortex
let values = [10, 20, 30];
let selected = values[3];
// compile-time error when the compiler proves index 3 is out of bounds
```

```vortex
fn divide(value: i32, divisor: i32) -> i32 {
    return value / divisor;
    // valid source, but execution fails if divisor is zero
}
```

```vortex
let impossible = u32(-1);
// compile-time error because the invalid conversion is already known
```

Unchecked overflow, silent wrapping, saturating casts, and a user-controlled
`unsafe` escape hatch are not part of v0.1.

## Floating-point behavior

`f32` and `f64` follow IEEE 754 floating-point behavior. In v0.1, the compiler
does not reorder floating-point operations in a way that changes their result.
Faster relaxed numerical modes may be added later when their behavior can be
made explicit.

Floating-point results are not exact decimal arithmetic. Programs should not
assume that every decimal calculation can be represented perfectly. V0.1 also
does not provide a fast-math flag, implicit integer-to-float conversion, or a
special decimal-money type.

## Compiler handling

Type checking first verifies that an operation is defined for its operand
types. Constant evaluation reports provably invalid operations during
compilation. For values known only at runtime, code generation emits overflow,
division, cast, and bounds checks as required. An optimizer may remove a check
only after proving the operation safe.

## Practice and self-check

For each case, decide whether the compiler can reject it immediately or must
keep a runtime check:

```vortex
let a = 10 / 0;
let b = values[100];

fn read(values: &[i32; 4], index: usize) -> i32 {
    return values[index];
}
```

The first two can be compile-time errors when the values and array shape are
known. The function needs a runtime bounds check because `index` is supplied by
its caller.
