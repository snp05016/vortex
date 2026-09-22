# Vortex v0.1 Language Tour

This directory is the practical, example-driven reference for writing Vortex
v0.1 programs. Read it in order the first time. Later, use the tables below to
jump directly to a rule or example.

The tour describes the language programmers can write. The
[formal grammar](../specification/grammar.md) is the parser contract, and the
[compiler documentation](../compiler/parser-design.md) explains how the
frontend implements that contract. When the documents disagree, treat the
formal grammar as authoritative and report the mismatch.

## How each chapter is organized

Each chapter follows the same assignment-style pattern:

1. **Learning goals** state what you should understand after reading.
2. **Syntax** shows the accepted source form.
3. **Allowed** lists what v0.1 accepts.
4. **Not allowed** marks syntax or behavior that is outside v0.1.
5. **Valid examples** show programs the compiler should accept.
6. **Invalid examples** show programs the compiler should reject and why.
7. **Compiler handling** identifies whether the lexer, parser, semantic
   analyzer, or runtime enforces the rule.
8. **Practice and self-check** gives a short exercise with an answer.

An invalid example is intentionally invalid. Do not copy one into a program
unless you are testing diagnostics.

## Guided reading order

| Part | Chapter | Use it to answer |
| --- | --- | --- |
| 1 | [Syntax philosophy](01-syntax-philosophy.md) | What kind of language is Vortex? |
| 2 | [Basic source rules](02-basic-source-rules.md) | How do names, comments, semicolons, and escapes work? |
| 3 | [Hello world and program entry](03-hello-world.md) | What must an executable program contain? |
| 4 | [Variables and types](04-variables-and-types.md) | How are values typed, stored, grouped, and referenced? |
| 5 | [Types planned for later](05-types-planned-for-later.md) | Which useful types are deliberately outside v0.1? |
| 6 | [Runtime and numerical rules](06-runtime-and-numerical-rules.md) | Which numerical failures are checked, and when? |
| 7 | [Kernels and parallel execution](07-kernels-and-parallel-execution.md) | Which accelerator features are planned but unavailable? |
| 8 | [Expressions](08-expressions.md) | Which pieces of syntax produce values? |
| 9 | [Statements](09-statements.md) | Which pieces of syntax perform actions and control flow? |
| 10 | [Declarations](10-declarations.md) | How are functions, structs, parameters, fields, and variables introduced? |

## Quick feature index

| Feature | Primary chapter |
| --- | --- |
| Arrays, dimensions, repeat arrays, indexing | [Variables and types](04-variables-and-types.md#fixed-size-arrays) and [Expressions](08-expressions.md#array-expressions) |
| Assignment and mutability | [Statements](09-statements.md#assignment-statements) |
| Binary and unary operators | [Expressions](08-expressions.md#unary-expressions) |
| Casts | [Expressions](08-expressions.md#cast-expressions) |
| Functions and `main` | [Declarations](10-declarations.md#function-declarations) |
| References | [Variables and types](04-variables-and-types.md#references) |
| Scope and names | [Declarations](10-declarations.md#names-and-scopes) |
| Struct definitions and values | [Declarations](10-declarations.md#struct-declarations) and [Expressions](08-expressions.md#struct-expressions-and-field-access) |
| `if`, loops, `break`, `continue`, and `return` | [Statements](09-statements.md) |
| Runtime errors and floating-point behavior | [Runtime and numerical rules](06-runtime-and-numerical-rules.md) |

## Compiler-stage legend

| Stage | Responsibility |
| --- | --- |
| Lexer | Turns source characters into tokens and reports malformed tokens. |
| Parser | Checks grammatical structure and builds the AST. |
| Name resolution | Connects each used name to a declaration and checks scope. |
| Type checking | Checks operand, argument, return, field, index, and assignment types. |
| Constant evaluation | Evaluates array-dimension expressions and other required compile-time values. |
| Runtime/code generation | Emits checks and executable behavior for values not known during compilation. |

## v0.1 boundary

The label **v0.1** means the small language described by the current grammar.
Features marked **planned for later** are design notes, not accepted syntax.
Keeping this boundary explicit prevents examples from promising compiler
behavior that has not been designed yet.
