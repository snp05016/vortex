# Vortex Language and Compiler Cheat Sheet

Use this page when you remember the idea but forget the terminology.

This is a quick-reference companion to the formal
[Vortex v0.1 grammar](specification/grammar.md) and the implementation-focused
[parser design assignment](compiler/parser-design.md). Use the grammar to settle
exact syntax, this page to identify a concept quickly, and the parser design
when writing parser code.

## Contents

- [Fast navigation](#fast-navigation)
- [The shortest possible overview](#the-shortest-possible-overview)
- [The four main pieces of a program](#the-four-main-pieces-of-a-program)
  - [Expression](#expression)
  - [Statement](#statement)
  - [Declaration](#declaration)
  - [Type](#type)
- [Program structure](#program-structure)
- [Every Vortex v0.1 statement](#every-vortex-v01-statement)
- [Every Vortex v0.1 expression](#every-vortex-v01-expression)
- [Array dimensions: expressions with fixed-size rules](#array-dimensions-expressions-with-fixed-size-rules)
- [Operator terminology](#operator-terminology)
- [Vortex types](#vortex-types)
- [Lexer terminology](#lexer-terminology)
- [Grammar terminology](#grammar-terminology)
- [Parser terminology](#parser-terminology)
- [AST terminology](#ast-terminology)
- [Semantic and type-checking terminology](#semantic-and-type-checking-terminology)
- [C++ implementation terminology used by Vortex](#c-implementation-terminology-used-by-vortex)
- [One fully annotated example](#one-fully-annotated-example)
- [Fast way to classify something](#fast-way-to-classify-something)

## Fast navigation

| If you are asking... | Go to... |
| --- | --- |
| "Does this produce a value?" | [Expression](#expression) |
| "Does this perform an action?" | [Statement](#statement) |
| "Does this introduce a name?" | [Declaration](#declaration) |
| "Is this `i32`, an array, a reference, or a named type?" | [Type](#type) |
| "Which AST child nodes does this syntax need?" | [AST terminology](#ast-terminology) and the [parser design](compiler/parser-design.md) |
| "Is this invalid syntax or a type error?" | [Grammar terminology](#grammar-terminology) and [Semantic analysis](#semantic-analysis) |
| "How do array dimension expressions work?" | [Array dimensions](#array-dimensions-expressions-with-fixed-size-rules) |
| "Why is this child a `unique_ptr`?" | [`std::unique_ptr`](#stdunique_ptr) |

### Quick allowed/not-allowed lookup

| Area | You can write | You cannot write in v0.1 |
| --- | --- | --- |
| Top level | `fn`, `struct` declarations | top-level `let`, imports, enums, classes, type aliases |
| Variables | `let value = 1;`, `let mut value: i32 = 1;` | an uninitialized declaration such as `let value;` |
| Assignment | `value = 2;`, `point.x += 1.0;` | assignment inside another expression |
| Conditions | `if expression { ... }`, `while expression { ... }` | brace-free statement bodies |
| Loops | `for item in expression { ... }` | C-style `for (init; condition; step)` |
| Arrays | `[1, 2]`, `[0; 2 + 2]`, `[i32; 2 + 2]` | empty `[]`; dimensions that fail fixed-size semantic rules |
| Types | primitive, array, reference, or named type | tuple, enum, function, generic, or alias types |
| Functions | parameters, optional return annotation, block body | default arguments or overloaded syntax not defined by the grammar |

### Which compiler stage owns the question?

| Stage | Main question | Example rejection |
| --- | --- | --- |
| Lexer | "Do these characters form known tokens?" | unterminated string or unknown character |
| Parser | "Do these tokens have a valid structure?" | missing `)` or `;` |
| Name resolver | "What declaration does this name refer to?" | unknown `Point` or `count` |
| Type checker | "Do these values and operations have compatible types?" | `true + 1` |
| Semantic checker | "Does this valid-looking construct obey contextual rules?" | `break;` outside a loop |
| Constant checker | "Can this required compile-time expression be evaluated?" | runtime call used as a fixed array dimension |
| Backend | "How is this valid program represented and executed?" | unsupported lowering after earlier checks succeed |

## The shortest possible overview

```text
Source code
    -> Lexer
    -> Tokens
    -> Parser
    -> AST
    -> Semantic and type checking
    -> Intermediate representation
    -> Optimization
    -> Machine code
```

- The **lexer** recognizes words, numbers, operators, and punctuation.
- The **parser** determines how those tokens fit together.
- The **AST** stores the meaningful structure of the program.
- The **semantic checker** checks rules that grammar alone cannot check.
- The **type checker** determines and validates types.
- The **backend** eventually produces CPU or GPU code.

## The four main pieces of a program

| Term | Simple meaning | Vortex example |
| --- | --- | --- |
| Expression | Produces a value | `value + 10` |
| Statement | Performs an action | `return value;` |
| Declaration | Introduces a name or definition | `fn add(...)` |
| Type | Describes what kind of value something is | `i32`, `[f32; 4]` |

### Expression

An expression calculates or refers to a value:

```vortex
10
value
left + right
value < 10
calculate(value)
```

Expressions can contain other expressions. In `value + 10`, both `value` and
`10` are child expressions.

**Expressions can:** produce values, appear as operands, initialize variables,
be passed as arguments, and provide array dimensions.

**Expressions cannot:** introduce top-level functions or structs, act as
assignment statements, or replace a required statement terminator. An
expression may still be rejected later when its type or context is wrong.

#### Expression subtypes: quick handling map

| Subtype | How to spot it | What to do with it |
| --- | --- | --- |
| Literal | A value written directly: `42`, `true`, `"hi"` | Create a literal node and record its value and type. |
| Identifier | A name used as a value: `count` | Resolve the name in the symbol table, then use its declared type. |
| Grouping | An expression in parentheses: `(a + b)` | Parse the inside first; the parentheses usually need no AST node. |
| Unary | One prefix operator: `-value`, `!ready`, `&mut item` | Parse one operand, then check that the operator accepts its type. |
| Binary | Two operands with an operator: `a + b`, `a < b` | Respect precedence, parse left and right children, then type-check both. Binary kinds include arithmetic, equality, comparison, logical, bitwise, and shift. |
| Range | Two endpoints joined by `..` or `..=` | Check compatible endpoint types and whether the end is exclusive or inclusive. |
| Call or cast | A value followed by arguments: `add(a, b)` or `f32(count)` | Resolve the callee; then decide whether it is a function call or type conversion and check the arguments. |
| Index | Brackets after a value: `items[i]` | Check that the base is indexable and each index has an allowed integer type. |
| Field access | A dot and field name: `point.x` | Resolve the base type, find the field, and use the field's type. |
| Array or repeat array | `[1, 2, 3]` or `[0; 2 + 2]` | Parse element and dimension expressions; later check element compatibility and fixed-shape dimension rules. |
| Struct construction | A type name with fields: `Point { x: 1.0, y: 2.0 }` | Resolve the struct and check required, unknown, duplicate, and mistyped fields. |

`primary` and `postfix` are useful parser categories rather than extra semantic
subtypes. A primary starts an expression; postfix operations extend it with a
call, index, or field access.

### Statement

A statement tells the program to do something:

```vortex
let value = 10;
value += 1;
return value;
```

Most simple Vortex statements end with `;`. Control-flow statements and blocks
do not need a trailing semicolon.

**Statements can:** change mutable storage, control execution, return from a
function, declare a local variable, or evaluate an expression for its effects.

**Statements cannot:** be used where a value is required. In particular,
assignment and `if` are statements rather than value-producing expressions in
v0.1.

#### Statement subtypes: quick handling map

| Subtype | How to spot it | What to do with it |
| --- | --- | --- |
| Variable declaration | Starts with `let` | Create the local symbol, infer or check its type, and record mutability. |
| Assignment | An assignable target followed by `=`, `+=`, and similar operators | Validate the target, require it to be mutable, and type-check the new value. |
| Return | Starts with `return` | Check the optional value against the current function's return type. |
| Expression statement | An expression followed by `;` | Process the expression and discard its result. |
| If | Starts with `if` | Require a `bool` condition and process each branch in its own scope. |
| While | Starts with `while` | Require a `bool` condition and process the body as a loop scope. |
| For | Starts with `for name in` | Check the iterable expression, introduce the loop variable, and process the loop body. |
| Break | `break;` | Accept it only inside a loop and target the nearest loop exit. |
| Continue | `continue;` | Accept it only inside a loop and target the nearest next iteration. |
| Block | Statements inside `{ ... }` | Create a nested scope and process its statements in order. |

### Declaration

A declaration introduces something by name:

```vortex
fn calculate() {}

struct Point {
    x: f32,
    y: f32,
}
```

A local `let` is called a variable declaration, although the grammar also
treats it as a kind of statement because it appears inside a block.

**Declarations can:** introduce functions, structs, parameters, fields, and
local variables in their permitted scopes.

**Declarations cannot:** automatically supply a runtime value. For example,
`left: i32` declares a parameter name and type; the corresponding value arrives
from an argument when the function is called.

#### Declaration subtypes: quick handling map

| Subtype | Where it appears | What to do with it |
| --- | --- | --- |
| Function declaration | At the top level: `fn name(...)` | Register its name and full signature, then check its body in a new scope. |
| Struct declaration | At the top level: `struct Name { ... }` | Register the type name, then collect and validate its fields. |
| Variable declaration | Inside a block: `let` or `let mut` | Add the variable to the current scope after checking its initializer. |
| Parameter declaration | Inside a function's parameter list | Add the parameter name and declared type to the function scope. |
| Field declaration | Inside a struct declaration | Add the field name and type to that struct and reject duplicate names. |

For a quick compiler rule: top-level declarations go in the global symbol
table; parameters and local variables go in the current scope; fields belong
to their struct's definition.

### Type

A type describes the values an expression or variable may hold:

```vortex
i32
f32
String
[f32; 16]
&mut [f32; 16]
Point
```

#### Type subtypes: quick handling map

| Subtype | Examples | What to do with it |
| --- | --- | --- |
| Primitive | `void`, `bool`, `char`, `i32`, `u32`, `usize`, `f32`, `f64`, `String` | Recognize it directly. Numeric primitives split into integers (`i32`, `u32`, `usize`) and floating point (`f32`, `f64`); the others have their own operation rules. |
| Array | `[f32; 16]`, `[f32; 2 + 2, 8 / 2]` | Resolve the element type, preserve each dimension expression, then require fixed compile-time integer extents during semantic checking. |
| Reference | `&i32`, `&mut [f32; 16]` | Resolve the referred-to type and record whether the reference is mutable. |
| User-defined | `Point` | Look up the name and require it to resolve to a declared type such as a struct. |

Type annotations use these same four forms recursively. For example,
`&mut [f32; 16]` is a mutable reference whose referred-to type is an array,
whose element type is the primitive `f32`.

**Types can:** describe function parameters and results, variable annotations,
struct fields, referenced values, and array elements.

**Types cannot:** serve as ordinary runtime values by themselves. A type name
may use call syntax for a cast, such as `f32(count)`, but name and type checking
must identify that conversion after parsing.

## Program structure

### Program

The complete source file. A Vortex program contains function and struct
declarations.

### Function

A named piece of code that can receive parameters and return a value:

```vortex
fn add(left: i32, right: i32) -> i32 {
    return left + right;
}
```

### Function name

The identifier after `fn`. In the example above, it is `add`.

### Parameter

A named input accepted by a function:

```vortex
left: i32
```

`left` is the parameter name and `i32` is its type.

### Argument

An expression supplied when calling a function:

```vortex
add(10, value)
```

`10` and `value` are arguments. Parameters belong to the function definition;
arguments belong to a particular call.

### Return type

The type written after `->`:

```vortex
fn size() -> usize
```

When the return type is omitted, Vortex treats it as `void`.

### Function body

The block containing the function's statements.

### Entry point

The function where an executable starts. In Vortex v0.1, it is `main` with no
parameters and a `void` return type.

### Block

A sequence of statements surrounded by braces:

```vortex
{
    let value = 10;
    return;
}
```

### Scope

The region where a declared name is visible. A function body and each nested
block create scopes.

### Struct

A user-defined type that groups named fields:

```vortex
struct Point {
    x: f32,
    y: f32,
}
```

### Field

A named value stored inside a struct. `x` and `y` are fields of `Point`.

## Every Vortex v0.1 statement

### Statement validity at a glance

| Form | Valid example | Closest invalid form | Why invalid |
| --- | --- | --- | --- |
| Variable declaration | `let value = 10;` | `let value;` | initializer required |
| Typed variable | `let value: i32 = 10;` | `let value i32 = 10;` | missing `:` |
| Assignment | `value += 1;` | `(value += 1)` | assignment is not an expression |
| Return | `return value;` | `return value` | missing `;` |
| Expression statement | `calculate();` | `calculate()` | missing `;` |
| If | `if ready { run(); }` | `if ready run();` | block required |
| While | `while ready { run(); }` | `while (ready) run();` | block required |
| For | `for i in 0..10 { run(); }` | `for (i = 0; i < 10; i += 1) {}` | C-style form unsupported |
| Break | `break;` | `break value;` | no break value in v0.1 |
| Continue | `continue;` | `continue value;` | no continue value in v0.1 |

An entry can be syntactically valid and still fail later. For example,
`break;` matches the grammar anywhere a statement is allowed, but semantic
analysis rejects it outside a loop.

### Variable declaration

Creates a local variable:

```vortex
let count = 10;
let mut total: i32 = 0;
```

- `let` begins the declaration.
- `mut` means the variable may be changed.
- The type annotation is optional when it can be inferred.
- The initializer is the expression after `=`.

### Assignment statement

Changes an existing mutable location:

```vortex
count = 10;
count += 1;
point.x = 4.0;
values[index] = 20;
```

Assignment is a statement in Vortex v0.1, not an expression.

### Assignment target

The location being changed on the left side of an assignment:

```vortex
value
point.x
matrix[row, column]
```

### Return statement

Ends the current function and optionally provides its result:

```vortex
return result;
return;
```

### Expression statement

An expression used as a complete statement:

```vortex
print(value);
calculate();
```

The returned value, if any, is ignored.

### If statement

Runs a block only when its condition is true:

```vortex
if value < 10 {
    print(value);
} else {
    print(10);
}
```

`if` is a statement, not a value-producing expression, in v0.1.

### While statement

Repeats a block while a condition remains true:

```vortex
while index < length {
    index += 1;
}
```

### For statement

Iterates over the values produced by an expression such as a range:

```vortex
for index in 0..10 {
    print(index);
}
```

### Break statement

Immediately exits the nearest loop:

```vortex
break;
```

### Continue statement

Skips the rest of the current loop iteration and begins the next one:

```vortex
continue;
```

### Block statement

A nested block used where a statement is allowed:

```vortex
{
    let temporary = 10;
}
```

## Every Vortex v0.1 expression

### Expression validity at a glance

| Form | Valid example | Not allowed or checked later |
| --- | --- | --- |
| Literal | `42`, `3.14`, `true`, `'V'`, `"text"` | literal range and final numeric type are checked later |
| Grouping | `(left + right)` | empty `()` is unsupported |
| Unary | `-value`, `!ready`, `&mut item` | operand type is checked later |
| Binary | `left + right`, `left < right` | compatible operand types are checked later |
| Range | `0..10`, `0..=10` | chained ranges are not part of the v0.1 grammar |
| Call/cast | `add(a, b)`, `f32(count)` | callee resolution and argument types are checked later |
| Index | `values[i]`, `matrix[row, column]` | index type and bounds are checked later |
| Field access | `point.x` | field existence is checked later |
| Array | `[1, 2, 3]` | empty `[]` is unsupported |
| Repeat array | `[0; 2 + 2]` | dimensions must pass fixed-size rules later |
| Struct construction | `Point { x: 1.0 }` | unknown, missing, duplicate, or mistyped fields are checked later |

### Literal expression

A value written directly in the source:

```vortex
42
3.14
true
'V'
"hello"
```

### Identifier expression

A programmer-defined name used as a value:

```vortex
value
matrix
calculate
```

### Grouping expression

Parentheses that force an expression to be evaluated as one unit:

```vortex
(left + right) * scale
```

The parentheses usually do not need their own AST node; the tree structure
already records the grouping.

### Unary expression

An operator applied to one expression:

```vortex
-value
+value
!enabled
~bits
&value
&mut value
```

`+` and `-` mean numeric sign, `!` means logical not, `~` means bitwise not,
and `&` creates a reference.

### Binary expression

An operator applied to a left expression and a right expression:

```vortex
left + right
value < 10
ready && enabled
```

A binary-expression AST node owns two expression children.

### Arithmetic expression

Numeric calculation using:

```text
+  -  *  /  %
```

### Equality expression

Tests whether two values are equal or unequal:

```vortex
left == right
left != right
```

Its result is a `bool`.

### Comparison expression

Compares ordered values:

```vortex
left < right
left <= right
left > right
left >= right
```

Its result is a `bool`.

### Logical expression

Combines boolean values:

```vortex
ready && enabled
ready || forced
```

### Bitwise expression

Operates on the individual bits of integer values:

```vortex
left & right
left | right
left ^ right
~value
```

### Shift expression

Moves an integer's bits left or right:

```vortex
value << 2
value >> 1
```

### Range expression

Describes a sequence between two endpoints:

```vortex
0..10
0..=10
```

`..` excludes the ending value. `..=` includes it.

### Call expression

Calls a function using zero or more argument expressions:

```vortex
print(value)
add(left, right)
```

### Cast expression

Vortex uses the same syntax as a call for explicit conversions:

```vortex
f32(count)
```

The parser initially treats this as a call. Type checking later determines that
`f32` names a type and that the call represents a conversion.

### Index expression

Reads an element from an array or multidimensional value:

```vortex
values[index]
matrix[row, column]
```

### Field-access expression

Reads a named field from a struct value:

```vortex
point.x
```

### Postfix expression

An expression followed by calls, indexing, or field access:

```vortex
object.field[index]
factory().value
```

It is called postfix because the extra operation appears after the initial
expression.

### Array expression

Creates an array from element expressions:

```vortex
[1, 2, 3]
```

### Repeat-array expression

Creates an array shape by repeating a value:

```vortex
[0.0; 2 + 2]
[0.0; 2 + 2, 8 / 2]
```

The repeated value and every dimension are expressions. They have different
roles: the value supplies array elements, while the dimensions describe the
shape.

### Struct expression

Constructs a struct value:

```vortex
Point {
    x: 1.0,
    y: 2.0,
}
```

### Primary expression

The smallest starting form of an expression: a literal, identifier, array,
struct value, or parenthesized expression.

## Array dimensions: expressions with fixed-size rules

Array types and repeat-array values preserve dimensions as expression AST
nodes:

```vortex
let values: [i32; 2 + 2] = [0; 2 + 2];
let matrix: [f32; 2 * 2, 8 / 2] = [0.0; 2 * 2, 8 / 2];
```

The parser does not immediately reduce `2 + 2` to `4`. It builds the normal
binary-expression tree so the source structure and location remain available:

```text
ArrayType or RepeatArrayExpr
└── dimension: BinaryExpr(Add)
    ├── Literal(2)
    └── Literal(2)
```

### What you can write syntactically

Because a dimension is an expression, the grammar can represent:

```vortex
[i32; 4]
[i32; 2 + 2]
[i32; (2 * 4)]
[i32; rows, columns]
[0; size_for_input()]
```

Parsing these forms does not promise that they form valid fixed-size array
types. Parsing only establishes their structure.

### What fixed-size v0.1 arrays require semantically

Each dimension must evaluate to an integer extent that is:

- known at compile time;
- representable by the implementation's supported array-size representation.

The grammar does not currently settle whether a zero-length extent is legal.
The parser must preserve `0` as an expression; the semantic specification must
make and enforce the zero-extent policy.

| Dimension | Syntax | Fixed-size semantic result |
| --- | --- | --- |
| `4` | valid | valid |
| `2 + 2` | valid | valid after constant evaluation |
| `(8 / 2)` | valid | valid if integer evaluation produces `4` |
| `3.5` | valid expression syntax | invalid dimension because it is not an integer |
| `0` | valid expression syntax | depends on the language's zero-extent policy |
| `runtime_size()` | valid expression syntax | invalid unless it has a defined compile-time value |

This distinction explains why dimensions are stored as
`std::vector<std::unique_ptr<Expr>>` instead of immediately as a vector of
integers.

### Array type versus repeat-array value

```vortex
[i32; 2 + 2] // type: element type is i32, dimension is 2 + 2
[0; 2 + 2]   // value: repeated element is 0, dimension is 2 + 2
```

The surrounding parser context distinguishes them. A type parser expects an
element type before `;`; an expression parser expects a repeated value.

### Dimension expression versus index expression

```vortex
let values: [i32; 2 + 2] = [0; 4];
let item = values[index + 1];
```

- `2 + 2` describes the array's fixed shape and must satisfy compile-time
  dimension rules.
- `index + 1` chooses an element and may be evaluated at runtime.

Do not store these as the same AST role even though both are expressions. The
array type or repeat-array node owns dimensions; an index-expression node owns
indices.

## Operator terminology

### Operand

A value that an operator acts on. In `left + right`, `left` and `right` are
operands.

### Operator

A symbol that performs an operation, such as `+`, `<`, or `&&`.

### Unary operator

An operator with one operand, such as `!ready`.

### Binary operator

An operator with two operands, such as `left + right`.

### Prefix operator

An operator written before its operand, such as `-value`.

### Postfix operation

An operation written after an expression, such as a call, index, or field
access.

### Precedence

The rule deciding which operator binds more tightly:

```vortex
1 + 2 * 3
```

Multiplication has higher precedence, so the expression means `1 + (2 * 3)`.

### Associativity

The direction used when operators have the same precedence:

```vortex
10 - 5 - 2
```

Left associativity makes this `(10 - 5) - 2`.

## Vortex types

### Primitive type

A type built directly into the language:

```text
void bool char i32 u32 usize f32 f64 String
```

### Literal value versus primitive type

These answer different questions:

| Concept | Question answered | Example from `let count: u32 = 42;` |
| --- | --- | --- |
| Literal value | What data was written? | magnitude `42` |
| Literal kind | What source-literal category was written? | integer literal |
| Primitive type | How is the value interpreted and validated? | `u32` |

One integer-literal representation can initially store the nonnegative
magnitude for `42` whether later checking selects `i32`, `u32`, or `usize`.
For `-42`, the literal stores `42` and a unary-expression parent stores the
negation operation.

Do not add `void` to `LiteralValue`: `void` is a type but has no literal value.
Do not store `i32` as the parameter's literal value in `left: i32`: it is the
parameter's declared type, and the runtime value comes from a call argument.

### Integer type

A whole-number type. Vortex v0.1 includes `i32`, `u32`, and `usize`.

### Signed integer

An integer type that can represent negative and positive values. `i32` is
signed.

### Unsigned integer

An integer type that represents zero and positive values. `u32` and `usize`
are unsigned.

### Floating-point type

A type for numbers with fractional parts. Vortex provides `f32` and `f64`.

### Boolean type

`bool`, whose values are `true` and `false`.

### Character type

`char`, representing one character.

### String type

`String`, used for text.

### Void type

`void` means a function returns no value. It is a type, not a literal value.

### Array type

A fixed-size collection whose shape is part of its type:

```vortex
[f32; 16]
[f32; 2 + 2, 8 / 2]
```

Each dimension is parsed as an expression. Fixed-size semantic checking later
requires a compile-time integer result that is valid as an array extent. See
[Array dimensions: expressions with fixed-size rules](#array-dimensions-expressions-with-fixed-size-rules).

### Reference type

A type that refers to another value without owning a new copy:

```vortex
&i32
&mut [f32; 16]
```

### User-defined type

A type declared by the programmer, such as `Point`.

### Type annotation

An explicitly written type:

```vortex
let count: i32 = 10;
```

### Type inference

The compiler determines a type from context:

```vortex
let count = 10;
```

### Static typing

Types are checked before the program runs.

## Lexer terminology

### Source text

The original characters contained in a Vortex file.

### Character

One unit examined by the lexer, such as `l`, `+`, or `7`.

### Lexeme

The exact source substring belonging to a token. The lexeme of `KW_LET` is
`let`.

### Token

A categorized piece of source text containing a token kind and source span.

### Token kind

The category assigned to a token, such as `KW_LET`, `IDENTIFIER`, or
`LIT_INT`.

### Keyword

A reserved word with a special language meaning, such as `fn`, `let`, or
`return`. It cannot be used as an identifier.

### Identifier

A programmer-chosen name, such as `value`, `Point`, or `_index2`.

### Literal

A value written directly in source code, such as `10`, `3.14`, `true`, `'V'`,
or `"hello"`.

### Punctuation

Symbols that organize syntax, such as `(`, `)`, `{`, `}`, `[`, `]`, `,`, `:`,
and `;`.

### Trivia

Source text that generally does not reach the parser, such as whitespace and
comments.

### Source location or source span

The starting position and length of a token or AST node in the original source.

### Lookahead

Examining upcoming characters or tokens without consuming them yet.

### End-of-file token

`EOF_TOKEN`, which tells the parser that no source tokens remain.

### Invalid token

A token produced when the lexer cannot recognize valid syntax.

## Grammar terminology

### Syntax

The rules describing how valid code is written.

### Semantics

The meaning and behavior of syntactically valid code.

For example, `break;` may be valid syntax but semantically invalid outside a
loop.

### Grammar

The formal collection of syntax rules for the language.

### EBNF

Extended Backus-Naur Form, the notation used by Vortex's grammar document.

### Grammar rule or production

A named syntax definition:

```ebnf
return_statement ::= "return", [ expression ], ";" ;
```

### Terminal

A concrete token that appears in source, such as `"return"` or `";"`.

### Nonterminal

A named grammar concept expanded using other rules, such as `expression` or
`statement`.

### Optional item

EBNF square brackets mean an item may appear zero or one time:

```ebnf
[ "mut" ]
```

### Repetition

EBNF braces mean an item may repeat zero or more times:

```ebnf
{ statement }
```

### Alternative

`|` means choose one possible form:

```ebnf
"true" | "false"
```

### Ambiguity

A grammar is ambiguous when the same token sequence can be interpreted in more
than one valid way.

## Parser terminology

### Parser

The compiler stage that consumes tokens, validates their structure, and builds
an AST.

### Current token

The token the parser is presently examining.

### Token lookahead

One or more upcoming tokens used to decide which grammar rule applies.

### Consume or advance

Accept the current token and move to the next one.

### `check`

Ask whether the current token has a particular kind without consuming it.

### `match`

Consume the current token only if it has the requested kind.

### `expect`

Require a particular token kind. If it is absent, report a syntax error.

### Recursive-descent parser

A hand-written parser where functions correspond to grammar rules. For example,
`parse_function()` implements the `function` rule.

### LL(1)

A left-to-right predictive parsing style that normally makes decisions using
one lookahead token.

### Pratt parser

An expression-parsing technique that handles operator precedence using binding
powers. Vortex can use recursive descent for program structure and Pratt parsing
for expressions.

### Precedence climbing

Another expression-parsing technique that groups operators according to their
precedence.

### Parse error or syntax error

An error caused by tokens appearing in an invalid structure.

### Error recovery

How the parser moves past an error so it can report more problems. Common
synchronization points are `;`, `}`, `fn`, and `struct`.

## AST terminology

### AST

Abstract Syntax Tree: a tree containing the meaningful structure of the
program. It discards unnecessary syntax such as grouping parentheses and
semicolons when their meaning is already represented by the tree.

### AST node

One object in the AST, such as an integer literal, binary expression, variable
declaration, function, or program.

### Root node

The top node representing the complete program.

### Parent and child

A parent node contains other nodes. In `left + right`, the binary expression is
the parent and the operand expressions are its children.

### Leaf node

A node with no child nodes, such as an integer literal or identifier expression.

### Expression node

An AST node that represents a value-producing expression.

### Statement node

An AST node that represents an action or control-flow statement.

### Declaration node

An AST node that introduces a variable, function, struct, parameter, or field.

### Binary-expression node

An expression node containing a left expression, operator, and right
expression.

### Unary-expression node

An expression node containing one operator and one operand expression.

### AST operator enum

An enum describing semantic operations, such as `Add` or `LessThan`. This keeps
AST meaning separate from lexer token categories such as `OP_PLUS`.

### AST visitor

An operation that walks AST nodes to inspect or process them. Pretty printers,
type checkers, and code generators can use visitors.

## Semantic and type-checking terminology

### Semantic analysis

Checks meaning that cannot be determined purely from grammar.

### Type checking

Verifies that operations receive compatible types and determines inferred
types.

### Symbol

A named program entity, such as a variable, function, struct, parameter, or
field.

### Symbol table

A structure mapping names to information about their declarations.

### Name resolution

Determines which declaration an identifier refers to.

### Mutability

Whether a value may be changed after declaration.

### Lvalue or assignable location

An expression or target that identifies storage which may be assigned to, such
as `value`, `point.x`, or `array[index]`.

### Rvalue

A value used for computation, such as the `10 + 5` on the right side of an
assignment.

### Constant

A value that cannot change after it is established.

### Compile time

The period when the compiler analyzes and translates the program.

### Runtime

The period when the compiled program executes.

### Diagnostic

A compiler message describing an error, warning, note, or suggestion.

## C++ implementation terminology used by Vortex

### Base type

A general parent type, such as `Node` or `Expression`.

### Derived type

A more specific child type, such as `BinaryExpression : Expression`.

### Inheritance

The relationship saying one type is a specialized form of another.

### Polymorphism

Handling different derived AST nodes through a shared base type.

### Virtual destructor

A base-class destructor that ensures deleting a derived object through a base
pointer destroys the entire object correctly.

### `std::unique_ptr`

An owning smart pointer. In the AST, it means one node exclusively owns a child
node and C++ deletes the child automatically.

### Raw pointer

A memory address such as `Expression*`. It does not communicate ownership and
does not automatically delete anything.

### Ownership

The responsibility for keeping an object alive and eventually destroying it.

### `struct` versus `class`

They have almost the same abilities in C++. A `struct` is public by default; a
`class` is private by default. AST nodes are commonly structs because they are
mostly data. Lexer and parser objects are commonly classes because they protect
internal state.

### Public

Part of a type's interface that outside code may use.

### Private

Internal state or helpers that only the type itself should control.

### Enum class

A fixed set of named choices with scoped names, such as `TokenKind::KW_LET` or
`BinaryOperation::Add`.

### Constructor

A special C++ function that initializes an object when it is created:

```cpp
PrimitiveType(SourceLocation location, PrimitiveTypeKind kind)
    : Type(location), primitive_type_(kind) {}
```

Reading it from left to right:

- `PrimitiveType(...)` lists the information required to create the node;
- `Type(location)` initializes the base `Type` part;
- `primitive_type_(kind)` stores the concrete primitive kind; and
- `{}` is the constructor body, which is empty because initialization is
  already complete.

A concrete AST constructor should normally accept every field required to make
the node valid. Base-category constructors such as `Expr(location)` may need
only the shared source location.

### Constructor initializer list

The portion after `:` that initializes bases and members before the constructor
body runs:

```cpp
: Expr(location), left_(std::move(left)), right_(std::move(right))
```

This is initialization, not assignment after construction.

### `std::move`

Marks an object as available for move construction or move assignment. AST
constructors use it to transfer ownership of strings, vectors, and
`unique_ptr` children into the new node:

```cpp
operand_(std::move(operand))
```

After ownership is moved from a `unique_ptr`, the old pointer no longer owns
the child and should not be used as though it does.

### `std::variant`

A type-safe container that holds exactly one value from a fixed list of C++
types. A general literal node can use it because integer, floating, string,
character, and boolean values need different C++ storage:

```cpp
using LiteralValue = std::variant<
    std::uint64_t,
    double,
    std::string,
    char,
    bool>;
```

The variant holds one alternative at a time. It does not mean every Vortex
primitive type needs a separate alternative.

### `std::vector`

An ordered growable C++ collection. AST nodes use vectors for source constructs
that may contain several children, such as function parameters, block
statements, call arguments, array elements, and array dimension expressions.

Use values for fixed child-record kinds that exist only inside their parent:

```cpp
std::vector<ParamDecl> parameters;
```

`ParamDecl` belongs directly to `FunctionDecl`, so a pointer is unnecessary
unless parameters later need polymorphism or stable addresses. Use owning
pointers when the vector contains different derived node types:

```cpp
std::vector<std::unique_ptr<Expr>> dimensions;
```

This means the collection owns zero or more polymorphic expression nodes in
source order.

## One fully annotated example

```vortex
fn calculate(value: i32) -> bool {
    let limit: i32 = 10;
    return value < limit;
}
```

- The complete text is a **program**.
- `fn calculate...` is a **function declaration**.
- `calculate` is an **identifier** and the function name.
- `value: i32` is a **parameter** with a **type annotation**.
- `bool` is the **return type**.
- The braces contain the function's **block** or body.
- `let limit: i32 = 10;` is a **variable-declaration statement**.
- `10` is an **integer literal expression** and the initializer.
- `return value < limit;` is a **return statement**.
- `value < limit` is a **binary comparison expression**.
- `value` and `limit` are **identifier expressions** and operands.
- `<` is the comparison operator.
- The comparison expression produces a `bool` value.

## Fast way to classify something

Ask these questions in order:

1. Does it directly produce or refer to a value? It is probably an expression.
2. Does it tell the program to perform an action? It is probably a statement.
3. Does it introduce a name? It is probably a declaration.
4. Does it describe what values are allowed? It is a type.
5. Is it a lexer category such as a word, number, or symbol? It is a token.
6. Is it an object in the parsed tree? It is an AST node.
