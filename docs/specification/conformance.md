# 1. Conformance and terminology

This chapter defines how to interpret the Vortex specification and what it
means for a source program or implementation to conform to v0.1.

## 1.1 Document authority

The following order resolves documentation conflicts:

1. This language specification defines required language behavior.
2. The [formal grammar](grammar.md) defines exact v0.1 source syntax.
3. The language guide explains the specification through examples.
4. Compiler design documents describe one implementation strategy.
5. Current compiler behavior is evidence of implementation status, not a rule.

An inconsistency between the implementation and the specification is either an
implementation defect or an explicit proposal to revise the specification. It
must not be resolved silently.

## 1.2 Programs

A **source file** is a sequence of source characters provided to the compiler.
A **program** is the collection of top-level declarations parsed from that
source file.

A program is **well-formed** when it satisfies all applicable lexical,
syntactic, name, type, semantic, and constant-evaluation rules in this
specification.

An executable Vortex program additionally must contain exactly one function
named `main` with no parameters and a `void` return type. An empty source file
is grammatically valid but does not satisfy the executable entry-point rule.

## 1.3 Implementation conformance

A conforming Vortex v0.1 implementation must:

- accept every well-formed v0.1 program within its documented implementation
  limits;
- reject every ill-formed program rather than silently changing its meaning;
- preserve the specified observable behavior of accepted programs;
- issue a diagnostic with a source location for each detected source error;
- avoid accepting planned syntax as though it were standardized v0.1 syntax;
- document any implementation limit that is narrower than the language design.

A compiler under development may be incomplete. It should report an explicit
"not implemented" diagnostic when it recognizes specified syntax that it
cannot yet lower. Crashing or silently omitting the construct is not conforming
behavior.

## 1.4 Static and dynamic rules

A **static rule** can be checked before execution. Syntax, name visibility,
types, mutability, and fixed array extent requirements are static rules.

A **dynamic rule** concerns values only known during execution. Examples
include an index that is out of bounds for a particular runtime value or
integer division by zero when the divisor is not constant.

An implementation may diagnose a dynamic failure during compilation when it
can prove the failure. It must otherwise preserve the required runtime check.

## 1.5 Undefined behavior

Vortex v0.1 does not intentionally expose undefined behavior to a well-formed
program. Operations with invalid dynamic conditions must either be rejected
statically or fail through the documented runtime diagnostic mechanism.

This rule does not promise that the current compiler is free from bugs. It
defines the language goal and the behavior later implementation work must
preserve.

## 1.6 Implementation-defined behavior

Implementation-defined behavior is permitted only where this specification
explicitly allows it. The implementation must document the selected behavior
and apply it consistently.

Host architecture, target architecture, object-file format, optimization
strategy, register allocation, and internal AST representation are
implementation choices. They must not change the specified meaning of a
program.

## 1.7 Source locations

Every token and AST node required by the implementation must retain enough
source information to identify its originating source span. Diagnostics should
report at least the source location and a plain description of the violated
rule.

For a compound construct, the stored span should cover the complete construct.
For example, the location of `left + right` covers both operands and the
operator, while the child nodes retain their own narrower spans.

## 1.8 Specification examples

The following annotations are used throughout this site:

| Label | Meaning |
| --- | --- |
| Valid | Required to be accepted when all referenced names and values satisfy later rules |
| Invalid syntax | Rejected during lexing or parsing |
| Semantic error | Parses successfully but must be rejected by a later static phase |
| Runtime error | Valid source whose evaluated values trigger a required dynamic check |
| Planned | Design direction that is not part of v0.1 |

## 1.9 Change discipline

A language change is complete only when the grammar, affected specification
chapters, valid and invalid examples, diagnostics, parser or AST design, and
tests agree. A code-only change does not revise the language specification.
