# Variables and types

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

## Boolean type

A `bool` is either `true` or `false`:

```vortex
let finished: bool = false;
```

A boolean contains one bit of information, but Vortex does not promise that it
will use exactly one bit of memory. The compiler may store it differently when
that is better for the target hardware.

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

## Characters

A `char` stores one Unicode character:

```vortex
let grade: char = 'A';
let symbol: char = 'λ';
```

Character values use single quotes. A character is not always one byte, because
some Unicode characters need more than one byte when stored in memory. A raw
`u8` byte type can be added after v0.1.

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

The repeat count must be a whole number that the compiler knows when it builds
the fixed-size array. In the first version of Vortex, use a number written
directly in the expression:

```vortex
let values = [0.0; 16];
```

Vortex can support named compile-time constants later. Use a `Vector` when the
number of values is only known while the program is running.

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
