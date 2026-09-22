# Vortex v0.1 Roadmap

This roadmap is organized like an implementation assignment. Each milestone has
an objective, required work, and an observable completion condition. Do not mark
a milestone complete merely because its main code path exists.

## Contents

- [How to use this roadmap](#how-to-use-this-roadmap)
- [Goal](#goal)
- [Milestones 0-3: compiler front end](#milestone-0-project-foundation)
- [Milestones 4-5: names and types](#milestone-4-names-and-scopes)
- [Milestones 6-10: executable programs](#milestone-6-basic-cpu-code-generation)
- [Milestone 11: release gate](#milestone-11-v01-release-gate)
- [Definition of done](#definition-of-done)
- [After v0.1](#after-v01)

## How to use this roadmap

For every milestone:

1. Read the matching grammar and language-tour sections.
2. Write at least one valid example and one invalid example before implementation.
3. Implement the smallest complete vertical slice.
4. Add positive, negative, and source-location tests.
5. Run all earlier milestone tests to detect regressions.
6. Record known limitations rather than silently accepting partial behavior.

### Evidence required before calling a milestone complete

- The project builds from a clean configuration.
- All automated tests pass.
- Every documented valid example for the milestone succeeds.
- Every documented invalid example fails for the documented reason.
- Diagnostics point at the relevant source span.
- The AST or generated output is stable enough to inspect.

### Scope rule

Milestones are cumulative. Work from a later milestone may be prototyped, but it
must not weaken or bypass an earlier milestone's correctness rules.

## Goal

Vortex v0.1 should compile a small Vortex source file into a correct CPU
executable. The final demonstration is a straightforward, unoptimized matrix
multiplication using fixed-size multidimensional arrays.

The first release is about correctness, useful errors, and a complete compiler
pipeline. Performance optimization begins after v0.1.

## Milestone 0: Project foundation

- Choose the implementation language and build system.
- Create a `vortex` command that can accept a source-file path.
- Add an automated test command.
- Create folders for valid programs, invalid programs, and expected output.
- Make one command build the compiler and run all tests.

This milestone is complete when the empty compiler builds reliably and the test
runner can report a passing and a failing test.

## Milestone 1: Source files and diagnostics

- Read a Vortex source file.
- Track file names, line numbers, and column numbers.
- Create one consistent diagnostic format for errors.
- Show the relevant source line and point to the problem.

This milestone is complete when the compiler can report a readable error at an
exact location in a source file.

## Milestone 2: Lexer

The lexer turns source text into tokens.

- Recognize identifiers and v0.1 keywords.
- Recognize integer, floating-point, boolean, character, and string literals.
- Recognize operators, punctuation, braces, and semicolons.
- Handle whitespace, `//` comments, and escape sequences.
- Report invalid characters and unfinished literals.
- Add tests for every token and lexer error.

This milestone is complete when a Vortex file can be printed as a correct token
stream with accurate source locations.

## Milestone 3: Parser and syntax tree

The parser turns tokens into a syntax tree.

- Parse functions, parameters, return types, and `main`.
- Parse variable declarations, assignments, blocks, and return statements.
- Parse `if`, `else`, `while`, `for`, `break`, and `continue`.
- Parse expressions using the documented operator precedence.
- Parse function calls, casts, arrays, indexing, structs, and references.
- Recover from simple syntax errors so one mistake does not hide every later
  error.
- Add parser tests for valid and invalid programs.

This milestone is complete when the compiler can print a stable syntax tree for
every v0.1 language construct.

## Milestone 4: Names and scopes

- Build a symbol table for functions, variables, parameters, and struct fields.
- Reject unknown names.
- Reject duplicate names in the same scope.
- Keep variables inside the blocks where they were declared.
- Validate that the program has exactly one valid `main` function.
- Add tests for shadowing, duplicate names, and out-of-scope variables.

This milestone is complete when every name in a program resolves to one known
declaration.

## Milestone 5: Types, mutability, and control-flow checks

- Implement the v0.1 types: `void`, `bool`, `char`, `i32`, `u32`, `usize`,
  `f32`, `f64`, `String`, fixed-size arrays, structs, and basic references.
- Infer local-variable types from their starting values.
- Check operators, assignments, function arguments, and return values.
- Check array dimensions and indexes.
- Parse array dimensions as expressions, evaluate them during compilation, and
  require integer results representable by the supported array-size model.
- Reject runtime-dependent, negative, non-integer, and overflowing array
  dimensions with dimension-specific diagnostics.
- Decide and document the zero-length-array policy before marking this
  milestone complete; parsing must not decide that semantic rule.
- Reject changes to immutable values.
- Enforce the basic shared-reference and mutable-reference rules.
- Verify that non-`void` functions return a value on every path.
- Reject `break` and `continue` outside loops.
- Add a negative test for every rule.

This milestone is complete when invalid programs are rejected before code
generation with clear explanations.

Required dimension examples:

```vortex
let matrix: [f32; 2 + 2, 8] = [0.0; 2 + 2, 8]; // valid: constant expressions
```

```vortex
fn invalid(rows: usize) {
    let matrix: [f32; rows, 8] = [0.0; rows, 8];
    // invalid in v0.1: rows is only known at runtime
}
```

## Milestone 6: Basic CPU code generation

- Generate CPU code for `main`, literals, arithmetic, variables, and `return`.
- Emit an object file or executable using the chosen backend.
- Link the generated code with Vortex's small runtime.
- Run the generated program and capture its exit status and output.

Use this program as the first end-to-end test:

```vortex
fn main() {
    let result = 2 + 3 * 4;
    print(result);
}
```

This milestone is complete when the program compiles, runs, and prints `14`.

## Milestone 7: Functions and control flow

- Generate calls to user-defined functions.
- Generate `if`, `else`, `while`, and `for` control flow.
- Generate `break`, `continue`, and early `return` behavior.
- Support local variables inside nested blocks.
- Test nested loops and nested conditionals.

This milestone is complete when functions and every v0.1 control-flow statement
work in compiled programs.

## Milestone 8: Strings, arrays, structs, and references

- Support basic UTF-8 string literals for printing and diagnostics.
- Support fixed-size one-dimensional and multidimensional arrays.
- Support array literals and repeat expressions such as `[0.0; 16]`.
- Support expression dimensions in array types and repeat-array expressions,
  including multidimensional forms such as `[0.0; 2 + 2, 8]`.
- Support struct creation and field access.
- Support shared and mutable references in function calls.
- Define the memory layout for arrays, structs, and references.
- Add tests that pass arrays into functions without copying them.

This milestone is complete when compiled programs can safely create, read,
change, and pass v0.1 data types.

## Milestone 9: Runtime safety

- Check array bounds unless the compiler can prove an access is safe.
- Detect integer division by zero.
- Detect integer overflow.
- Detect invalid numerical casts.
- Stop the program with a clear runtime error when a check fails.
- Test each failure and each successful boundary case.

This milestone is complete when the runtime behavior matches the language tour.

## Milestone 10: Matrix multiplication

- Write matrix multiplication using ordinary Vortex functions and loops.
- Use fixed-size multidimensional `f32` arrays.
- Pass inputs through shared references and the output through a mutable
  reference.
- Compare the result with a known correct answer.
- Test square and rectangular matrix shapes.
- Report shape or type mistakes during compilation.

This milestone is complete when Vortex compiles and correctly runs the matrix
multiplication example on the CPU without optimization.

## Milestone 11: v0.1 release gate

- Run every valid and invalid compiler test.
- Compile every v0.1 example from the documentation.
- Verify diagnostic source locations and runtime error messages.
- Document the compiler command, supported features, and known limitations.
- Confirm that a clean checkout can build and run the test suite.
- Mark the release as `v0.1.0` only after every item above passes.

## Definition of done

Vortex v0.1 is finished when it can:

1. Read, tokenize, parse, and type-check a documented Vortex program.
2. Reject invalid programs with useful source-based diagnostics.
3. Produce and link a CPU executable.
4. Run the documented scalar, control-flow, function, string, struct, array,
   and reference examples.
5. Detect the documented runtime safety errors.
6. Compile and correctly run naive matrix multiplication.

## After v0.1

These features are deliberately outside the first release:

- dead-code elimination and constant folding;
- loop transformations, tiling, and fusion;
- SIMD vectorization;
- multicore CPU execution;
- vectors, slices, and higher-level tensor types;
- kernels and GPU code generation;
- optimization diagnostics and cost models;
- auto-tuning;
- modules, packages, generics, traits, and advanced ownership.
