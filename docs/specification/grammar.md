# Vortex v0.1 Grammar

This document defines the source grammar for Vortex v0.1 using EBNF.

The notation means:

- quoted text such as `"fn"` must appear exactly as written;
- `[ value ]` means that `value` is optional;
- `{ value }` means that `value` may repeat zero or more times;
- `a | b` means either `a` or `b`;
- a comma separates items that appear in sequence.

Whitespace and `//` comments may appear between tokens unless they occur inside
a string or character literal.

## Program structure

```ebnf
program ::=
    { function | struct_definition } ;

function ::=
    "fn", identifier,
    "(", [ parameter_list ], ")",
    [ "->", type ],
    block ;

parameter_list ::=
    parameter, { ",", parameter } ;

parameter ::=
    identifier, ":", type ;

block ::=
    "{", { statement }, "}" ;
```

When a function omits `-> type`, its return type is `void`.

Every executable program must contain exactly one `main` function with no
parameters and a `void` return type. This requirement is checked after parsing.

## Struct definitions
```ebnf
struct_definition ::=
    "struct", identifier, "{",
    [ struct_field, { ",", struct_field }, [ "," ] ],
    "}" ;

struct_field ::=
    identifier, ":", type ;
```

Example:

```vortex
struct Point {
    x: f32,
    y: f32,
}
```

## Statements

```ebnf
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

variable_declaration ::=
    "let", [ "mut" ], identifier,
    [ ":", type ],
    "=", expression, ";" ;

assignment_statement ::=
    assignment_target,
    assignment_operator,
    expression,
    ";" ;

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

return_statement ::=
    "return", [ expression ], ";" ;

expression_statement ::=
    expression, ";" ;

if_statement ::=
    "if", expression, block,
    [ "else", ( if_statement | block ) ] ;

while_statement ::=
    "while", expression, block ;

for_statement ::=
    "for", identifier, "in", expression, block ;

break_statement ::=
    "break", ";" ;

continue_statement ::=
    "continue", ";" ;
```

Whether an assignment target is mutable, whether `return` has the correct
value, and whether `break` or `continue` appears inside a loop are semantic
checks performed after parsing.

## Expressions

The expression grammar is written from lowest precedence to highest precedence.

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
    logical_and_expression,
    { "||", logical_and_expression } ;

logical_and_expression ::=
    bitwise_or_expression,
    { "&&", bitwise_or_expression } ;

bitwise_or_expression ::=
    bitwise_xor_expression,
    { "|", bitwise_xor_expression } ;

bitwise_xor_expression ::=
    bitwise_and_expression,
    { "^", bitwise_and_expression } ;

bitwise_and_expression ::=
    equality_expression,
    { "&", equality_expression } ;

equality_expression ::=
    comparison_expression,
    { ( "==" | "!=" ), comparison_expression } ;

comparison_expression ::=
    shift_expression,
    { ( "<" | "<=" | ">" | ">=" ), shift_expression } ;

shift_expression ::=
    additive_expression,
    { ( "<<" | ">>" ), additive_expression } ;

additive_expression ::=
    multiplicative_expression,
    { ( "+" | "-" ), multiplicative_expression } ;

multiplicative_expression ::=
    unary_expression,
    { ( "*" | "/" | "%" ), unary_expression } ;

unary_expression ::=
      ( "+" | "-" | "!" | "~" ), unary_expression
    | "&", [ "mut" ], unary_expression
    | postfix_expression ;

postfix_expression ::=
    primary_expression,
    { call_suffix | index_suffix | field_suffix } ;

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

Python-style casts such as `f32(value)` use the normal call grammar. Name and
type checking later determine that `f32` is a type conversion rather than a
user-defined function.

## Array expressions

```ebnf
array_expression ::=
    "[", ( repeat_array_body | array_element_list ), "]" ;

array_element_list ::=
    expression, { ",", expression }, [ "," ] ;

repeat_array_body ::=
    expression, ";",
    integer_literal, { ",", integer_literal } ;
```

Examples:

```vortex
[1.0, 2.0, 3.0]
[0.0; 16]
[0.0; 4, 4]
```

Empty array literals are not supported in v0.1 because they do not provide an
element type for local type inference.

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

Vortex v0.1 requires at least one field in a struct expression. During parsing,
an identifier followed by `{` begins a struct expression only when the contents
have the `field: value` form.

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
    integer_literal, { ",", integer_literal },
    "]" ;

reference_type ::=
    "&", [ "mut" ], type ;
```

Examples:

```vortex
i32
[f32; 16]
[f32; 4, 4]
&[f32; 16]
&mut [f32; 4, 4]
Point
```

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

`character_content` means one valid Unicode character other than `'`, `\\`, or
a line break. `string_content` means a valid Unicode character other than `"`,
`\\`, or a line break.

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

For v0.1, `letter` means an ASCII letter from `A` through `Z` or `a` through
`z`. Keywords cannot be used as identifiers.

The v0.1 keywords are:

```text
fn      struct  let     mut     return
if      else    while   for     in
break   continue
void    bool    char    i32     u32
usize   f32     f64     String
true    false
```
