# Basic source rules

Vortex uses a small set of source-code rules in v0.1:

- `//` starts a comment that continues to the end of the line.
- Simple statements end with a semicolon.
- Blocks use `{}` and do not need a semicolon after the closing brace.
- Names may contain letters, numbers, and `_`, but they cannot start with a
  number.
- Names are case-sensitive, so `value` and `Value` are different names.

Strings and characters support these escape sequences:

| Escape | Meaning |
| --- | --- |
| `\n` | new line |
| `\t` | tab |
| `\\` | backslash |
| `\"` | double quote |
| `\'` | single quote |

For example:

```vortex
print("first line\nsecond line");
let quote: char = '\'';
```
