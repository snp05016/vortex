# Hello World and Program Entry

[Previous: Basic source rules](02-basic-source-rules.md) | [Tour index](README.md) | [Next: Variables and types](04-variables-and-types.md)

## Learning goals

After this chapter, you should know the required executable entry point and the
difference between parsing a function and validating `main`.

## Smallest useful program

```vortex
fn main() {
    print("Hello, world!");
}
```

Every executable Vortex program begins in `main`. In v0.1, `main` takes no
arguments and returns `void`. The `-> void` may be left out, as shown above.

## Syntax

```text
fn main() [-> void] {
    statements
}
```

The brackets above explain that `-> void` is optional; they are not typed in a
Vortex program.

## Allowed

Both entry-point declarations are equivalent:

```vortex
fn main() {
    print("ready");
}
```

```vortex
fn main() -> void {
    print("ready");
}
```

A file may also contain helper functions and structs.

## Not allowed

```vortex
fn main(arguments: String) { }
// invalid: v0.1 main takes no parameters
```

```vortex
fn main() -> i32 {
    return 0;
}
// invalid: v0.1 main returns void
```

```vortex
fn main() { }
fn main() { }
// invalid: an executable must have exactly one main
```

A file containing only helper declarations may be syntactically parseable, but
it is not a valid executable program without `main`.

## Compiler handling

The parser builds ordinary function-declaration nodes for `main` and other
functions. A later semantic/program-validation pass checks that exactly one
function is named `main`, has no parameters, and returns `void`. The runtime
then begins execution at that validated function.

## Practice and self-check

**Question:** Is `fn main() -> void {}` different from `fn main() {}`?

**Answer:** No. An omitted return type means `void`.
