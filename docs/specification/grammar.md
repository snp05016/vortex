# Vortex v0.1 Grammar Reference

This is the formal source-language reference for Vortex v0.1. It combines the
EBNF grammar with implementation notes, valid and invalid examples, semantic
boundaries, and an implementation checklist.

Use it while building the lexer, parser, AST, semantic checker, tests, or error
messages. If a tutorial example conflicts with this document, this document
defines the v0.1 syntax.

## Contents

- [How to read the grammar](#how-to-read-the-grammar)
- [Parsing versus semantic checking](#parsing-versus-semantic-checking)
- [Program structure](#program-structure)
- [Functions and parameters](#functions-and-parameters)
- [Struct definitions and fields](#struct-definitions-and-fields)
- [Blocks and statements](#blocks-and-statements)
- [Expressions and precedence](#expressions-and-precedence)
- [Array expressions](#array-expressions)
- [Struct expressions](#struct-expressions)
- [Types](#types)
- [Literals](#literals)
- [Identifiers and keywords](#identifiers-and-keywords)
- [Implementation checklist](#implementation-checklist)

## How to read the grammar

The grammar uses Extended Backus-Naur Form (EBNF):

| Notation | Meaning |
| --- | --- |
| `"fn"` | The exact source text `fn` |
| `rule_name` | Another grammar rule |
| `[ value ]` | `value` is optional |
| `{ value }` | `value` may occur zero or more times |
| `a \| b` | Either `a` or `b` |
| `a, b` | `a` followed by `b`; this comma is EBNF punctuation |
| `;` | Ends an EBNF rule; a source semicolon is written as `";"` |

Whitespace and `//` comments may appear between tokens unless they occur inside
a string or character literal. Newlines do not end statements. A semicolon is
required everywhere the grammar contains `";"`.

For each major section:

1. Read the EBNF to see what token shapes the parser accepts.
2. Read **Allowed** and **Not allowed** for the v0.1 boundary.
3. Read **Semantic checks** for rules handled after parsing.
4. Read **AST responsibility** for information the parser must preserve.

## Parsing versus semantic checking

The parser asks: **Does this source have a valid Vortex shape?** The semantic
checker asks: **Does that valid shape make sense?**

Both lines match the variable-declaration grammar:

```vortex
let count: i32 = 10;
let count: bool = 10;
```

The second is semantically invalid because an integer cannot initialize a
`bool`.

Array dimensions follow the same separation. Dimensions are full expressions
in the grammar:

```vortex
let values: [f32; 2 + 2] = [0.0; 2 + 2];
```

The parser stores `2 + 2` as an expression. Semantic analysis must prove it is
integer-valued and evaluable at compile time. Fixed-size arrays do not accept a
runtime-only dimension:

```vortex
fn make_values(size: usize) {
    let values: [f32; size] = [0.0; size]; // semantic error in v0.1
}
```

Unless a section says otherwise, these are semantic checks:

- whether a name is declared, visible, or duplicated;
- whether an expression has the required type;
- whether a literal fits its required type;
- whether an assignment target is mutable;
- whether a reference is valid;
- whether `return`, `break`, or `continue` is legal in context;
- whether an array dimension is a compile-time integer expression;
- whether a call has a valid callee, argument count, and argument types.

## Program structure

```ebnf
program ::=
    { top_level_declaration } ;

top_level_declaration ::=
      function
    | struct_definition ;
```

### Allowed

- Any number of top-level functions and structs, in either order.
- An empty source file as a parseable program.

```vortex
struct Pair {
    left: i32,
    right: i32,
}

fn main() {
    let pair = Pair { left: 2, right: 3 };
    print(pair.left + pair.right);
}
```

### Not allowed

- Top-level executable statements or `let` declarations.
- Nested functions or structs.
- Modules, imports, classes, enums, traits, aliases, or generics.

```vortex
let count = 3;      // syntax error: local declarations belong in blocks
print("starting");  // syntax error: statement at top level
```

### Semantic checks

An executable must contain exactly one `main` with no parameters and a `void`
return type. The parser may still build an AST for a file that violates this so
semantic analysis can issue a precise entry-point diagnostic.

### AST responsibility

The program root preserves top-level declarations in source order. Each
declaration preserves its source location.

## Functions and parameters

```ebnf
function ::=
    "fn", identifier,
    "(", [ parameter_list ], ")",
    [ "->", type ],
    block ;

parameter_list ::=
    parameter, { ",", parameter } ;

parameter ::=
    identifier, ":", type ;
```

An omitted return annotation means `void`.

### Allowed

- Zero or more typed parameters.
- Any v0.1 type as a parameter or explicit return type.
- An omitted return annotation for a `void` function.
- An empty body.

```vortex
fn announce() {
    print("ready");
}

fn add(left: i32, right: i32) -> i32 {
    return left + right;
}

fn fill(values: &mut [f32; 2 + 2], value: f32) -> void {
    values[0] = value;
}
```

### Not allowed

- Untyped or defaulted parameters.
- A trailing comma in a parameter list.
- Function declarations without bodies.
- Methods, generic parameters, or variadic parameters.

```vortex
fn untyped(value) {}             // missing `: type`
fn defaulted(value: i32 = 1) {} // defaults are unsupported
fn trailing(value: i32,) {}     // trailing comma is unsupported
```

### Semantic checks

Parameter names must be unique. The return type controls which `return` forms
are valid. `main` must have the required v0.1 signature.

### AST responsibility

A function preserves its name, ordered parameter declarations, return type,
body, and locations. Each parameter preserves its name and type. Tooling that
must reproduce source exactly may also distinguish omitted `-> type` from
explicit `-> void`.

## Struct definitions and fields

```ebnf
struct_definition ::=
    "struct", identifier, "{",
    [ struct_field, { ",", struct_field }, [ "," ] ],
    "}" ;

struct_field ::=
    identifier, ":", type ;
```

### Allowed

- Empty structs.
- Typed, comma-separated fields.
- An optional trailing field comma.

```vortex
struct Marker {}

struct Point {
    x: f32,
    y: f32,
}
```

### Not allowed

- Field defaults, untyped fields, methods, visibility modifiers, inheritance,
  or generic fields.
- Semicolons in place of commas between fields.

```vortex
struct Point {
    x: f32 = 0.0, // field defaults are unsupported
    y,            // a field type is required
}
```

### Semantic checks

Struct names must not conflict with another declaration in the same scope.
Field names must be unique, and every field type must resolve.

### AST responsibility

A struct preserves its name and ordered field declarations. Each field
preserves its name, type, and location.

## Blocks and statements

```ebnf
block ::=
    "{", { statement }, "}" ;

statement ::=
      variable_declaration
    | assignment_statement
    | return_statement
    | expression_statement
    | if_statement
    | while_statement
    | for_statement
    | break_statement
    | continue_statement
    | block ;
```

A block contains zero or more statements, is itself a statement, and creates a
nested local scope.

### Variable declarations

```ebnf
variable_declaration ::=
    "let", [ "mut" ], identifier,
    [ ":", type ],
    "=", expression, ";" ;
```

**Allowed:** one immutable or mutable local, with an explicit or inferred type,
and always with an initializer.

```vortex
let width = 128;
let mut total: f32 = 0.0;
```

**Not allowed:** a missing initializer, `mut` after the name, or multiple names.

```vortex
let width;
let total mut = 0;
let left, right = 0, 1;
```

**Semantic checks:** infer an omitted type, check an explicit type against the
initializer, and record mutability.

**AST responsibility:** preserve the name, `mut` flag, optional written type,
initializer, and location.

### Assignment statements

```ebnf
assignment_statement ::=
    assignment_target, assignment_operator, expression, ";" ;

assignment_target ::=
    identifier,
    {
        "[", expression, { ",", expression }, "]"
      | ".", identifier
    } ;

assignment_operator ::=
      "="
    | "+="
    | "-="
    | "*="
    | "/="
    | "%=" ;
```

**Allowed:** names, indexed elements, fields, and chains of index and field
suffixes as targets.

```vortex
count = 4;
total += value;
matrix[row, column] *= scale;
points[index].x = 0.0;
```

**Not allowed:** assignment as an expression, an arbitrary expression target,
or unsupported compound assignment operators.

```vortex
let result = count = 4;
(left + right) = 4;
flags &= mask;
```

**Semantic checks:** the target must exist, be assignable and mutable, and
accept the right-hand type. Compound assignment also requires the corresponding
binary operation to be valid.

**AST responsibility:** preserve the operator, complete target, value, and
location.

### Return statements

```ebnf
return_statement ::=
    "return", [ expression ], ";" ;
```

**Allowed:** `return;` and `return expression;`.

**Not allowed:** a missing semicolon or multiple returned expressions.

```vortex
return value             // missing `;`
return left, right;      // multiple returns are unsupported
```

**Semantic checks:** `return` must occur in a function. A `void` function cannot
return a value, and a non-`void` function must return a compatible value.

**AST responsibility:** preserve the optional expression.

### Expression statements

```ebnf
expression_statement ::=
    expression, ";" ;
```

**Allowed:** any expression followed by `;`; calls are the usual useful form.

```vortex
print("ready");
calculate();
```

**Not allowed:** omitting the semicolon. Assignment uses its own statement rule
because it is not an expression.

**Semantic checks:** an implementation may warn about a discarded value with
no effect, but it remains valid syntax.

**AST responsibility:** preserve the evaluated expression.

### Conditional statements

```ebnf
if_statement ::=
    "if", expression, block,
    [ "else", ( if_statement | block ) ] ;
```

**Allowed:** `if`, `if`/`else`, and `else if`, all with braced bodies.

```vortex
if score >= 50 {
    print("passed");
} else if score >= 40 {
    print("close");
} else {
    print("try again");
}
```

**Not allowed:** value-producing `if`, an unbraced body, or a bare statement
after `else`.

```vortex
let label = if ready { "yes" } else { "no" };
if ready print("ready");
```

**Semantic checks:** each condition must be `bool`.

**AST responsibility:** preserve the condition, then block, and optional else
branch. An else branch is another `if` or a block.

### While loops

```ebnf
while_statement ::=
    "while", expression, block ;
```

**Allowed:** one condition and a braced body.

```vortex
while index < count {
    index += 1;
}
```

**Not allowed:** `do while`, a missing condition, or an unbraced body.

**Semantic checks:** the condition must be `bool`.

**AST responsibility:** preserve the condition and body.

### For loops

```ebnf
for_statement ::=
    "for", identifier, "in", expression, block ;
```

**Allowed:** one loop variable, one iterable expression, and a braced body.
Ranges are the normal v0.1 iterable.

```vortex
for index in 0..4 {
    print(index);
}

for index in start..=finish {
    print(index);
}
```

**Not allowed:** C-style loops, multiple bindings, or an unbraced body.

```vortex
for (let i = 0; i < 4; i += 1) {}
for index, value in values {}
```

**Semantic checks:** the expression must be iterable. The loop variable receives
the element type and loop-local scope.

**AST responsibility:** preserve the variable name, iterable, and body.

### Break and continue

```ebnf
break_statement ::=
    "break", ";" ;

continue_statement ::=
    "continue", ";" ;
```

**Allowed:** exactly `break;` and `continue;`.

**Not allowed:** labels, values, conditions, or omitted semicolons.

```vortex
break outer;
continue 2;
```

**Semantic checks:** both must occur inside a loop.

**AST responsibility:** preserve the statement kind and location.

### Standalone blocks

A block may appear anywhere a statement may appear:

```vortex
{
    let temporary = 42;
    print(temporary);
}
```

Blocks do not produce values in v0.1, and no semicolon follows the closing
brace.

## Expressions and precedence

The grammar is ordered from lowest to highest precedence. Postfix operations
bind most tightly; ranges bind least tightly.

| Precedence | Category | Operators/forms | Associativity |
| ---: | --- | --- | --- |
| 1 | Range | `..`, `..=` | Non-chaining |
| 2 | Logical OR | `||` | Left |
| 3 | Logical AND | `&&` | Left |
| 4 | Bitwise OR | `\|` | Left |
| 5 | Bitwise XOR | `^` | Left |
| 6 | Bitwise AND | `&` | Left |
| 7 | Equality | `==`, `!=` | Left by grammar |
| 8 | Comparison | `<`, `<=`, `>`, `>=` | Left by grammar |
| 9 | Shift | `<<`, `>>` | Left |
| 10 | Additive | `+`, `-` | Left |
| 11 | Multiplicative | `*`, `/`, `%` | Left |
| 12 | Unary | `+`, `-`, `!`, `~`, `&`, `&mut` | Right |
| 13 | Postfix | call, index, field | Left/chained |
| 14 | Primary | literal, name, array, struct, grouping | N/A |

```ebnf
expression ::=
    range_expression ;

range_expression ::=
    logical_or_expression,
    [ range_operator, logical_or_expression ] ;

range_operator ::=
      ".."
    | "..=" ;

logical_or_expression ::=
    logical_and_expression, { "||", logical_and_expression } ;

logical_and_expression ::=
    bitwise_or_expression, { "&&", bitwise_or_expression } ;

bitwise_or_expression ::=
    bitwise_xor_expression, { "|", bitwise_xor_expression } ;

bitwise_xor_expression ::=
    bitwise_and_expression, { "^", bitwise_and_expression } ;

bitwise_and_expression ::=
    equality_expression, { "&", equality_expression } ;

equality_expression ::=
    comparison_expression, { ( "==" | "!=" ), comparison_expression } ;

comparison_expression ::=
    shift_expression, { ( "<" | "<=" | ">" | ">=" ), shift_expression } ;

shift_expression ::=
    additive_expression, { ( "<<" | ">>" ), additive_expression } ;

additive_expression ::=
    multiplicative_expression, { ( "+" | "-" ), multiplicative_expression } ;

multiplicative_expression ::=
    unary_expression, { ( "*" | "/" | "%" ), unary_expression } ;

unary_expression ::=
      ( "+" | "-" | "!" | "~" ), unary_expression
    | "&", [ "mut" ], unary_expression
    | postfix_expression ;

postfix_expression ::=
    primary_expression, { call_suffix | index_suffix | field_suffix } ;

call_suffix ::=
    "(", [ argument_list ], ")" ;

argument_list ::=
    expression, { ",", expression } ;

index_suffix ::=
    "[", expression, { ",", expression }, "]" ;

field_suffix ::=
    ".", identifier ;

primary_expression ::=
      literal
    | identifier
    | array_expression
    | struct_expression
    | "(", expression, ")" ;
```

### Allowed

- Arithmetic, equality, comparison, logical, bitwise, and shift operations.
- Inclusive and exclusive ranges with both endpoints.
- Prefix unary operations, including shared and mutable references.
- Calls with zero or more arguments.
- One or more comma-separated indices.
- Chained postfix operations and parenthesized grouping.

```vortex
left + right * scale
!(ready && enabled)
start..=finish
matrix[row, column]
points[index].x
scale(&mut values, 2.0)
```

Python-style casts such as `f32(value)` use the call grammar. Name and type
checking determine whether `f32` is a conversion rather than a function.

### Not allowed

- Assignment, ternary, value-producing `if`, `match`, or lambda expressions.
- Open-ended or chained ranges.
- A trailing comma in a call or index suffix.
- Optional chaining, null-coalescing, increment, or decrement.

```vortex
count = 3
ready ? left : right
..finish
start..middle..finish
calculate(value,)
```

### Semantic checks

Operators must support their operand types. Logical operands must be `bool`;
bitwise and shift operands must be valid integers. Calls, indices, fields,
references, and ranges are checked against resolved types. `&mut` requires
mutable, addressable storage.

The grammar permits repeated equality and comparison operators because these
levels use the same left-folding form as other binary levels. It does not add
mathematical comparison chaining; semantic analysis rejects an operation when
the left subexpression's result type is incompatible.

### AST responsibility

Preserve operators, children, and locations. A range preserves `..` versus
`..=`. An index preserves every index in order. A reference preserves whether
`mut` was written.

## Array expressions

```ebnf
array_expression ::=
    "[", ( repeat_array_body | array_element_list ), "]" ;

array_element_list ::=
    expression, { ",", expression }, [ "," ] ;

repeat_array_body ::=
    expression, ";",
    expression, { ",", expression } ;
```

The semicolon distinguishes a repeat array from an element list. Every repeat
dimension is a full expression.

### Allowed

- One or more explicit elements, optionally with a trailing comma.
- A repeated value followed by one or more dimension expressions.
- Multidimensional repeats and compound dimension expressions.

```vortex
[1.0, 2.0, 3.0]
[1, 2 + 3, calculate()]
[0.0; 16]
[0.0; 2 + 2, 8 / 2]
[false; (rows_per_tile * 2)]
```

The final example is syntactically valid, but it is semantically valid only if
`rows_per_tile * 2` is an integer compile-time constant.

### Not allowed

- Empty arrays.
- A missing repeat value or dimension.
- A trailing comma after repeat dimensions.
- Runtime-sized repeat arrays.
- Mixing the list comma form with the repeat semicolon form.

```vortex
[]
[; 4]
[0.0;]
[0.0; 4,]
[1.0, 2.0; 4]
```

### Semantic checks

Element-list values need a compatible common type and shape. Each repeat
dimension must have an integer type and be evaluable at compile time. Layout,
dimension-value validity, and total-size overflow are semantic or layout checks.

### AST responsibility

An element-list array preserves ordered element expressions. A repeat array
preserves its value expression and ordered dimension expressions. The parser
must not reduce dimensions to integer tokens; constant evaluation comes later.

## Struct expressions

```ebnf
struct_expression ::=
    identifier, "{",
    field_initializer,
    { ",", field_initializer },
    [ "," ],
    "}" ;

field_initializer ::=
    identifier, ":", expression ;
```

### Allowed

- A named struct expression with at least one named field.
- Any expression as a field value.
- An optional trailing comma.

```vortex
Point { x: 0.0, y: 0.0 }

Point {
    x: calculate_x(),
    y: origin.y + offset,
}
```

### Not allowed

- Empty, positional, shorthand, anonymous, or update-style struct expressions.

```vortex
Point {}
Point(0.0, 0.0) // a call, not struct construction
Point { x, y }
```

An identifier followed by `{` begins a struct expression only when its contents
have `field: value` form.

### Semantic checks

The name must resolve to a struct. All required fields must appear exactly once;
unknown fields and incompatible field values are invalid.

### AST responsibility

Preserve the struct name plus every field name, value, order, and location.

## Types

```ebnf
type ::=
      primitive_type
    | array_type
    | reference_type
    | identifier ;

primitive_type ::=
      "void"
    | "bool"
    | "char"
    | "i32"
    | "u32"
    | "usize"
    | "f32"
    | "f64"
    | "String" ;

array_type ::=
    "[", type, ";",
    expression, { ",", expression },
    "]" ;

reference_type ::=
    "&", [ "mut" ], type ;
```

Every array dimension is a full expression. Semantic analysis reduces it to a
compile-time integer before type equality, layout, and code generation.

### Primitive types

| Type | Meaning |
| --- | --- |
| `void` | No returned value |
| `bool` | `true` or `false` |
| `char` | One Unicode character |
| `i32` | 32-bit signed integer |
| `u32` | 32-bit unsigned integer |
| `usize` | Target-sized unsigned size/index integer |
| `f32` | 32-bit floating-point number |
| `f64` | 64-bit floating-point number |
| `String` | UTF-8 string value |

### Allowed

- Listed primitive types.
- Fixed arrays with one or more dimension expressions.
- Nested array and reference types.
- Shared and mutable references.
- Identifiers naming user-defined types.

```vortex
i32
[f32; 16]
[f32; 2 + 2, 8 / 2]
[[i32; 2]; 3]
&[f32; 16]
&mut [f32; rows_per_tile * 2, columns_per_tile]
Point
```

The mutable-reference example is syntactically valid only in the sense that it
parses. It becomes semantically valid only if both names resolve to integer
compile-time constants. v0.1 does not add constant declarations merely because
dimension syntax accepts expressions.

### Not allowed

- Missing dimensions or a trailing dimension comma.
- Runtime-sized arrays.
- Tuples, unions, function types, inferred `_` types, generics, aliases, or raw
  pointers.
- New primitive names such as `i64`, `u64`, `u8`, `f16`, or `bf16`.

```vortex
[f32;]
[f32; 4,]
Vector<f32>
*mut i32
u64 // parses as a named type, then fails name resolution
```

### Semantic checks

Named types must resolve. `void` may appear only where a no-value type makes
sense. Every array dimension must be integer-typed, compile-time evaluable, and
safe to use for layout. Reference validity and mutability are checked where
reference values are created and used.

### AST responsibility

Use distinct primitive, array, reference, and named type nodes. An array type
preserves its element type and all dimension expressions. A reference preserves
its referenced type and `mut` flag. A named type preserves the identifier until
name resolution.

## Literals

```ebnf
literal ::=
      integer_literal
    | floating_literal
    | boolean_literal
    | character_literal
    | string_literal ;

integer_literal ::=
      decimal_integer
    | binary_integer ;

decimal_integer ::=
    digit, { digit } ;

binary_integer ::=
    "0b", binary_digit, { binary_digit } ;

floating_literal ::=
    digit, { digit }, ".", digit, { digit }, [ exponent ] ;

exponent ::=
    ( "e" | "E" ), [ "+" | "-" ], digit, { digit } ;

boolean_literal ::=
      "true"
    | "false" ;

character_literal ::=
    "'", ( character_content | escape_sequence ), "'" ;

string_literal ::=
    '"', { string_content | escape_sequence }, '"' ;

escape_sequence ::=
      "\\n"
    | "\\t"
    | "\\\\"
    | '\\"'
    | "\\'" ;
```

`character_content` is one valid Unicode character other than `'`, `\\`, or a
line break. `string_content` is one valid Unicode character other than `"`,
`\\`, or a line break.

### Allowed

```vortex
0
42
0b101010
3.14
1.0e-4
true
'A'
'λ'
'\n'
"Vortex"
"line one\nline two"
```

### Not allowed

- Hexadecimal or octal notation, numeric separators, or type suffixes.
- A leading or trailing decimal point without digits on both sides.
- Raw, byte, or multiline strings.
- Unsupported escapes.
- More or fewer than one character or escape in a character literal.

```vortex
0xff
1_000
42u32
.5
5.
"bad\rvalue"
'ab'
```

### Semantic checks

Numeric literals must fit or convert to the required type. With no contextual
type, integers default to `i32` and floating literals default to `f32`. A
leading `-` is a unary operator applied to a positive numeric literal, not part
of the literal grammar.

### AST responsibility

Preserve literal category, decoded value, and location. Type checking may assign
the concrete numeric type later. `-42` is a negate node containing the positive
literal `42`.

## Identifiers and keywords

```ebnf
identifier ::=
    identifier_start, { identifier_continue } ;

identifier_start ::=
      letter
    | "_" ;

identifier_continue ::=
      letter
    | digit
    | "_" ;

digit ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;

binary_digit ::= "0" | "1" ;
```

In v0.1, `letter` means ASCII `A` through `Z` or `a` through `z`. Names are
case-sensitive.

### Allowed

```text
value
item_count
Point
_temporary
matrix2
```

### Not allowed

- A digit first, punctuation other than `_`, a keyword, or non-ASCII letters.

```vortex
let 2values = 0;
let item-count = 0;
let while = true;
```

The v0.1 keywords are:

```text
fn      struct  let     mut     return
if      else    while   for     in
break   continue
void    bool    char    i32     u32
usize   f32     f64     String
true    false
```

### Semantic checks

The lexer distinguishes keywords from identifiers. Later stages resolve names,
enforce scope, and diagnose duplicates or unknown names.

### AST responsibility

Preserve spelling and location. Do not resolve a type-name or value-name inside
the parser.

## Implementation checklist

### Source file and declarations

- [ ] Parse an ordered sequence of functions and structs.
- [ ] Parse typed parameters and optional return annotations.
- [ ] Parse empty and non-empty structs with optional trailing field commas.
- [ ] Preserve declaration and child-node locations.

### Statements

- [ ] Parse variable, assignment, return, expression, `if`, `while`, `for`,
  `break`, `continue`, and block statements.
- [ ] Preserve variable mutability and optional written type.
- [ ] Restrict assignment syntax to assignment-target shapes.
- [ ] Preserve exact assignment operators and `else if` structure.

### Expressions

- [ ] Implement every precedence level in the documented order.
- [ ] Keep unary parsing right-associative and binary loops left-associative.
- [ ] Prevent a range from consuming a second range operator.
- [ ] Preserve exclusive/inclusive ranges and shared/mutable references.
- [ ] Support chained calls, multidimensional indices, and fields.
- [ ] Keep assignment outside the expression parser.

### Arrays and types

- [ ] Distinguish element lists from repeat arrays at `;`.
- [ ] Parse each repeat-array dimension with the full expression parser.
- [ ] Parse each array-type dimension with the full expression parser.
- [ ] Stop a dimension expression at its enclosing comma or `]`.
- [ ] Preserve dimension expressions in source order.
- [ ] Leave integer typing and compile-time evaluation to semantic analysis.
- [ ] Build distinct primitive, array, reference, and named type nodes.

### Diagnostics and verification

- [ ] Report the unexpected token, its location, and the expected form.
- [ ] Recover at `;`, `}`, or the next top-level `fn`/`struct` when possible.
- [ ] Add accepted and rejected tests for every major production.
- [ ] Test compound dimensions such as `[f32; 2 + 2]` and
  `[0.0; rows * columns]`.
- [ ] Verify runtime-only and non-integer dimensions parse first, then fail
  semantic checking.

## v0.1 boundary summary

Vortex v0.1 contains functions, structs, local variables, fixed-size arrays,
references, imperative control flow, and a compact expression system. It does
not contain modules, generics, classes, enums, methods, pattern matching,
exceptions, closures, dynamic arrays, raw pointers, or runtime-sized layouts.

Full expression syntax for array dimensions does not make arrays dynamic. The
parser accepts the expression; semantic analysis must still prove it is an
integer compile-time constant before type layout or code generation.
