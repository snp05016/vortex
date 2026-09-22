# 9. References and mutability

References let a function access an existing value without exposing raw pointer
arithmetic. V0.1 distinguishes shared references from mutable references.

## 9.1 Reference types

```ebnf
reference_type ::=
    "&", [ "mut" ], type ;
```

`&T` is a shared reference to `T`. `&mut T` is a mutable reference to `T`.

```vortex
&Point
&[f32; 4]
&mut [f32; 4]
```

Reference mutability is part of the type.

## 9.2 Reference expressions

Prefix `&` creates a shared reference. Prefix `&mut` creates a mutable
reference.

```vortex
let value = 10;
let shared = &value;

let mut values = [1.0, 2.0, 3.0, 4.0];
scale(&mut values, 2.0);
```

The operand must be addressable storage, such as a variable, field, or indexed
array element. A temporary calculation is not addressable in v0.1.

## 9.3 Local mutability

Bindings are immutable by default:

```vortex
let limit = 10;
```

`mut` permits assignment to the bound storage:

```vortex
let mut count = 0;
count += 1;
```

Mutability does not change the value's base type. It controls whether the
storage associated with the binding can be modified.

## 9.4 Shared references

A shared reference permits reading the referenced value. Mutation through a
shared reference is not allowed.

Multiple shared references may refer to the same value when their lifetimes
are valid.

## 9.5 Mutable references

A mutable reference requires mutable source storage and permits mutation
through that reference.

While a mutable reference is actively used, no conflicting shared or mutable
reference may access the same storage. This prevents overlapping writes and
read-write conflicts in safe code.

```vortex
let value = 10;
let invalid = &mut value;
// semantic error: value is immutable
```

## 9.6 Function parameters

References are especially useful for fixed arrays and structs:

```vortex
fn scale(values: &mut [f32; 4], factor: f32) {
    for index in 0..4 {
        values[index] *= factor;
    }
}
```

The argument must satisfy the parameter's mutability and type requirements.
The callee cannot retain a reference beyond its valid source lifetime.

## 9.7 Lifetimes

A reference must never outlive the storage it references. V0.1 has no written
lifetime syntax. The compiler must enforce the supported reference patterns
through semantic analysis.

The complete ownership and lifetime model is not yet finalized. Until it is,
implementations should support direct, lexically scoped reference use and
reject cases they cannot prove safe rather than guessing.

## 9.8 Aliasing

The minimum v0.1 aliasing rule is:

- any number of shared accesses may coexist;
- one mutable access may exist only when no conflicting access coexists.

The exact analysis required across function calls, nested aggregates, and
separate array elements remains implementation and specification work. A
conservative rejection is preferable to unsound acceptance while that model is
unfinished.

## 9.9 Excluded pointer behavior

V0.1 has no raw pointers, null references, pointer arithmetic, address casts,
manual allocation, manual deallocation, user-visible unsafe blocks, or
reference fields with unbounded lifetimes.
