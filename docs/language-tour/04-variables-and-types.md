# Variables and types

[Previous: Hello world](03-hello-world.md) | [Tour index](README.md) | [Next: Types planned for later](05-types-planned-for-later.md)

## Learning goals

After this chapter, you should be able to declare mutable and immutable local
variables, choose a v0.1 type, read an array type, and explain how the compiler
validates dimension expressions and references.

## Variable declaration syntax

```text
let [mut] name [: type] = expression;
```

The brackets explain optional pieces; they are not typed in the program. Every
v0.1 local variable needs an initializer expression. A type annotation is
optional when the compiler can infer the type from that initializer.

### Allowed

```vortex
let width = 128;
let height: usize = 64;
let mut total: f32 = 0.0;
```

### Not allowed

```vortex
let missing;              // no initializer
let count: i32 = "five"; // initializer has the wrong type
let width = 128;
width = 256;              // width was not declared mut
```

The parser checks that the declaration has the required pieces. Type checking
infers or verifies the type. Assignment checking later enforces mutability.

## Automatic type inference

Vortex infers local-variable types when you use `let` with a starting value.
This saves you from writing obvious types repeatedly, while the program remains
statically typed.

```vortex
let x = 5; // inferred as i32

fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

fn main() {
    let result = add(5, 10); // inferred as i32
}
```

Vortex does not use `auto` for local variables. `let` is the one clear way to
ask for local type inference.

The compiler cannot infer a type when there is no starting value:

```vortex
let x; // Compile-time error
```

Every value in Vortex has a type. A type tells the compiler what kind of value it
is working with and how that value can be used.

You can write the type yourself:

```vortex
let width: i32 = 128; // i32 is here for a 32 bit signed integer
```

When the type is obvious, Vortex can figure it out for you:

```vortex
let width = 128; // automatically inferred as i32
```

Variables cannot be changed by default. Write `mut` when a variable needs to
change:

```vortex
let width = 128;
let mut total = 0.0; // stands for mutable type

total = total + 1.0;
```
Mutable variables are variables that can be changed after they are created. Immutable variables cannot be changed after they are created.

## v0.1 type quick reference

| Type form | Example | What it represents |
| --- | --- | --- |
| Primitive | `i32`, `bool`, `String` | A built-in scalar or string type. |
| Fixed-size array | `[f32; 4, 4]` | One element type and compile-time dimensions. |
| Shared reference | `&Point` | Read access to an existing value. |
| Mutable reference | `&mut [f32; 4]` | Read and write access to an existing mutable value. |
| Named type | `Point` | A type introduced by a struct declaration. |

Type aliases, generic types, slices, vectors, tuples, and function types are
not part of v0.1. [Types planned for later](05-types-planned-for-later.md)
separates future ideas from accepted syntax.

## Integer types

Integers are whole numbers. Vortex v0.1 starts with these integer types:

```text
i32   u32   usize
```

`i32` can hold negative and positive whole numbers. `u32` starts at zero and
cannot hold negative numbers. Both use 32 bits.

```vortex
let temperature: i32 = -10;
let image_width: u32 = 1920;
```

Vortex also has `usize` for array lengths and indexes. Its size matches the
machine the program is running on.

```vortex
let index: usize = 4;
```

Invalid uses include assigning a negative value to `u32` or `usize`, applying
integer-only operations to strings, and relying on an integer value outside its
type's range:

```vortex
let count: u32 = -1; // invalid: unsigned integers cannot be negative
let name = "Vortex" % 2; // invalid: remainder requires integers
```

The lexer records the integer literal. Unary `-` is represented separately.
Type checking chooses or verifies `i32`, `u32`, or `usize` and checks whether
the value and operation are legal.

When an integer does not have an explicit type, Vortex treats it as an `i32`
unless the surrounding code requires another integer type.

The smaller and larger integer types `i8`, `i16`, `i64`, `u8`, `u16`, and
`u64` can be added after v0.1. Until `u8` is added, Vortex does not expose a
separate raw-byte type.

## Floating-point types

Floating-point types store numbers with a decimal part:

```text
f32
f64
```

`f32` is smaller and is commonly used for high-performance numerical work.
`f64` uses more memory but can represent numbers with greater precision.

```vortex
let weight: f32 = 0.5;
let precise_value: f64 = 0.123456789;
```

When a decimal number does not have an explicit type, Vortex treats it as an
`f32`. This default fits Vortex's focus on numerical work across CPUs and GPUs.

Types such as `f16` and `bf16` (brain float, with 7 mantissa bits, used for machine learning) 
may be added later for machine-learning and GPU workloads. They are 
not part of the first version of the language.

The `%`, bitwise, and shift operators are not defined for `f32` or `f64` in
v0.1. Type checking rejects uses such as `3.0 & 1.0`.

## Boolean type

A `bool` is either `true` or `false`:

```vortex
let finished: bool = false;
```

A boolean contains one bit of information, but Vortex does not promise that it
will use exactly one bit of memory. The compiler may store it differently when
that is better for the target hardware.

Only `true` and `false` are boolean literals. Vortex does not use integers as
implicit booleans, so `if 1 { ... }` is invalid. The type checker requires a
`bool` condition.

## Void

`void` means that a function does not return a value. Vortex uses this name
because it is familiar to C and C++ programmers.

```vortex
fn show_result(value: f32) -> void {
    print(value);
}
```

The `-> void` can be left out when a function has no return value:

```vortex
fn show_result(value: f32) {
    print(value);
}
```

`void` describes the absence of a returned value. It is not a value that can be
stored in a local variable, used as a parameter value, or written as a literal.

## Characters

A `char` stores one Unicode character:

```vortex
let grade: char = 'A';
let symbol: char = 'λ';
```

Character values use single quotes. A character is not always one byte, because
some Unicode characters need more than one byte when stored in memory. A raw
`u8` byte type can be added after v0.1.

A character literal contains exactly one Unicode character or one supported
escape. `'AB'` and `''` are invalid character literals; the lexer reports them.

## Strings

A `String` stores a piece of text:

```vortex
let message: String = "matrix multiplication started";
print(message);
```

String values use double quotes. Vortex strings use UTF-8, so they can contain
text from many languages:

```vortex
let message = "Hello from Vortex!";
let status = "GPU kernel ready";
```

Strings are useful for normal output, error messages, and debugging information.
The built-in `print` function can print strings as well as basic values:

```vortex
let rows: usize = 128;
let elapsed: f32 = 0.42;

print("starting matrix multiplication");
print("rows:", rows);
print("elapsed time:", elapsed);
```

The first version of Vortex only needs simple string creation and printing.
More advanced operations, such as changing parts of a string or searching
inside it, can be added later.

String indexing, interpolation, mutation, concatenation rules, and parsing text
as a number are not defined in v0.1. Use string literals and `print` only where
the current language rules explicitly permit them.

## Fixed-size arrays

An array stores a fixed number of values of the same type next to each other in
memory. Its length is known when the program is compiled.

The type `[f32; 4]` means "an array containing four `f32` values":

```vortex
let values: [f32; 4] = [1.0, 2.0, 3.0, 4.0];
```

Vortex also supports fixed-size multidimensional arrays. Write each dimension
after the semicolon:

```vortex
let matrix: [f32; 2, 2] = [
    [1.0, 2.0],
    [3.0, 4.0],
];

let value = matrix[row, column];
```

The type `[f32; 2, 2]` means a two-row, two-column array of `f32` values. Every
dimension is part of the type and must be known during compilation. This gives
the compiler the shape information it needs for matrix operations.

### Dimension expressions

Each dimension position accepts a full Vortex expression, not only a number
written directly in the source:

```vortex
let row: [f32; 2 + 2] = [0.0; 2 + 2];
let matrix: [f32; 2 * 2, 8 / 2] = [0.0; 2 * 2, 8 / 2];
```

Although the parser accepts an expression, the compiler must be able to
evaluate it during compilation. Its result must be an integer that fits
`usize`. The type `[f32; 2 + 2]` is therefore the same shape as
`[f32; 4]` after constant evaluation.

Allowed dimension expressions are expressions the compiler can prove are
constant integers using supported v0.1 operators. Runtime-dependent,
floating-point, boolean, string, and negative dimensions are invalid:

```vortex
fn make(size: usize) {
    let values: [f32; size] = [0.0; size];
    // invalid: size is a runtime parameter, not a compile-time value
}

let fractional: [f32; 2.5] = [0.0; 2.5];
// invalid: a dimension must evaluate to an integer
```

The parser stores each dimension as an expression AST. Constant evaluation
reduces each expression to an integer, and type checking uses the evaluated
dimensions when comparing array shapes. Code generation receives the final
fixed layout rather than evaluating dimensions at runtime.

## Repeat array expressions

Vortex also supports Rust-style repeat array expressions. Write one value, then
a semicolon, then the number of times it should appear:

```vortex
let zeros = [0.0; 1024];
let flags = [false; 8];
let zero_matrix = [0.0; 4, 4];
```

The first example creates an array of 1,024 `f32` values, all set to `0.0`.
The second creates eight `bool` values, all set to `false`. The final example
creates a four-row, four-column array filled with `0.0`.

Repeat dimensions use the same expression rules as array-type dimensions. Each
one must evaluate during compilation to an integer that fits `usize`:

```vortex
let values = [0.0; 8 + 8];
let tiles = [0.0; 2 * 2, 2 + 2];
```

The value expression before `;` is evaluated once, and its value is used to
fill the complete array. A runtime-dependent dimension is not allowed:

```vortex
fn make(size: usize) {
    let values = [0.0; size];
    // invalid: fixed-size shape cannot depend on a runtime parameter
}
```

Named compile-time constants are not part of v0.1. Use a future `Vector` when
the number of values is only known while the program is running.

## Structs

A struct groups related values into a new type:

```vortex
struct Point {
    x: f32,
    y: f32,
}

let origin = Point {
    x: 0.0,
    y: 0.0,
};
```

Vortex structs are simple value types. Inheritance and complicated class
systems are not part of the initial language.

All declared fields must be present exactly once when constructing a value.
Unknown, repeated, missing, or incorrectly typed fields are compile-time
errors. Struct methods, inheritance, and anonymous structs are not supported.

## References

Vortex v0.1 has basic references so functions can use arrays, strings, and
structs without copying all their data.

A shared reference, written as `&T`, allows a function to read a value. A
mutable reference, written as `&mut T`, also allows the function to change it:

```vortex
fn scale(values: &mut [f32; 4], factor: f32) -> void {
    for index in 0..4 {
        values[index] *= factor;
    }
}

fn main() {
    let mut values = [1.0, 2.0, 3.0, 4.0];
    scale(&mut values, 2.0);
}
```

Vortex does not allow a value to be changed through a shared reference. While a
mutable reference is being used, another reference cannot access the same value.
These basic rules prevent accidental overlapping writes without requiring the
complete future ownership system in v0.1.

The operand of `&` or `&mut` must be an addressable value such as a variable,
field, or array element. A mutable reference additionally requires mutable
storage:

```vortex
let value = 10;
let shared = &value;      // valid: read-only reference
let changed = &mut value; // invalid: value was not declared mut
```

The parser records whether `mut` appears. Semantic analysis checks
addressability, mutability, lifetime, and conflicting access. A reference is a
safe connection to an existing value, not a raw integer memory address.

## Practice and self-check

For each declaration, decide whether it is valid and identify the stage that
rejects it if it is invalid:

```vortex
let count: u32 = 4;
let grid: [f32; 2 + 2, 3] = [0.0; 4, 3];
let bad: [f32; 2.5] = [0.0; 2.5];
let name: String = 'V';
```

Answers:

1. Valid.
2. Valid; constant evaluation reduces `2 + 2` to `4`.
3. Invalid during constant/type checking because `2.5` is not an integer
   dimension.
4. Invalid during type checking because `'V'` is `char`, not `String`.
