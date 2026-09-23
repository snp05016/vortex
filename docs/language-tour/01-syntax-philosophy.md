# Syntax philosophy

## Learning goals

After this chapter, you should be able to explain why Vortex is statically
typed, where its syntax is familiar, and which simplicity rules define v0.1.

## Core design

Vortex uses familiar ideas from Rust and C++, while leaving room for its own
numerical and CPU/GPU features.

- Vortex is statically typed. Every value has a known type before the program
  runs.
- Vortex uses `fn` for functions and `let` for local variables.
- Vortex can infer the type of a local variable from its starting value.
- Function parameter and return types are written explicitly in v0.1, except
  that an omitted function return type means `void`.
- Vortex uses `print` for simple output and debugging.
- Vortex uses `{}` for code blocks.

## What v0.1 allows

- Top-level function and struct declarations.
- Explicit parameter types and optional local-variable type annotations.
- Fixed-size arrays whose dimensions are compile-time integer expressions.
- Straightforward statement-based control flow.
- Shared and mutable references.

## What v0.1 does not allow

- Dynamic typing or changing a variable's type after declaration.
- Classes, inheritance, traits, generics, modules, or packages.
- Closures, pattern matching, or value-producing `if` expressions.
- Accelerator kernels or parallel-loop syntax. These are planned for later.

## Valid example

```vortex
fn square(value: f32) -> f32 {
    return value * value;
}

fn main() {
    let result = square(4.0);
    print(result);
}
```

This example is valid because every value has a statically known type, the
parameter and non-`void` return type are explicit, and statements use the
v0.1 syntax.

## Invalid example

```vortex
fn main() {
    let value = 10;
    value = "ten";
}
```

This fails for two reasons: `value` is immutable because it lacks `mut`, and a
variable inferred as `i32` cannot later contain a `String`.

## Compiler handling

<details markdown="1">
<summary>Which compiler stage enforces each rule (optional reading)</summary>

The parser (the [compiler stage](../compiler/guide/index.md) that checks how tokens fit together and builds the program's structure) recognizes declarations, blocks, and statements. Name resolution
connects `square` and `value` to their declarations. Type checking assigns or
verifies every value's type and rejects incompatible operations before code
generation.

</details>

## Self-check

**Question:** Does type inference make Vortex dynamically typed?

**Answer:** No. Inference saves the programmer from writing an obvious type,
but the compiler still chooses and checks one fixed type before execution.
