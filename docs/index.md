# Vortex language specification

<p class="page-intro">A precise, implementation-oriented specification for a statically typed language focused on numerical kernels, fixed shapes, and explicit mutation.</p>

<nav class="quick-links" aria-label="Start reading">
  <a href="specification/">Specification</a>
  <a href="language-tour/">Language guide</a>
  <a href="compiler/architecture/">Compiler guide</a>
  <a href="language-and-compiler-cheatsheet/">Cheat sheet</a>
</nav>

Vortex v0.1 is an evolving language design and compiler project. This site
separates the language contract from implementation guidance so that a parser,
type checker, runtime, and programmer can agree on the same behavior.

!!! important "Specification status"
    The specification defines the intended v0.1 language. It does not claim
    that every described feature is implemented by the current compiler. Check
    the [implementation roadmap](roadmap.md) and tests before relying on a
    feature in the compiler.

## Read the documentation

<div class="document-index">
  <div class="document-link">
    <a href="specification/">Language specification</a>
    <span>Normative rules for syntax, types, behavior, and diagnostics.</span>
  </div>
  <div class="document-link">
    <a href="language-tour/">Language guide</a>
    <span>Example-driven chapters for learning to write Vortex programs.</span>
  </div>
  <div class="document-link">
    <a href="compiler/architecture/">Compiler implementation</a>
    <span>Pass boundaries, parser design, AST ownership, and verification.</span>
  </div>
  <div class="document-link">
    <a href="language-and-compiler-cheatsheet/">Language and compiler cheat sheet</a>
    <span>A compact lookup page for language and compiler terminology.</span>
  </div>
</div>

## Specification chapters

<ol class="chapter-list">
  <li><span class="chapter-number">01</span><a href="specification/conformance/">Conformance and terminology</a><span>How to read normative requirements and determine whether an implementation conforms.</span></li>
  <li><span class="chapter-number">02</span><a href="specification/lexical-structure/">Lexical structure</a><span>Source text, identifiers, literals, comments, keywords, and punctuation.</span></li>
  <li><span class="chapter-number">03</span><a href="specification/declarations/">Programs and declarations</a><span>Translation units, functions, structs, scopes, and the program entry point.</span></li>
  <li><span class="chapter-number">04</span><a href="specification/types-and-values/">Types and values</a><span>Primitive types, compound types, values, conversions, and type equivalence.</span></li>
  <li><span class="chapter-number">05</span><a href="specification/expressions/">Expressions</a><span>Operators, calls, casts, indexing, field access, evaluation, and precedence.</span></li>
  <li><span class="chapter-number">06</span><a href="specification/statements/">Statements and control flow</a><span>Bindings, assignment, return, blocks, branches, and loops.</span></li>
  <li><span class="chapter-number">07</span><a href="specification/arrays/">Arrays and shapes</a><span>Fixed-size array types, construction, indexing, assignment, and nested arrays.</span></li>
  <li><span class="chapter-number">08</span><a href="specification/structs/">Structs</a><span>Struct declarations, values, fields, nested structs, and array-valued fields.</span></li>
  <li><span class="chapter-number">09</span><a href="specification/references/">References and mutability</a><span>Borrowed access, mutation permissions, assignment targets, and lifetime boundaries.</span></li>
  <li><span class="chapter-number">10</span><a href="specification/diagnostics/">Diagnostics</a><span>Required errors, implementation limits, warnings, and source reporting.</span></li>
  <li><span class="chapter-number">11</span><a href="specification/grammar/">Formal grammar</a><span>The parser-facing grammar and its relationship to semantic requirements.</span></li>
  <li><span class="chapter-number">12</span><a href="specification/glossary/">Glossary</a><span>Definitions of the terms used throughout the specification.</span></li>
</ol>

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

## Authorship and methodology

Large language models were used heavily to save time on drafting, restructuring,
and transferring design notes into documentation. The language specification
itself was developed with both human intervention and LLM assistance. Human
review remains part of resolving language decisions and checking the grammar,
examples, and cross-page rules for inconsistencies.

This process improves coverage but does not guarantee that the evolving
specification is error-free. Contradictions and unclear requirements should be
reported and resolved in the specification. Read the full
[authorship and methodology statement](authorship.md).

## Design direction

Vortex is designed around three commitments:

1. Expose useful information about types, shapes, memory access, and mutation.
2. Keep the mathematical computation separate from hardware scheduling.
3. Prefer explicit, testable behavior over hidden performance assumptions.

Read the [language philosophy](philosophy.md) for the full rationale and the
[v0.1 roadmap](roadmap.md) for the implementation sequence.
