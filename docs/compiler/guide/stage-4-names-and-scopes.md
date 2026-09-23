# 4. Names and scopes

<p class="page-intro">Name resolution connects every name in the program to the one declaration it means. It is the first stage that reads the syntax tree for meaning rather than shape.</p>

The parser left every name as a plain spelling. In `print(width * 2)`, the
tree says there is a name `width`, and nothing more. It does not know whether
`width` was declared, where, or whether the declaration is still in reach at
this point in the program. The parser was told not to care. This stage has to.

Its job is bookkeeping. It walks the tree, notes every place a name is
introduced, and checks every place a name is used against those notes. When
it finishes, each use in the tree is linked to exactly one declaration, or the
compiler has reported why it could not be.

On the [mountain in the overview](index.md#the-shape-of-the-whole-thing),
this is the third step up. It sits between the [parser](stage-3-parser-and-tree.md),
which builds the tree, and [type checking](stage-5-types-and-rules.md), which
needs to know what each name refers to before it can ask what type it has.

## What this stage is for

Name resolution answers one question for every name the program uses: which
declaration does this mean? The [declarations
chapter](../../specification/declarations.md#37-name-resolution) puts it this
way: name resolution "must later connect each name expression and named type
to one visible declaration".

Along the way it rejects the programs where that question has no good answer.
A name that nothing declares is an error. Two declarations with the same name
in the same place are an error, because a use could not choose between them.
A name used outside the region where it exists is an error. And, because the
roadmap puts it here, a program without exactly one valid `main` is an error.

What this stage does not do is ask whether a use makes sense for the thing it
found. `width * 2` could name a boolean. That is a type question, and it waits
for stage 5.

## Words for this stage

name
: A word the programmer chose, such as `width`, `scale` or `Point`. The
  grammar calls it an **identifier**. Vortex names are case-sensitive, so
  `count` and `Count` are different names.

declaration
: A place in the program that introduces a name, such as `let width = 128;`
  or `fn scale(...)`. It says "from here, this name means this thing".

use
: A place in the program where a name appears in order to refer to something
  already declared, such as `width` in `print(width)`.

scope
: The region of source text where a declaration's name can be used. Outside
  its scope, the name means nothing (or means something else).

block
: A pair of braces `{ }` with statements inside. In Vortex every block
  creates a new scope.

nested scope
: A scope inside another scope. Code in the inner scope can see names from
  the outer one; code in the outer one cannot see names declared inside.

visible
: A declaration is visible at a point in the program if its name can be used
  there.

lookup
: Searching for the declaration a use refers to, starting from the scope
  where the use appears.

shadowing
: Declaring a name in an inner scope that is already declared in an outer
  scope, so that inside the inner scope the name means the new declaration.
  Whether Vortex allows this is not yet decided.

duplicate declaration
: Two declarations of the same name in the same scope.

symbol table
: The compiler's record of which names are declared in which scopes, and
  what each one refers to. It is an idea here, not a particular data
  structure.

namespace
: A separate set of names. A struct's field names form their own namespace,
  reached through a value of that struct type, and do not clash with local
  variables.

name resolution
: The stage that links each use to its declaration and reports unknown,
  duplicate and out-of-scope names.

entry point
: The function a program starts running from. In Vortex it is `main`.

name error
: The [diagnostic category](../../specification/diagnostics.md#name-error)
  for a name that is unknown, duplicated in the same scope, or used outside
  its scope, and for a named type that does not resolve.

## Declarations and uses

Every name in a Vortex program appears in one of two roles. Either it is
being declared, or it is being used. Take this program:

```vortex
struct Point {
    x: f32,
    y: f32,
}

fn shift(point: Point, amount: f32) -> Point {
    let moved = Point { x: point.x + amount, y: point.y };
    return moved;
}

fn main() {
    let start = Point { x: 1.0, y: 2.0 };
    let end = shift(start, 0.5);
    print(end.x);
}
```

The declarations are `Point`, its fields `x` and `y`, the functions `shift`
and `main`, the parameters `point` and `amount`, and the locals `moved`,
`start` and `end`. Everything else that looks like a name is a use: `Point`
in the parameter type and in the two struct expressions, `point`, `amount`,
`moved`, `start`, `end`, `shift`, and `print`.

The [language tour](../../language-tour/10-declarations.md#declaration-quick-reference)
lists the five kinds of declaration Vortex v0.1 has. Functions and structs
appear at the top level. Parameters live inside a function's signature. Fields
live inside a struct. Local variables are statements inside a block. The
`for` loop adds a sixth, smaller case: `for index in 0..4` declares `index`.

Not every use is resolved at this stage. `point.x` has two names in it.
`point` is an ordinary use and is resolved here. But `x` can only be found
once the compiler knows that `point` is a `Point`, and that is a fact about
types. The [expressions chapter](../../specification/expressions.md#59-postfix-expressions)
gives that check to type checking. More on this in
[names that wait for types](#names-that-wait-for-types).

## Scopes nest

A **scope** is where a name can be used. The [declarations
chapter](../../specification/declarations.md#36-scopes) lists four kinds:

- the program scope, holding function and struct names;
- a function scope, holding that function's parameter names;
- a block scope, one for every `{ }`, holding the locals declared in it;
- a struct's field namespace, reached through a value of that struct type.

Blocks can sit inside blocks, so scopes form a nest, like boxes inside boxes.
Two rules from the spec decide what is visible where. "A local variable
becomes visible at its declaration and remains visible until the end of its
block." And "A loop variable is visible only in the loop body."

When the compiler meets a use, it looks for the name in the innermost scope
around it. If the name is not there, it looks one scope further out, and so
on, until it reaches the program scope. If it is not there either, the name is
unknown.

```vortex
fn scale(factor: f32) {
    let base = 2.0;
    {
        let local = base * factor;
        print(local);
    }
}

fn main() {
    scale(3.0);
}
```

<figure class="vx-figure">
<svg viewBox="0 0 760 400" role="img" aria-labelledby="walk-title walk-desc">
<title id="walk-title">Looking up the name factor through nested scopes</title>
<desc id="walk-desc">Four nested boxes. The outermost is the program scope holding scale and main. Inside it is the parameter scope of scale holding factor. Inside that is the body block holding base. Innermost is an inner block holding local, which contains the line let local = base * factor. The lookup for factor checks the inner block, then the body block, and finds factor in the parameter scope.</desc>
<rect class="vx-box" x="10" y="10" width="740" height="380"/>
<text class="vx-text" x="24" y="34">Program scope: scale, main</text>
<rect class="vx-box" x="40" y="50" width="680" height="320"/>
<text class="vx-text" x="54" y="74">Parameters of scale: factor</text>
<rect class="vx-box" x="70" y="90" width="620" height="260"/>
<text class="vx-text" x="84" y="114">Body block of scale: base</text>
<rect class="vx-box-strong" x="100" y="130" width="560" height="200"/>
<text class="vx-text" x="114" y="154">Inner block: local</text>
<rect class="vx-box-accent vx-pulse" x="286" y="192" width="52" height="24"/>
<text class="vx-mono" x="140" y="209">let local = base * factor;</text>
<text class="vx-mono" x="140" y="239">print(local);</text>
<text class="vx-text-muted" x="140" y="284">The use of factor is looked up from the inside out.</text>
<text class="vx-text-muted" x="140" y="302">The search stops at the first scope that declares it.</text>
<g class="vx-seq" style="--vx-i: 0; --vx-n: 3">
<text class="vx-text-muted" x="448" y="154">1. inner block: not here</text>
</g>
<g class="vx-seq" style="--vx-i: 1; --vx-n: 3">
<text class="vx-text-muted" x="448" y="114">2. body block: not here</text>
</g>
<g class="vx-seq" style="--vx-i: 2; --vx-n: 3">
<text class="vx-text-accent" x="448" y="74">3. found: parameter factor</text>
</g>
<circle class="vx-dot" r="6">
<animateMotion dur="3.6s" repeatCount="indefinite" path="M312 192 L432 150 L432 110 L432 70" keyPoints="0;0.62;0.62;0.81;0.81;1;1" keyTimes="0;0.25;0.35;0.55;0.65;0.85;1" calcMode="linear"/>
</circle>
</svg>
<figcaption>Figure 1. Scopes as nested boxes. The use of <code>factor</code> is not declared in the inner block or the body block, so the lookup keeps moving outward until it reaches the parameters of <code>scale</code>. If it had reached the program scope and still found nothing, <code>factor</code> would be an unknown name.</figcaption>
</figure>

The same rules explain why a block keeps its variables to itself. The tour's
example:

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

Inside the inner block, the lookup for `outside` moves one box outward and
finds it. After the block closes, its box is gone, and the last line's lookup
for `inside` finds nothing. The roadmap names this as a required check:
"Keep variables inside the blocks where they were declared."

Visible "at its declaration" also means that order matters inside a block. A
local cannot be used on a line above its own `let`:

```vortex
fn main() {
    print(total); // invalid: total is not declared yet at this point
    let total = 10;
}
```

## What the symbol table must answer

The roadmap asks for a symbol table "for functions, variables, parameters,
and struct fields". This guide will not tell you how to build one. What
matters is the questions it must be able to answer, because every check on
this page is one of these questions:

- Is this name declared in this exact scope already? (Needed to catch
  duplicates as each declaration arrives.)
- Walking outward from here, which declaration of this name is the nearest
  visible one? (Needed for every use.)
- What kind of thing is that declaration: a function, a struct, a parameter,
  a local, a loop variable? (A call needs something callable. A written type
  needs a struct.)
- Where in the source is that declaration? (Error messages need to point at
  it, as in "first declared here".)
- For a given struct, which field names does it declare, and in what order?
  (Needed for struct expressions now and field access in stage 5.)

The answer to the second question is the output of this stage. Later stages
should never have to repeat the search. When type checking meets `base` in
`base * factor`, it should be able to go straight to the `let base` that the
use was linked to. The [architecture page](../architecture.md#pass-contracts)
describes this pass's output as "declaration links or symbol information".

## Three ways a name can go wrong

The roadmap asks for tests of "shadowing, duplicate names, and out-of-scope
variables". Those three look similar on the page, and they are easy to mix
up, so it helps to see them side by side.

<figure class="vx-figure">
<svg viewBox="0 0 760 310" role="img" aria-labelledby="three-title three-desc">
<title id="three-title">Unknown name, duplicate name, and shadowing compared</title>
<desc id="three-desc">Three panels. First, a block declares total but the code prints totl, which no scope declares; this is a name error. Second, one block declares count twice; the second declaration is a duplicate in the same scope and is a name error. Third, an outer block declares count and a nested inner block declares count again; this is shadowing, and Vortex has not decided whether it is allowed.</desc>
<text class="vx-text" x="130" y="24" text-anchor="middle">Unknown name</text>
<rect class="vx-box" x="12" y="40" width="236" height="170"/>
<text class="vx-text-muted" x="24" y="60">one block</text>
<rect class="vx-box-bad vx-pulse" x="74" y="106" width="38" height="20"/>
<text class="vx-mono" x="30" y="90">let total = 4;</text>
<text class="vx-mono" x="30" y="120">print(totl);</text>
<text class="vx-text-muted" x="130" y="240" text-anchor="middle">no scope declares totl</text>
<text class="vx-text" x="130" y="262" text-anchor="middle">name error</text>
<text class="vx-text" x="380" y="24" text-anchor="middle">Duplicate name</text>
<rect class="vx-box" x="262" y="40" width="236" height="170"/>
<text class="vx-text-muted" x="274" y="60">one block</text>
<rect class="vx-box-bad vx-pulse" x="308" y="106" width="46" height="20"/>
<text class="vx-mono" x="280" y="90">let count = 1;</text>
<text class="vx-mono" x="280" y="120">let count = 2;</text>
<text class="vx-text-muted" x="380" y="240" text-anchor="middle">same scope, same name</text>
<text class="vx-text" x="380" y="262" text-anchor="middle">name error</text>
<text class="vx-text" x="630" y="24" text-anchor="middle">Shadowing</text>
<rect class="vx-box" x="512" y="40" width="236" height="170"/>
<text class="vx-text-muted" x="524" y="60">outer block</text>
<text class="vx-mono" x="530" y="90">let count = 1;</text>
<rect class="vx-box-strong" x="524" y="106" width="212" height="92"/>
<text class="vx-text-muted" x="536" y="126">inner block</text>
<rect class="vx-box-accent" x="566" y="142" width="46" height="20"/>
<text class="vx-mono" x="540" y="156">let count = 2;</text>
<text class="vx-mono" x="540" y="184">print(count);</text>
<text class="vx-text-muted" x="630" y="240" text-anchor="middle">different scopes, same name</text>
<text class="vx-text-accent" x="630" y="262" text-anchor="middle">not yet decided</text>
</svg>
<figcaption>Figure 2. Two errors and one open question. In the first two panels the dashed box marks what the compiler rejects. In the third, both declarations are legal on their own; whether the inner one may reuse the outer name is a rule Vortex has not written yet.</figcaption>
</figure>

**Unknown name.** A use whose lookup reaches the program scope without
finding a declaration. The spec's own example is `print(missing_value);`
inside `main`. It parses perfectly and fails here. A named type counts too:
the grammar notes that `u64` "parses as a named type, then fails name
resolution", because v0.1 has no `u64` and no struct by that name.

**Duplicate name.** Two declarations of one name in the same scope. The spec
is firm on this one: "Duplicate names in the same scope are invalid." It
applies to every kind of scope. Two parameters called `value` in one
function are rejected. Two fields called `value` in one struct are rejected.
Two top-level functions called `add` are rejected. The error is reported at
the second declaration, since the first was fine when it arrived. The
[diagnostics chapter](../../specification/diagnostics.md#101-required-diagnostic-data)
allows "optional notes pointing to related declarations", and a duplicate is
the clearest case for one: point at the second, and add a note at the first.

**Shadowing.** An inner scope declaring a name that an outer scope already
declared. Neither declaration is a duplicate, because they are in different
scopes. Whether this is allowed is the first of the open decisions below.

## Decisions Vortex has not made yet

The spec leaves several naming rules open. Name resolution cannot be
finished while they are open, because each one changes which programs are
accepted. For each, this section says what the documents currently say and
what the decision has to settle. When a decision is made, it belongs in the
[declarations chapter](../../specification/declarations.md), not only in the
compiler.

### Shadowing an outer local

What the documents say: the [declarations
chapter](../../specification/declarations.md#36-scopes) states that "The exact
policy for shadowing an outer local name in a nested block remains a
specification decision", and that code "should not depend on shadowing until
that rule is finalized". The [tour](../../language-tour/10-declarations.md#names-and-scopes)
says the same.

The [roadmap](../../roadmap.md#milestone-4-names-and-scopes) asks for
"tests for shadowing" in Milestone 4. A test needs an expected result, so
this decision has to be made before the milestone can be called finished.

The decision must settle:

- whether a local in an inner block may reuse the name of a local in an
  outer block;
- if it is allowed, that the inner name hides the outer one until the inner
  block ends, and the outer one is visible again afterwards;
- if it is forbidden, which diagnostic is reported, and whether it is the
  same "duplicate name" error or a separate one that says "this hides an
  outer variable".

### Parameters and the function body

What the documents say: "a function scope contains its parameter names" and
"each block creates a nested local scope". A function body is a block. Read
literally, that makes the body a scope nested inside the parameter scope.

The decision must settle whether this is a duplicate or a case of shadowing:

```vortex
fn describe(value: i32) {
    let value = 10; // duplicate, or shadowing? not yet specified
    print(value);
}
```

The same question arises for a `for` loop. `for index in 0..4 { let index = 1; }`
declares `index` once as the loop variable and once in the body block. If
the parameter list and the top of the body count as one scope, both examples
are duplicates. If they are two scopes, both are shadowing. Vortex has not
said which.

### Using a local inside its own initializer

What the documents say: a local "becomes visible at its declaration". They do
not say whether that point is before or after the initializer.

```vortex
fn main() {
    let total = 5;
    {
        let total = total + 1; // which total does the right side mean?
    }
}
```

If shadowing is forbidden, this is rejected anyway. If it is allowed, the
decision must settle whether the right-hand `total` means the outer one (so
the inner `total` is 6) or the one being declared (which has no value yet,
and must be an error).

### Order of top-level declarations

What the documents say: "Functions and structs may appear in either order",
and the program scope "contains function and struct names". For locals, the
spec says exactly when the name becomes visible. For functions and structs it
does not. The tour says "A program can call a function after its declaration
has been recorded by the compiler", which could mean after it appears in the
file or after the compiler has collected all top-level names.

The "either order" sentence is about the grammar: the parser accepts
functions and structs in any sequence. It does not say whether `main` may
call a function written below it. The decision must settle:

- whether a function may call a function declared later in the file;
- whether a struct field's type, or a parameter's type, may name a struct
  declared later;
- whether a function may call itself, which needs its own name to be visible
  inside its own body;
- whether two functions may call each other.

Until this is decided, write test programs that declare everything before it
is used, and add tests for the other order once the rule exists.

### Where `print` lives

What the documents say: the [tour](../../language-tour/04-variables-and-types.md)
calls `print` "the built-in `print` function", and the [types
chapter](../../specification/types-and-values.md) mentions it among
supported functions. No chapter says which scope holds it. Nothing in any
Vortex program declares it, yet every example uses it, so name resolution
must find it somewhere.

The decision must settle:

- which scope `print` belongs to (for example, an implicit scope outside the
  program scope, or the program scope itself);
- whether a program may declare its own function called `print`, and if so
  whether that is a duplicate or hides the built-in one;
- whether a local variable may be called `print`.

Cast-shaped calls such as `f32(count)` are a different case. `f32` is a
keyword, not a name, so it never needs a lookup. (How the grammar admits a
keyword in that position is an open question noted on the
[parser page](stage-3-parser-and-tree.md#places-where-the-grammar-needs-care).)

### A struct and a function with the same name

What the documents say: the program scope holds both function and struct
names. The [grammar](../../specification/grammar.md#struct-definitions-and-fields)
says "Struct names must not conflict with another declaration in the same
scope." The [structs chapter](../../specification/structs.md#81-declaration)
says a struct declaration "introduces the struct name into program type
scope", a phrase that hints at a separate scope just for types.

Read together, the first two say `struct Point` and `fn Point` clash. The
third leaves room for them not to. The decision must settle whether types
and functions share one set of top-level names, and whether a local variable
may reuse a struct's name.

## Names that wait for types

Some names cannot be resolved by looking outward through scopes, because
which declaration they mean depends on a type.

**Field access.** In `end.x`, the lookup for `end` happens here. The `x` is
looked up in the field namespace of whatever struct `end` turns out to be.
The [expressions chapter](../../specification/expressions.md#59-postfix-expressions)
says "Type checking verifies that the selected field exists on the resolved
object type". Leave field access alone at this stage, apart from resolving
the name on the left of the dot.

**Struct expressions.** In `Point { x: 1.0, y: 2.0 }`, the struct name is
resolved here like any other use, and it must lead to a struct. Once it does,
the field names `x` and `y` can be checked against that struct's declaration
without knowing any other types. The [structs
chapter](../../specification/structs.md#83-struct-construction) says "Name
and type checking" must require that every field appears exactly once and no
unknown field appears. It does not say which of the two stages does it, so
either is acceptable, as long as it is done once.

**Field names never clash with locals.** A field is not a variable. The tour
is explicit that a field "is not placed in local or top-level value scope".
So a struct with a field `x` and a function with a local `x` are unrelated.
Inside one struct, though, field names must be unique:

```vortex
struct Invalid {
    value: i32,
    value: f32, // invalid: duplicate field name
}
```

**The right kind of thing.** Finding a declaration is not always enough. A
written type such as `point: Point` must lead to a struct, since structs are
the only user-defined types in v0.1. A call such as `shift(start, 0.5)` must
lead to something callable. The grammar points out that `Point(0.0, 0.0)` is
"a call, not struct construction", so a struct name followed by `(` is found
by lookup and then rejected because a struct is not a function. Record the
kind of each declaration in this stage so the check is possible, whether it
is reported here or in stage 5.

## Exactly one main

The roadmap puts the entry-point check in this milestone: "Validate that the
program has exactly one valid `main` function." The [entry point
section](../../specification/declarations.md#33-entry-point) defines valid:
exactly one function named `main`, with no parameters, returning `void`
either by leaving out the return type or by writing `-> void`.

```vortex
fn main() {
    print("ready");
}
```

Three kinds of program fail this check. A file with no `main` at all (a file
of helper functions parses fine but is not an executable). A `main` with
parameters or a non-`void` return type. And a file with two functions named
`main`, which is also a plain duplicate in the program scope.

That last case is a small trap. The same mistake breaks two rules, and a
careless compiler reports it twice. Pick one clear message. Note also that
the [diagnostics chapter](../../specification/diagnostics.md#semantic-error)
files "an invalid `main` signature" under semantic errors, not name errors.
That is not a contradiction. The chapter says plainly that category and the
pass that detects the problem "are related but not identical". The check can
run here while its message says what the user needs to know.

## What you need to have

<div class="vx-split" markdown="1">
<div markdown="1">

#### Need to have

- A record of every declaration in every scope: functions, structs,
  parameters, fields, locals, loop variables.
- Scopes that nest exactly as blocks nest, with the loop variable limited to
  the loop body.
- A local that becomes visible at its declaration, not before.
- Every name use and named type linked to one declaration: later stages
  depend on it.
- Unknown-name errors, pointing at the use.
- Duplicate-name errors in every kind of scope, pointing at the second
  declaration with a note at the first.
- Out-of-scope errors, such as a use after the block ends.
- The kind of each declaration recorded, so "not a struct" and "not callable"
  can be checked.
- The `main` check: exactly one, no parameters, `void`.
- A written decision on each open rule above, and tests that follow it.
- Tests pairing each rejected program with its nearest accepted one.

</div>
<div class="vx-not-yet" markdown="1">

#### Not yet

- Types of anything, including which field `end.x` means: stage 5.
- Mutability, such as assigning to a `let` without `mut`: stage 5.
- `break` and `continue` outside loops: stage 5 semantic checks.
- Array dimensions, even when they use names: resolve the names here,
  evaluate them in stage 5.
- How long values live and where they are stored: reference rules are
  stage 5, memory layout is stage 8.
- Modules, imports, overloading, methods, generics: outside v0.1 entirely.

</div>
</div>

## What you do not need yet

The right-hand column is the list. The general rule is to stop at the
declaration. Once a use knows which declaration it means, this stage's job
for that use is done. Anything that needs to know what the declaration
*holds* (its type, whether it can change, how big it is) belongs to a later
stage. The spec keeps two ideas apart that are easy to blur here: "The
lifetime of a value and the visibility of its name are related but distinct
compiler concepts." This stage is about visibility only.

## How you know it is finished

[Milestone 4](../../roadmap.md#milestone-4-names-and-scopes) is complete
"when every name in a program resolves to one known declaration". In
practice:

- every valid example in the spec and the tour resolves with no errors, and
  you can print, for any use, which declaration it was linked to;
- `print(missing_value);` in `main` is rejected with a name error at
  `missing_value`;
- the tour's `print(inside);` after the block is rejected;
- duplicate locals, parameters, fields and top-level functions are each
  rejected, with the second declaration marked;
- a file with no `main`, two `main` functions, `fn main(arguments: String)`
  or `fn main() -> i32` is rejected, once per mistake;
- the shadowing decision is written down and the roadmap's shadowing tests
  follow it;
- the parser tests from [stage 3](stage-3-parser-and-tree.md) still pass, and
  `let value: MissingType = unknown_name + true;` now fails here, with two
  name errors, and not in the parser.

## Traps

**Resolving in the parser.** It feels efficient to check names while the tree
is being built. It breaks the stage boundary the architecture sets, and it
makes top-level order matter whether or not the language decides it should.

**Guessing the open rules.** When the spec is silent, it is tempting to copy
the language you know best. Languages differ here, and more than one choice
is defensible. Vortex's rule must be written in the spec, so that
tests and users can rely on it.

**Treating `.x` as a normal name.** Looking up `x` in the ordinary scopes
will find the wrong thing, or nothing. A field is reached through a value's
type.

**Forgetting the second role of a name.** `Point` in `let p: Point` and
`Point` in `Point { x: 1.0, y: 2.0 }` are both uses of the same struct. Named
types need resolving just as much as names in expressions do.

**Letting one mistake become many.** If `let total = 4 +;` failed to parse,
there is no `total` declaration, and every later use of `total` becomes an
unknown name. That is a cascade from the parser. Decide how this stage treats
declarations the parser could not build, so the user sees the real error
first.

**Reporting at the wrong place.** A duplicate is reported at the second
declaration, not the first. An unknown name is reported at the use, not at
the end of the function. The span is half the message.

## How others teach this stage

**Crafting Interpreters, "Resolving and Binding".** Nystrom adds a separate
pass that runs after parsing and before any code runs, and resolves each
variable once.[^ci-resolve] It looks from the innermost scope outward, just
as in Figure 1. It reports a second declaration of a name in the same local
scope, and reading a local variable inside its own initializer. Read it for
the clearest account of why names are worth a pass of their own. It differs
from Vortex in one large way: Lox's global variables are not tracked by the
resolver and are left to be found at run time. Vortex has no global
variables, and its top-level names are all functions and structs, so the
whole program can be checked before it runs.

**Crafting Interpreters, "Local Variables".** The second implementation
resolves local names while compiling, and allows two variables with the same
name in different scopes, reporting an error only for the same
scope.[^ci-locals] That is one possible answer to Vortex's open shadowing
question, not the answer. The chapter is also a good picture of scope depth:
the global scope, the first block inside it, the block inside that.

**Kaleidoscope, chapter 3.** Kaleidoscope has no separate name pass. Variable
names are looked up while generating code, and "Unknown variable name" is
reported at that point, as are unknown functions and wrong argument
counts.[^kal3] It also refuses to redefine a function that already has a
body. This is fine for a small tutorial language. Vortex does the opposite on
purpose: the roadmap finishes name and type checks (milestones 4 and 5)
before any code generation (milestone 6), because invalid programs must be
"rejected before code generation with clear explanations".

**The Rust compiler.** The rustc development guide says the parser produces
an AST, and that macro expansion, AST validation, name resolution and early
linting take place during the lexing and parsing stage. The AST is then
converted to a different representation, HIR, and type inference and type
checking run on that.[^rustc-overview] So in rustc, too, names are settled
on the tree the parser built, and types come afterwards. Vortex keeps the
same order with far less machinery.

[^ci-resolve]: Robert Nystrom, *Crafting Interpreters*, chapter "Resolving and Binding". <https://craftinginterpreters.com/resolving-and-binding.html>
[^ci-locals]: Robert Nystrom, *Crafting Interpreters*, chapter "Local Variables". <https://craftinginterpreters.com/local-variables.html>
[^kal3]: LLVM Project, "My First Language Frontend with LLVM Tutorial", chapter 3, "Code generation to LLVM IR". <https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl03.html>
[^rustc-overview]: Rust Compiler Development Guide, "Overview of the compiler". <https://rustc-dev-guide.rust-lang.org/overview.html>
