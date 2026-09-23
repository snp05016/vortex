# 6. Statements and control flow

A statement performs an action or changes control flow. Simple statements end
with `;`. Block-shaped statements do not take a trailing semicolon.

## 6.1 Statement forms

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
```

## 6.2 Blocks

A block contains zero or more statements and creates a nested local scope.

```ebnf
block ::=
    "{", { statement }, "}" ;
```

Blocks do not produce values in v0.1.

## 6.3 Variable declarations

```vortex
let width = 128;
let mut total: f32 = 0.0;
```

Every local variable has an initializer. `mut` permits later assignment to its
storage. The optional written type must be compatible with the initializer;
otherwise the type is inferred.

## 6.4 Assignment

Assignment changes mutable storage:

```vortex
total = value;
matrix[row, column] *= scale;
point.x = 0.0;
```

The target grammar permits an identifier followed by any sequence of index or
field suffixes. Arbitrary expressions and literals are not assignment targets.

Supported operators are `=`, `+=`, `-=`, `*=`, `/=`, and `%=`. Compound
assignment requires both an assignable mutable target and a valid underlying
binary operation. The target is evaluated once.

Assignment is not an expression and cannot appear inside another expression.

## 6.5 Expression statements

Any grammatical expression may be followed by `;` and used as a statement.
Calls made for their effects are the common case:

```vortex
print("ready");
calculate();
```

An implementation may warn when a pure value is discarded, but the form
remains valid syntax.

## 6.6 Conditional statements

```ebnf
if_statement ::=
    "if", expression, block,
    [ "else", ( if_statement | block ) ] ;
```

Every condition must have type `bool`. Branch bodies use braces. `else if` is
represented as an `else` branch containing another `if` statement.

```vortex
if score >= 50 {
    print("passed");
} else if score >= 40 {
    print("close");
} else {
    print("try again");
}
```

An `if` statement does not produce a value.

## 6.7 While loops

```ebnf
while_statement ::=
    "while", expression, block ;
```

The condition must have type `bool`. It is evaluated before every iteration,
so the body may execute zero times.

## 6.8 For loops

```ebnf
for_statement ::=
    "for", identifier, "in", expression, block ;
```

The identifier introduces one loop-local variable. The expression after `in`
must be iterable. Vortex v0.1 iteration is specified for integer ranges.

```vortex
for index in 0..4 {
    print(index);
}
```

The loop variable is visible only in the body. C-style `for` syntax and
multiple loop bindings are not accepted.

## 6.9 Break and continue

`break;` exits the nearest enclosing loop. `continue;` begins the next
iteration of the nearest enclosing loop.

Both statements are syntactically recognizable anywhere a statement is
allowed, but semantic analysis must reject either one outside a loop.

## 6.10 Return

```ebnf
return_statement ::=
    "return", [ expression ], ";" ;
```

`return;` is valid in a `void` function. `return expression;` is required to
provide a compatible result from a non-`void` function.

A non-`void` function must return a compatible value on every reachable path.
A `void` function must not return a value. Return legality and path coverage
are semantic and control-flow checks.

## 6.11 Scope and lifetime

A local variable exists from its declaration until execution leaves its block.
An inner block can read visible outer declarations. A name declared inside the
inner block is not visible after that block ends.

The lifetime of a value and the visibility of its name are related but
distinct compiler concepts. Reference checking must prevent a reference from
outliving the storage to which it refers.

## 6.12 Excluded statements

Vortex v0.1 has no `switch`, `match`, `do while`, exception handling, `defer`, unsafe
block, parallel loop, kernel launch, label, or `goto` statement.
