# 10. Diagnostics

Every ill-formed Vortex program must be rejected. Diagnostics explain which
rule failed and identify the relevant source span. This chapter defines the
minimum diagnostic taxonomy; exact prose and internal C++ exception types are
implementation details.

## 10.1 Required diagnostic data

Every diagnostic should contain:

- a category;
- a concise primary message;
- a primary source span;
- the unexpected or invalid construct;
- the expected form or violated rule when that information is useful;
- optional notes pointing to related declarations or source spans.

Diagnostics must not rely only on an internal enum name or raw token number.

## 10.2 Categories

### Lexical error

The source characters cannot form a valid token. Examples include an unknown
character, unterminated string, malformed number, and unsupported escape.

### Syntax error

The token sequence does not match the grammar. Examples include a missing
semicolon, unmatched delimiter, missing type after `:`, or unexpected token at
top level.

### Name error

A name is unknown, duplicated in the same scope, or used outside its scope. A
named type that does not resolve also produces a name error.

### Type error

An operation receives unsupported or incompatible operand types, an
initializer does not match its annotation, a call has invalid arguments, an
assignment value is incompatible, or a return value does not match the
function result type.

### Semantic error

The syntax and local types are valid, but a contextual rule fails. Examples
include assigning immutable storage, taking `&mut` from immutable storage,
using `break` outside a loop, or providing an invalid `main` signature.

### Constant-evaluation error

An expression required at compile time cannot be evaluated to a valid value.
Array extents that depend on runtime values, are noninteger, are negative, or
overflow supported layout limits are in this category.

### Runtime error

A valid program reaches a checked operation whose dynamic values violate a
runtime rule. Required v0.1 checks include:

- array index out of bounds;
- integer division or remainder by zero;
- checked integer overflow;
- a checked cast whose value cannot be represented by its destination type.

When the compiler can prove the failure, it should diagnose it before
execution instead.

### Implementation-limit error

The program uses specified v0.1 behavior that the current compiler has not yet
implemented. This category is appropriate during compiler development. It must
not be used to disguise a crash or silently ignore source.

## 10.3 Error phase versus category

Category and detection phase are related but not identical. An out-of-bounds
constant index can be detected during compilation, while an input-dependent
index requires a runtime check. Both violate the same bounds rule.

Diagnostics should describe the language failure rather than forcing users to
understand the compiler pass that happened to detect it.

## 10.4 Examples

```vortex
let value = "unterminated;
```

Required result: lexical error at the unterminated string.

```vortex
let value i32 = 10;
```

Required result: syntax error identifying the missing `:` or unexpected type
token.

```vortex
let value: bool = 10;
```

Required result: type error relating the initializer to the written type.

```vortex
let value = 10;
value = 20;
```

Required result: semantic error pointing to the immutable assignment target,
with an optional note at the declaration.

```vortex
fn make(size: usize) {
    let values: [f32; size] = [0.0; size];
}
```

Required result: constant-evaluation error explaining that a fixed dimension
depends on a runtime parameter.

## 10.5 Recovery

After a syntax error, a parser may synchronize at:

- `;` for a simple statement;
- `}` for a block or declaration body;
- the next top-level `fn` or `struct` token.

Recovery should report useful independent errors without producing cascades
from one missing token. A compiler may stop after a documented maximum number
of errors.

## 10.6 Runtime reporting

A runtime failure must stop the affected program execution and identify the
error category. When source-location metadata is available, the report should
include the originating operation.

Vortex v0.1 does not specify exception handling inside the language, so a Vortex
program cannot catch these runtime failures.

## 10.7 Verification requirements

Each diagnostic rule should have:

- one minimal rejected program;
- the expected category;
- an assertion that the primary source span covers the relevant construct;
- a nearby accepted program that proves the test is not rejecting a broader
  valid form.
