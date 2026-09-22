# Basic Source Rules

[Previous: Syntax philosophy](01-syntax-philosophy.md) | [Tour index](README.md) | [Next: Hello world](03-hello-world.md)

## Learning goals

After this chapter, you should be able to write valid identifiers, place
semicolons and braces, add comments, and use the supported escapes.

## Comments

### Syntax

```vortex
// comment text continues to the end of this line
```

### Allowed

- `//` comments on their own line.
- `//` comments after code.
- Comments between tokens where whitespace is allowed.

```vortex
let width = 128; // image width
```

### Not allowed

Block comments are not part of v0.1:

```vortex
/* not valid v0.1 syntax */
```

The lexer skips a `//` comment but does not treat `//` inside a string as a
comment.

## Statements, blocks, and semicolons

Simple statements end in `;`. Blocks use `{}` and do not take a semicolon after
the closing brace.

```vortex
let value = 10;
print(value);

if value > 0 {
    print("positive");
}
```

These forms are invalid:

```vortex
let value = 10       // invalid: missing semicolon

if value > 0 {
    print(value);
};                    // invalid: block statement followed by semicolon
```

The parser enforces statement termination and matching braces.

## Identifiers

An identifier is a programmer-defined name.

### Syntax

```text
first character: letter or _
later characters: letters, digits, or _
```

### Allowed

```vortex
let value = 1;
let item_count = 2;
let _temporary = 3;
let Value2 = 4;
```

Names are case-sensitive, so `value` and `Value` are different names.

### Not allowed

```vortex
let 2values = 2; // invalid: starts with a digit
let item-count = 2; // parsed as subtraction, not one name
let fn = 2; // invalid: fn is a keyword
```

The lexer determines whether text is an identifier or keyword. Name resolution
later checks whether an identifier was declared and is visible.

## String and character escapes

Strings use double quotes. Characters use single quotes.

| Escape | Meaning |
| --- | --- |
| `\n` | new line |
| `\t` | tab |
| `\\` | backslash |
| `\"` | double quote |
| `\'` | single quote |

Valid examples:

```vortex
print("first line\nsecond line");
let quote: char = '\'';
let path = "left\\right";
```

Invalid examples:

```vortex
let bad = "unknown: \q"; // invalid: unsupported escape
let two: char = 'ab'; // invalid: more than one character
let open = "missing end; // invalid: unterminated string
```

The lexer validates delimiters and escape spellings. Type checking later
distinguishes `char` from `String`.

## Practice and self-check

Correct this source:

```vortex
let 1st_value = "line one\qline two"
```

One valid answer is:

```vortex
let first_value = "line one\nline two";
```
