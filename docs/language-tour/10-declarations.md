# Declarations

A declaration introduces a name or definition that the rest of a program can
use. Vortex has declarations for functions, structs, local variables,
parameters, and struct fields.

Declarations are checked by the compiler. The compiler records each name, its
scope, and its type before it generates code.

## Function declarations

A function declaration gives a function a name, parameters, an optional return
type, and a body:

```vortex
fn add(left: i32, right: i32) -> i32 {
    return left + right;
}
```

The declaration contains these parts:

- `fn` marks the declaration as a function.
- `add` is the function name.
- `left` and `right` are parameter names.
- `i32` is the declared type of each parameter.
- `-> i32` declares the return type.
- The block after the signature is the function body.

A function with no return value can omit the return type. Vortex treats the
missing type as `void`:

```vortex
fn show_message(message: String) {
    print(message);
}
```

Functions are declared at the top level. A program can call a function after
its declaration has been recorded by the compiler.

Every executable program must contain exactly one `main` function. In v0.1,
`main` takes no parameters and returns `void`:

```vortex
fn main() {
    show_message("ready");
}
```

A function declaration does not run the function. The function runs only when
code calls it, or when the runtime starts the `main` function.

## Struct declarations

A struct declaration creates a user-defined type with named fields:

```vortex
struct Point {
    x: f32,
    y: f32,
}
```

Each field declaration contains a name and a type. The field names must be
unique within the struct.

A struct declaration defines a type. It does not create a value by itself. Use
a struct expression to create a value:

```vortex
let origin = Point {
    x: 0.0,
    y: 0.0,
};
```

A struct expression must provide the fields required by the struct. The
compiler rejects unknown fields, duplicate fields, missing fields, and values
with the wrong type.

Structs are simple value types in v0.1. Inheritance and class declarations are
not part of the language.

## Variable declarations

Use `let` to declare a local variable:

```vortex
fn main() {
    let width = 128;
    let mut total: f32 = f32(width);

    total = total + 1.0;
}
```

A variable declaration can include an optional type annotation:

```vortex
let count: i32 = 10;
let name: String = "vortex";
```

When the type is omitted, the compiler infers it from the initializer:

```vortex
let count = 10;       // i32
let temperature = 21.5; // f32
```

A local variable must have an initializer in v0.1. The compiler cannot infer a
type from a declaration without a starting value:

```vortex
let missing_value; // compile-time error
```

Variables are immutable by default. Add `mut` when the variable must change:

```vortex
let mut counter = 0;
counter += 1;
```

A declaration with no `mut` cannot be the target of an assignment:

```vortex
let limit = 10;
limit = 20; // compile-time error
```

A local variable declaration is also a statement because it appears inside a
block and affects program execution by creating a value.

## Parameter declarations

A parameter declaration names one input to a function and gives it a type:

```vortex
fn scale(value: f32, factor: f32) -> f32 {
    return value * factor;
}
```

`value` and `factor` are parameters. Their types are written in the function
signature and are required in v0.1.

A parameter is available inside the function body. A call supplies an argument
for each parameter:

```vortex
let result = scale(2.0, 3.0);
```

The compiler checks the number and types of arguments against the parameter
list. Parameters are local to their function and cannot be used outside it.

Parameters can use references when a function must read or change an existing
value without copying it:

```vortex
fn scale_values(values: &mut [f32; 4], factor: f32) {
    for index in 0..4 {
        values[index] *= factor;
    }
}
```

The rules for shared and mutable references are described in
[Variables and types](04-variables-and-types.md).

## Field declarations

A field declaration gives a name and type to one value stored in a struct:

```vortex
struct Size {
    width: usize,
    height: usize,
}
```

`width` and `height` are fields of `Size`. A field belongs to its struct type;
it is not a local variable and cannot be used as a standalone name.

Read a field with the field-access operator:

```vortex
let area = size.width * size.height;
```

Field access uses the type of the value on the left side of the dot. The
compiler rejects a field name that does not exist for that type.

## Names and scopes

A declaration makes a name visible in a scope. Vortex uses these scopes:

- top-level declarations are visible in the program's global declaration scope;
- parameters are visible inside their function body;
- local variables are visible from their declaration to the end of their block;
- fields are visible through values of their struct type.

A nested block creates a nested scope:

```vortex
fn main() {
    let outside = 1;

    {
        let inside = 2;
        print(outside);
        print(inside);
    }

    print(outside);
    print(inside); // compile-time error: outside the scope of inside
}
```

Names are case-sensitive. A name cannot start with a number, and names can
contain letters, numbers, and `_`:

```vortex
let item_count = 4;
let ItemCount = 8;
```

These are different names. The compiler rejects duplicate declarations in the
same scope. Shadowing rules for nested scopes are part of the name-resolution
checks and should remain explicit in compiler diagnostics.

## Declaration grammar

The v0.1 grammar defines top-level functions and structs as follows:

```ebnf
program ::=
    { function | struct_definition } ;

function ::=
    "fn", identifier,
    "(", [ parameter_list ], ")",
    [ "->", type ],
    block ;

struct_definition ::=
    "struct", identifier, "{",
    [ struct_field, { ",", struct_field }, [ "," ] ],
    "}" ;

struct_field ::=
    identifier, ":", type ;
```

Local variable declarations are block statements:

```ebnf
variable_declaration ::=
    "let", [ "mut" ], identifier,
    [ ":", type ],
    "=", expression, ";" ;
```

See the complete grammar in [Vortex v0.1 Grammar](../specification/grammar.md).

## Declarations planned for later

The first version keeps declarations small. These features are outside v0.1:

- modules and packages;
- generic declarations;
- traits or interfaces;
- compile-time constants;
- methods declared inside structs;
- class and inheritance declarations.

These features can be added later when their effect on name resolution, types,
and numerical code is defined clearly.
