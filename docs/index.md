<div class="home-hero">
  <div>
    <h1>Vortex language specification</h1>
    <p>A precise, implementation-oriented specification for a statically typed language focused on numerical kernels, fixed shapes, and explicit mutation.</p>
  </div>
  <pre class="home-example"><code>fn dot(left: [f32; 4], right: [f32; 4]) -&gt; f32 {
    let mut result: f32 = 0.0;
    for index in 0..4 {
        result += left[index] * right[index];
    }
    return result;
}</code></pre>
</div>

Vortex v0.1 is an evolving language design and compiler project. This site
separates the language contract from implementation guidance so that a parser,
type checker, runtime, and programmer can agree on the same behavior.

!!! important "Specification status"
    The specification defines the intended v0.1 language. It does not claim
    that every described feature is implemented by the current compiler. Check
    the [implementation roadmap](roadmap.md) and tests before relying on a
    feature in the compiler.

## Choose your path

<div class="chapter-grid">
  <a class="chapter-link" href="specification/">
    <strong>Read the specification</strong>
    <span>Normative rules for syntax, types, behavior, and diagnostics.</span>
  </a>
  <a class="chapter-link" href="language-tour/">
    <strong>Learn the language</strong>
    <span>Example-driven chapters for writing small Vortex programs.</span>
  </a>
  <a class="chapter-link" href="compiler/architecture/">
    <strong>Build the compiler</strong>
    <span>Pass boundaries, AST ownership, parser design, and verification.</span>
  </a>
  <a class="chapter-link" href="language-and-compiler-cheatsheet/">
    <strong>Look up a term</strong>
    <span>A compact reference for language and compiler terminology.</span>
  </a>
</div>

## v0.1 at a glance

| Area | Included in v0.1 |
| --- | --- |
| Program structure | Top-level functions and structs, with exactly one valid `main` for an executable |
| Primitive types | `void`, `bool`, `char`, `i32`, `u32`, `usize`, `f32`, `f64`, and `String` |
| Compound types | Fixed-size arrays, user-defined structs, and basic references |
| Values | Literals, names, array values, repeat arrays, and struct values |
| Expressions | Unary, binary, range, call or cast, index, field access, and grouping |
| Statements | Local declarations, assignment, return, expression statements, blocks, conditionals, and loops |
| Safety model | Explicit mutation, name and type checks, fixed-shape validation, and documented runtime checks |

The first target is a correct CPU compiler for straightforward fixed-size
matrix multiplication. GPU execution, runtime-sized collections, tensors,
generics, modules, and advanced scheduling are later design work.

## One complete example

```vortex
struct Point {
    x: f32,
    y: f32,
}

fn length_squared(point: Point) -> f32 {
    return point.x * point.x + point.y * point.y;
}

fn main() {
    let point = Point { x: 3.0, y: 4.0 };
    print(length_squared(point));
}
```

This program demonstrates both top-level declaration forms, inferred local
types, struct construction, field access, arithmetic, a function call, and the
required entry point.

## How the documents relate

```text
Language specification
    defines accepted programs and their required meaning

Formal grammar
    defines the exact token structure accepted by the parser

Language guide
    teaches the same rules through examples and exercises

Compiler documentation
    explains how this repository can implement and test those rules
```

When two documents disagree about v0.1 syntax, the
[formal grammar](specification/grammar.md) is authoritative. When the grammar
accepts a form whose meaning is constrained later, the relevant specification
chapter defines the semantic rule.

## Design direction

Vortex is designed around three commitments:

1. Expose useful information about types, shapes, memory access, and mutation.
2. Keep the mathematical computation separate from hardware scheduling.
3. Prefer explicit, testable behavior over hidden performance assumptions.

Read the [language philosophy](philosophy.md) for the full rationale and the
[v0.1 roadmap](roadmap.md) for the implementation sequence.
