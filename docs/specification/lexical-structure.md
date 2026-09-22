# 2. Lexical structure

The lexer converts source characters into tokens. It removes whitespace and
comments that separate tokens, preserves token spelling and source spans, and
reports malformed token text.

## 2.1 Whitespace and line boundaries

Spaces, horizontal tabs, and line breaks separate tokens. A line break has no
statement-terminating meaning. Statements that require a semicolon must contain
`;` even when followed by a line break.

```vortex
let width = 128;
let area =
    width * width;
```

Both declarations are valid.

## 2.2 Comments

A line comment begins with `//` outside a string or character literal and
continues to the end of the line.

```vortex
// Allocate one row.
let row: [f32; 4] = [0.0; 4]; // four zero values
```

Block comments are not part of v0.1:

```vortex
/* invalid v0.1 syntax */
```

The character sequence `//` inside a string literal is ordinary string
content.

## 2.3 Identifiers

An identifier begins with an ASCII letter or `_`. Each later character is an
ASCII letter, decimal digit, or `_`.

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
```

Identifiers are case-sensitive. `value`, `Value`, and `VALUE` are distinct
names.

Valid spellings include:

```text
value
item_count
_temporary
Matrix2
```

Invalid identifier spellings include `2values`, `item-count`, and non-ASCII
letters. A keyword cannot be used as an identifier.

## 2.4 Keywords

The following spellings are reserved:

```text
fn      struct  let     mut     return
if      else    while   for     in
break   continue
void    bool    char    i32     u32
usize   f32     f64     String
true    false
```

Keyword matching is case-sensitive. `String` is a keyword, while `string` is
an identifier.

## 2.5 Literals

### Integer literals

Decimal integer literals contain one or more decimal digits. Binary integer
literals begin with `0b` followed by one or more binary digits.

```vortex
0
42
0b101010
```

Hexadecimal, octal, digit separators, and suffixes are not accepted in v0.1.
A leading minus sign is a unary operator, not part of the literal.

### Floating-point literals

A floating-point literal requires digits on both sides of `.` and may contain
an exponent.

```vortex
3.14
1.0e-4
6.02E+23
```

`.5` and `5.` are invalid.

### Boolean literals

The two boolean literals are `true` and `false`.

### Character and string literals

Characters use single quotes. Strings use double quotes.

```vortex
'A'
'λ'
"Vortex"
"first line\nsecond line"
```

A character literal contains exactly one Unicode character or one supported
escape. String and character literals may use these escapes:

| Escape | Decoded value |
| --- | --- |
| `\n` | Line feed |
| `\t` | Horizontal tab |
| `\\` | Backslash |
| `\"` | Double quote |
| `\'` | Single quote |

Raw strings, byte strings, multiline strings, and other escapes are outside
v0.1.

## 2.6 Operators and punctuation

The lexer recognizes the following operator spellings:

```text
+   -   *   /   %
=   ==  !=  <   >   <=  >=
&&  ||  !
&   |   ^   ~   <<  >>
+=  -=  *=  /=  %=
..  ..=
```

It also recognizes:

```text
( ) { } [ ] ; . : , ->
```

When one token spelling is a prefix of another, the lexer uses the longest
valid token. For example, `..=` is one inclusive-range token rather than `..`
followed by `=`.

## 2.7 Lexical errors

The lexer must reject:

- an unknown character;
- an unterminated string or character literal;
- an unsupported escape;
- a character literal with the wrong number of decoded characters;
- `0b` with no following binary digit;
- malformed floating-point exponent text.

The lexer must always make progress after reporting an invalid character. End
of file is represented by one stable end token and does not consume source
characters repeatedly.

## 2.8 Token source data

Each token preserves its kind, original spelling, and source span. Later stages
must not need to reconstruct spelling from the token kind. If token text points
into the original source buffer, that buffer must remain alive for the complete
lifetime of the tokens.
