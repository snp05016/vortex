# 3. Programs and declarations

A Vortex source file contains an ordered sequence of top-level declarations.
Vortex v0.1 has two top-level declaration forms: functions and structs.

## 3.1 Program structure

```ebnf
program ::=
    { top_level_declaration } ;

top_level_declaration ::=
      function
    | struct_definition ;
```

Functions and structs may appear in either order. An empty source file is
grammatically valid, although it is not a valid executable program.

Executable statements, local variables, and nested function or struct
declarations are not permitted at top level.

## 3.2 Functions

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

A function declaration preserves:

- the function name and source location;
- parameters in source order, including each parameter name, type, and
  location;
- the return type;
- the body block.

When the return annotation is omitted, the function return type is `void`.

```vortex
fn announce() {
    print("ready");
}

fn add(left: i32, right: i32) -> i32 {
    return left + right;
}
```

Parameter names must be unique within a function. Parameters always have an
explicit type. Default values, variadic parameters, a trailing parameter
comma, function prototypes without bodies, and nested functions are not part
of v0.1.

## 3.3 Entry point

An executable program must define exactly one function named `main`. Its v0.1
signature is:

```vortex
fn main() {
    // program body
}
```

`main` has no parameters and returns `void`, either implicitly or through an
explicit `-> void` annotation. A program with no `main`, more than one `main`,
or a `main` with another signature is rejected during semantic analysis rather
than parsing.

## 3.4 Struct declarations

```ebnf
struct_definition ::=
    "struct", identifier, "{",
    [ struct_field, { ",", struct_field }, [ "," ] ],
    "}" ;

struct_field ::=
    identifier, ":", type ;
```

A struct declaration preserves its name, source location, and ordered field
declarations. Each field preserves its own name, type, and location.

```vortex
struct Point {
    x: f32,
    y: f32,
}
```

An empty struct and a trailing field comma are accepted:

```vortex
struct Marker {}
struct Pair { left: i32, right: i32, }
```

Field names must be unique. Field defaults, methods, visibility modifiers,
inheritance, and generic fields are outside v0.1.

## 3.5 Local variable declarations

Local variables are declaration statements rather than top-level
declarations:

```ebnf
variable_declaration ::=
    "let", [ "mut" ], identifier,
    [ ":", type ],
    "=", expression, ";" ;
```

Every local declaration has an initializer. The type annotation is optional;
when omitted, the type is inferred from the initializer.

```vortex
let width = 128;
let mut total: f32 = 0.0;
```

The declaration preserves the name, `mut` flag, optional written type,
initializer, and complete source location. A declaration without `mut` creates
immutable storage.

## 3.6 Scopes

Vortex v0.1 has these scope categories:

- the program scope contains function and struct names;
- a function scope contains its parameter names;
- each block creates a nested local scope;
- a struct has a field namespace selected through a value of that type.

A local variable becomes visible at its declaration and remains visible until
the end of its block. A loop variable is visible only in the loop body.

Duplicate names in the same scope are invalid. The exact policy for shadowing
an outer local name in a nested block remains a specification decision; code
should not depend on shadowing until that rule is finalized.

## 3.7 Name resolution

The parser preserves identifier spelling without resolving it. Name resolution
must later connect each name expression and named type to one visible
declaration.

The following source parses but fails name resolution:

```vortex
fn main() {
    print(missing_value);
}
```

Name resolution must diagnose unknown names, duplicate declarations, invalid
scope use, and unresolved named types with the relevant source location.

## 3.8 Excluded declaration forms

Vortex v0.1 has no modules, imports, namespaces, global variables, type aliases,
constants, enums, classes, traits, interfaces, generics, methods, overload
sets, or separate function prototypes.

See [Structs](structs.md) for struct value construction and
[Statements](statements.md) for local declaration execution.
