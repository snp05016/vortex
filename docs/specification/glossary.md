# 12. Glossary

This glossary fixes the meaning of recurring terms in the Vortex
specification. Language-specific definitions link to the chapter containing the
full rule.

## Terms

abstract syntax tree (AST)
: A tree representation that preserves the meaningful structure of parsed
  source without retaining every punctuation token as its own node. See the
  [AST learning guide](../compiler/ast-guide.md).

addressable
: Describes an expression that identifies existing storage and can therefore
  be referenced. Variables, fields, and indexed array elements are addressable
  in supported contexts; temporary calculations are not.

aggregate type
: A type composed of subordinate values. Vortex v0.1 aggregate types are fixed
  arrays and structs.

argument
: An expression supplied by a call for one function parameter.

array
: A homogeneous value with a fixed element type and compile-time shape. See
  [Arrays and shapes](arrays.md).

associativity
: The rule that determines grouping when operators at the same precedence
  level repeat. Most binary Vortex operators group left; prefix unary
  operators group right.

binding
: The association between a name and a declaration or storage location.

block
: A brace-delimited ordered sequence of statements that creates a local scope.

compile time
: The period in which the compiler translates and validates a program before
  that program executes.

constant evaluation
: Evaluation performed by the compiler for an expression whose value is
  required before code generation, including fixed array dimensions.

declaration
: A construct that introduces a name or definition. Top-level Vortex
  declarations are functions and structs; locals are declaration statements.

dimension
: One compile-time integer extent in an array shape.

dynamic rule
: A requirement involving values that may only be known during execution.

element type
: The single type shared by every element of an array.

expression
: A construct that computes, constructs, or refers to a value. See
  [Expressions](expressions.md).

field
: A named component declared by a struct.

grammar
: The formal rules describing which token sequences form valid source
  constructs. See the [formal grammar](grammar.md).

identifier
: A programmer-defined, case-sensitive name with the spelling rules in
  [Lexical structure](lexical-structure.md).

ill-formed
: Describes source that violates at least one required language rule and must
  be rejected.

implementation-defined
: Describes behavior for which the specification permits a choice and requires
  the implementation to document that choice.

initializer
: The expression that supplies the starting value of a local binding or one
  named field value in struct construction.

lexing
: Converting source characters into tokens.

lifetime
: The interval during which a value or storage location exists and may be
  accessed safely.

lowering
: Translating a validated representation into a lower-level intermediate or
  target representation.

mutable
: Describes storage whose value may be changed through a valid assignment or
  mutable reference.

name resolution
: Connecting each name use to one visible declaration and diagnosing unknown,
  duplicate, or out-of-scope names.

named type
: A type written as an identifier and preserved until name resolution selects
  its declaration.

normative
: Required for language conformance. Informative explanations help readers but
  do not create additional requirements.

parameter
: A named, typed input declared in a function signature.

parsing
: Checking the grammatical structure of a token stream and building the AST.

precedence
: The rule that determines which operator groups more tightly when parentheses
  do not state the grouping explicitly.

primitive type
: One built-in type named directly by the language, such as `i32`, `f32`, or
  `bool`.

rank
: The number of dimensions in an array type.

reference
: A safe connection to existing storage, written `&T` or `&mut T`. See
  [References and mutability](references.md).

run time
: The period in which a compiled Vortex program executes.

scope
: The source region in which a declaration's name is visible.

semantic analysis
: Static checks that require context beyond grammar, including mutability,
  control-flow placement, and entry-point validation.

shape
: The ordered list of fixed extents for an array. Shape is part of an array
  type in v0.1.

source span
: A start position and length identifying the source characters represented by
  a token, AST node, or diagnostic.

statement
: A construct that performs an action or changes control flow. See
  [Statements](statements.md).

static rule
: A rule that a conforming implementation can check before program execution.

struct
: A nominal, named value type containing ordered named fields.

token
: One lexical unit with a kind, source spelling, and source span.

type checking
: Determining expression types and validating that operations, calls,
  assignments, and returns use compatible types.

undefined behavior
: Behavior for which a language imposes no requirements. Vortex v0.1 does not
  intentionally expose undefined behavior to well-formed safe programs.

value
: The result of evaluating an expression. A value has one Vortex type.

well-formed
: Describes a program that satisfies every applicable v0.1 rule.

## Reference sources

The language-specific rules in this glossary are defined by the linked Vortex
chapters. The following external references provide background terminology but
do not override Vortex rules:

- [LLVM Language Reference](https://llvm.org/docs/LangRef.html) for
  intermediate-representation and lowering terminology.
- [IEEE 754 overview](https://standards.ieee.org/ieee/754/6210/) for the
  floating-point model referenced by v0.1.
- [C++ object model reference](https://en.cppreference.com/w/cpp/language/object)
  for concepts encountered in the compiler implementation.
