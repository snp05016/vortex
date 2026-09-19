# Vortex Language and Compiler Cheat Sheet

Use this page when you remember the idea but forget the terminology.

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

### Statement

A statement tells the program to do something:

```vortex
let value = 10;
value += 1;
return value;
```

Most simple Vortex statements end with `;`. Control-flow statements and blocks
do not need a trailing semicolon.

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
[0.0; 16]
[0.0; 4, 4]
```

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
[f32; 4, 4]
```

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
