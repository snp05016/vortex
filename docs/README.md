# Vortex Documentation

This directory is the source of truth for the Vortex language and compiler.
Use this page as the starting point instead of searching through files at
random.

## Quick navigation

| If you want to... | Start here |
| --- | --- |
| Learn Vortex from the beginning | [Language tour](language-tour/README.md) |
| Look up one term or AST concept quickly | [Language and compiler cheat sheet](language-and-compiler-cheatsheet.md) |
| Check whether exact syntax is valid | [Formal v0.1 grammar](specification/grammar.md) |
| Implement or debug the parser | [Parser design](compiler/parser-design.md) |
| Learn the C++ concepts behind AST nodes | [AST learning guide](compiler/ast-guide.md) |
| Understand why Vortex exists | [Language philosophy](philosophy.md) |
| See what must be built and in what order | [v0.1 roadmap](roadmap.md) |

## Recommended reading paths

### I am learning the language

1. Read the [language-tour index](language-tour/README.md).
2. Complete chapters 1 through 4 in order.
3. Use chapters 8 through 10 as the detailed expression, statement, and
   declaration references.
4. Use the [cheat sheet](language-and-compiler-cheatsheet.md) when you remember
   a concept but not its name.
5. Check the [formal grammar](specification/grammar.md) when exact punctuation
   or nesting matters.

### I am implementing the compiler

1. Read the [formal grammar](specification/grammar.md).
2. Read the [parser design](compiler/parser-design.md).
3. If AST ownership or C++ inheritance is unfamiliar, work through the
   [AST learning guide](compiler/ast-guide.md).
4. Keep the [cheat sheet](language-and-compiler-cheatsheet.md) open for AST and
   semantic terminology.
5. Use the [roadmap](roadmap.md) as the implementation and verification order.
6. Confirm behavior against the relevant language-tour chapter before adding a
   parser, type-checker, or code-generation rule.

### I am deciding whether to add a feature

1. Check the [language philosophy](philosophy.md).
2. Check whether the feature is already planned in
   [types planned for later](language-tour/05-types-planned-for-later.md).
3. Check the [roadmap](roadmap.md) to determine whether it belongs in v0.1.
4. Update the grammar, language tour, cheat sheet, parser design, tests, and
   implementation together if the decision changes the language.

## How to read these documents

Each language feature should answer the same questions:

1. **Purpose:** Why does this feature exist?
2. **Syntax:** What exact source form does it use?
3. **Allowed:** Which forms are accepted in v0.1?
4. **Not allowed:** Which similar-looking forms are rejected?
5. **Examples:** What do valid and invalid programs look like?
6. **Compiler work:** What must the lexer, parser, semantic checker, type
   checker, and backend do?
7. **Verification:** How can an implementation prove the rule works?

The documents distinguish three different questions:

| Question | Compiler stage |
| --- | --- |
| Can these tokens form this construct? | Parser and grammar |
| Does the construct make sense here? | Semantic analysis |
| Are its value and operand types valid? | Type checking |

For example, an array dimension is parsed as an expression:

```vortex
[f32; rows + 1]
```

The syntax is valid, but v0.1 fixed-size arrays require each dimension to be a
compile-time integer representable as a supported array extent. If `rows` is a
runtime variable, semantic or type checking rejects the type even though
parsing succeeded. Whether a zero-length extent is legal remains an explicit
semantic-design decision and is not decided by the parser.

## v0.1 status vocabulary

Documentation uses these labels when implementation status matters:

- **Specified:** the behavior is part of the v0.1 language design.
- **Implemented:** the current compiler handles the behavior.
- **Planned:** the behavior is intended but not yet implemented.
- **Out of scope:** the behavior is deliberately excluded from v0.1.

Do not assume that a specified feature is already implemented. The language
tour and grammar define the target behavior; the roadmap and tests establish
implementation progress.

## Documentation maintenance checklist

When a language rule changes, update all affected places:

- [ ] Formal grammar production.
- [ ] Relevant language-tour chapter.
- [ ] Allowed and not-allowed examples.
- [ ] Language/compiler cheat sheet.
- [ ] Parser and AST design notes.
- [ ] Roadmap or planned-feature boundary.
- [ ] Valid and invalid compiler tests once that test layer exists.

Keeping these synchronized prevents the parser, AST, and written language from
quietly becoming three different designs.
