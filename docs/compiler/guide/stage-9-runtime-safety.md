# 9. Runtime safety

<p class="page-intro">Some mistakes depend on values that exist only while the program runs. This stage adds the checks that catch those mistakes at the moment they happen and stop the program with a clear message.</p>

The front end already rejects a great many bad programs. It cannot reject all
of them, because some mistakes depend on numbers nobody knows until the
program runs. A function that divides by its argument is fine when the
argument is 3 and wrong when it is 0. An index that comes from a loop, a
calculation or a caller may be inside the array or outside it.

Vortex makes a strong promise about those cases. The
[conformance chapter](../../specification/conformance.md#15-undefined-behavior)
says v0.1 "does not intentionally expose undefined behavior" to a well-formed
program. Every invalid operation must either be rejected before the program
runs or stop the program through a documented runtime error. Stage 9 is where
that second half of the promise is kept.

The work happens on the right-hand slope of the
[compiler mountain](index.md#the-shape-of-the-whole-thing): in the code the
compiler generates, and in the small runtime library that reports the failure.

## What this stage is for

The roadmap's [Milestone 9](../../roadmap.md#milestone-9-runtime-safety) has
six items: check array bounds unless an access is proven safe, detect integer
division by zero, detect integer overflow, detect invalid numeric casts, stop
the program with a clear runtime error when a check fails, and test every
failure and every successful boundary case.

The milestone is complete "when the runtime behavior matches the language
tour". So the [tour's chapter on runtime and numerical
rules](../../language-tour/06-runtime-and-numerical-rules.md) is the other
half of your checklist. Read it before this page, and again after.

## Words for this stage

compile time
: While the compiler is working on the source, before any executable exists.

run time
: While the finished executable is running.

static rule
: A rule the compiler can check by reading the source, such as a type or a
  mutability rule.

dynamic rule
: A rule about values that exist only at run time, such as "this index is
  inside the array".

runtime check
: A small test the compiler places in the generated code, just before an
  operation that might be invalid, to decide whether it is safe to go ahead.

runtime error
: What happens when a runtime check fails. The program stops and reports what
  went wrong. The diagnostics chapter lists it as its own category.

bounds check
: A runtime check that an array index is at least zero and less than the size
  of its dimension.

integer overflow
: An integer calculation whose true answer does not fit in its type. For
  `i32`, anything above 2,147,483,647 or below -2,147,483,648.

wrapping
: Handling overflow by silently throwing away the high bits, so a number that
  is too big comes back as a small or negative one. Vortex v0.1 does not do
  this.

saturating
: Handling overflow by clamping to the largest or smallest value. Vortex v0.1
  does not do this either.

cast
: An explicit conversion from one numeric type to another, written like a
  call: `f32(count)`, `i32(temperature)`, `u32(value)`.

undefined behavior
: A situation where the rules of a language or machine say nothing at all
  about what happens next. The program may crash, give a wrong answer, or
  appear to work. Vortex v0.1 forbids exposing it.

proven safe
: Known by the compiler, from the source alone, to be unable to fail. Only a
  proof allows a check to be left out.

exit status
: The number a finished program hands back to whatever started it. By common
  convention zero means success.

IEEE 754
: The international standard for floating-point arithmetic. Vortex's `f32`
  and `f64` follow it.

NaN
: Short for "not a number". A special IEEE 754 value produced by operations
  such as zero divided by zero.

infinity
: A special IEEE 754 value produced, for example, by dividing a nonzero number
  by zero.

## One rule, two moments

The diagnostics chapter makes a point worth reading twice. An out-of-bounds
constant index "can be detected during compilation, while an input-dependent
index requires a runtime check. Both violate the same bounds rule"
([10.3](../../specification/diagnostics.md#103-error-phase-versus-category)).
The rule is the same. What changes is when the compiler has enough
information to apply it.

For every operation that could fail, the compiler is in one of three
positions:

1. It can prove the operation will fail. Then it should report a compile-time
   error, as the tour requires for `values[3]` on a three-element array and
   for `u32(-1)`.
2. It can prove the operation will succeed. Then it may leave out the check.
3. It cannot tell. Then the generated code must keep the check.

The third case is the normal one, and the one this stage is really about.
Figure 1 shows the four kinds of failure against the two moments.

<figure class="vx-figure">
<svg viewBox="0 0 760 330" role="img" aria-labelledby="s9-when-title s9-when-desc">
<title id="s9-when-title">Which failures are caught while compiling and which at run time</title>
<desc id="s9-when-desc">A table with four rows: out-of-bounds index, integer division by zero, integer overflow and invalid cast. For each, the middle column shows an example whose values are known while compiling, which becomes a compile-time error, and the right column shows an example whose values are known only at run time, which keeps a runtime check. The rows light up one after another.</desc>
<rect class="vx-box-strong" x="20" y="20" width="200" height="44"/>
<rect class="vx-box-strong" x="220" y="20" width="260" height="44"/>
<rect class="vx-box-strong" x="480" y="20" width="260" height="44"/>
<text class="vx-text" x="34" y="47">Failure</text>
<text class="vx-text" x="234" y="47">Values known while compiling</text>
<text class="vx-text" x="494" y="47">Values known only when running</text>
<g class="vx-seq" style="--vx-i: 0; --vx-n: 4">
<rect class="vx-box" x="20" y="64" width="200" height="62"/>
<rect class="vx-box" x="220" y="64" width="260" height="62"/>
<rect class="vx-box" x="480" y="64" width="260" height="62"/>
<text class="vx-text" x="34" y="100">Index out of bounds</text>
<text class="vx-mono" x="234" y="90">values[3]</text>
<text class="vx-text-accent" x="234" y="112">compile-time error</text>
<text class="vx-mono" x="494" y="90">values[index]</text>
<text class="vx-text-muted" x="494" y="112">bounds check stays in the code</text>
</g>
<g class="vx-seq" style="--vx-i: 1; --vx-n: 4">
<rect class="vx-box" x="20" y="126" width="200" height="62"/>
<rect class="vx-box" x="220" y="126" width="260" height="62"/>
<rect class="vx-box" x="480" y="126" width="260" height="62"/>
<text class="vx-text" x="34" y="162">Division by zero</text>
<text class="vx-mono" x="234" y="152">10 / 0</text>
<text class="vx-text-accent" x="234" y="174">compile-time error</text>
<text class="vx-mono" x="494" y="152">value / divisor</text>
<text class="vx-text-muted" x="494" y="174">zero check stays in the code</text>
</g>
<g class="vx-seq" style="--vx-i: 2; --vx-n: 4">
<rect class="vx-box" x="20" y="188" width="200" height="62"/>
<rect class="vx-box" x="220" y="188" width="260" height="62"/>
<rect class="vx-box" x="480" y="188" width="260" height="62"/>
<text class="vx-text" x="34" y="224">Integer overflow</text>
<text class="vx-mono" x="234" y="214">2147483647 + 1</text>
<text class="vx-text-accent" x="234" y="236">compile-time error</text>
<text class="vx-mono" x="494" y="214">left + right</text>
<text class="vx-text-muted" x="494" y="236">overflow check stays in the code</text>
</g>
<g class="vx-seq" style="--vx-i: 3; --vx-n: 4">
<rect class="vx-box" x="20" y="250" width="200" height="62"/>
<rect class="vx-box" x="220" y="250" width="260" height="62"/>
<rect class="vx-box" x="480" y="250" width="260" height="62"/>
<text class="vx-text" x="34" y="286">Invalid cast</text>
<text class="vx-mono" x="234" y="276">u32(-1)</text>
<text class="vx-text-accent" x="234" y="298">compile-time error</text>
<text class="vx-mono" x="494" y="276">u32(offset)</text>
<text class="vx-text-muted" x="494" y="298">range check stays in the code</text>
</g>
</svg>
<figcaption>Figure 1. The four failures Milestone 9 names, each at the two moments it can be found. In the middle column the values are all written in the source, so the compiler can see the failure coming. In the right column the values come from variables or parameters, so only a runtime check can decide. The <code>values</code> array is assumed to hold three elements.</figcaption>
</figure>

The middle column is not new work for this stage. Proving a failure ahead of
time is the job of constant evaluation and type checking in
[stage 5](stage-5-types-and-rules.md). The right column is new.

## The checks Vortex requires

The [diagnostics chapter](../../specification/diagnostics.md#runtime-error)
lists the runtime checks v0.1 requires: array index out of bounds, integer
division or remainder by zero, checked integer overflow, and a checked cast
whose value "cannot be represented by its destination type". The tour adds
what is *not* allowed instead: "Unchecked overflow, silent wrapping,
saturating casts, and a user-controlled `unsafe` escape hatch are not part of
v0.1."

### Array bounds

Every index expression that cannot be proven safe needs a **bounds check**. The
check must hold in every dimension separately. For a `[f32; 2, 3]`, the row
index must be below 2 and the column index below 3.

<figure class="vx-figure">
<svg viewBox="0 0 760 320" role="img" aria-labelledby="s9-guard-title s9-guard-desc">
<title id="s9-guard-title">A bounds check standing guard in front of an array read</title>
<desc id="s9-guard-desc">An index read, values[index], on a four-element array flows into a check that asks whether index is less than 4. The yes path leads up to reading the element. The no path leads down to a dashed box that stops the program with a runtime error. One dot carries index 2 along the yes path; another carries index 5 along the no path.</desc>
<rect class="vx-box-strong" x="30" y="130" width="160" height="60"/>
<text class="vx-mono" x="110" y="157" text-anchor="middle">values[index]</text>
<text class="vx-text-muted" x="110" y="177" text-anchor="middle">values: [i32; 4]</text>
<line class="vx-line" x1="190" y1="160" x2="242" y2="160"/>
<polygon class="vx-arrowhead" points="250,160 238,154 238,166"/>
<rect class="vx-box-accent vx-pulse" x="250" y="130" width="180" height="60"/>
<text class="vx-text" x="340" y="157" text-anchor="middle">Is index below 4?</text>
<text class="vx-text-muted" x="340" y="177" text-anchor="middle">the bounds check</text>
<path class="vx-line" d="M430 160 L470 160 L470 70 L502 70"/>
<polygon class="vx-arrowhead" points="510,70 498,64 498,76"/>
<path class="vx-line" d="M470 160 L470 255 L502 255"/>
<polygon class="vx-arrowhead" points="510,255 498,249 498,261"/>
<text class="vx-text-muted" x="480" y="112">yes</text>
<text class="vx-text-muted" x="480" y="216">no</text>
<rect class="vx-box" x="510" y="40" width="220" height="60"/>
<text class="vx-text" x="620" y="67" text-anchor="middle">Read the element</text>
<text class="vx-text-muted" x="620" y="87" text-anchor="middle">index 2: the program carries on</text>
<rect class="vx-box-bad" x="510" y="220" width="220" height="70"/>
<text class="vx-text" x="620" y="248" text-anchor="middle">Stop with a runtime error</text>
<text class="vx-text-muted" x="620" y="270" text-anchor="middle">index 5: out of bounds for 4</text>
<circle class="vx-dot" r="6">
<animateMotion dur="6s" repeatCount="indefinite" path="M190 160 L430 160 L470 160 L470 70 L510 70" keyPoints="0;0;1;1" keyTimes="0;0.1;0.45;1" calcMode="linear"/>
</circle>
<circle class="vx-dot" r="6">
<animateMotion dur="6s" repeatCount="indefinite" path="M190 160 L430 160 L470 160 L470 255 L510 255" keyPoints="0;0;0;1;1" keyTimes="0;0.5;0.55;0.9;1" calcMode="linear"/>
</circle>
</svg>
<figcaption>Figure 2. A bounds check guarding one array read. The check runs first. Only if it passes does the read happen. The two dots are two runs of the same code: with index 2 the read goes ahead, and with index 5 the program stops before touching memory it does not own.</figcaption>
</figure>

The index types matter too. The spec requires an index to have "a supported
integer type". If a signed type such as `i32` is accepted as an index, a
negative index is just as out of bounds as one that is too large, and the
check must catch both.

### Integer division and remainder by zero

`/` and `%` on integers need a check that the right-hand side is not zero.
The tour's example is short:

```vortex
fn divide(value: i32, divisor: i32) -> i32 {
    return value / divisor;
    // valid source, but execution fails if divisor is zero
}
```

There is a second failure hiding in signed division. The smallest `i32`,
-2,147,483,648, divided by -1 should give 2,147,483,648, which does not fit in
an `i32`. That is an overflow, and Vortex requires overflow to be caught, so
signed division needs both checks.

### Integer overflow

Every integer operation whose result can fall outside its type needs an
overflow check: addition, subtraction, multiplication, negation, and division
as just described. The compound assignments `+=`, `-=`, `*=`, `/=` and `%=`
are the same operations and need the same checks.

Unsigned types overflow in the downward direction as well. In `u32` or
`usize`, `0 - 1` has no answer, because the true result is negative. Under
Vortex rules that is a runtime error, not a very large number.

The [expressions chapter](../../specification/expressions.md#55-arithmetic-and-bitwise-expressions)
also lists "invalid shift counts" among checked conditions, while the
diagnostics chapter's list of required runtime checks and the roadmap do not
mention shifts. Until the documents agree, treat an invalid shift count as a
checked error, and note the question in your list of open decisions.

### Invalid numeric casts

A cast converts a value to another numeric type. The tour gives the rules
that exist so far: widening an integer keeps its value, converting an integer
to a floating-point type may lose precision for very large values, converting
a floating-point value to an integer rounds toward zero, and "converting a
value that is outside the new type's range causes a checked runtime error".

```vortex
let temperature: f32 = 21.8;
let whole_degrees = i32(temperature); // 21, rounded toward zero

let impossible = u32(-1);
// compile-time error because the invalid conversion is already known
```

The [types chapter](../../specification/types-and-values.md#412-casts-and-conversions)
warns that "a complete numeric conversion matrix is not yet fixed" and that
implementations "must not invent silent lossy conversions". That leaves an
open decision you need to write down: for each pair of v0.1 numeric types,
is the cast allowed, and which values fail at run time? One case to settle
explicitly is a floating-point NaN or infinity converted to an integer. No
integer can represent either, so the natural reading of the tour is that the
cast fails, but the documents do not say so in words.

## Stopping with a clear runtime error

When a check fails, the program must stop. The
[diagnostics chapter](../../specification/diagnostics.md#106-runtime-reporting)
says a runtime failure "must stop the affected program execution and identify
the error category", and that the report should include the operation's
source location "when source-location metadata is available". It also says a
Vortex program cannot catch these failures, because v0.1 has no exception
handling.

That leaves several things for you to decide and document:

- the wording and layout of a runtime error message, ideally matching the
  compile-time diagnostic format from [stage 1](stage-1-source-and-diagnostics.md);
- where the message is written, so that tests can tell it apart from the
  program's normal output;
- the exit status of a program stopped by a runtime error;
- whether anything the program printed before the failure is guaranteed to
  appear.

The last one is easy to overlook. A test that prints three lines and then
divides by zero should see the three lines and then the error. If the output
was sitting in a buffer when the program stopped, the test sees only the
error, and the person debugging sees a program that seems to have died on its
first line.

There is also a choice about where the checking code lives. Ghuloum describes
two approaches in his incremental Scheme compiler: insert an explicit check
in the generated code at every primitive operation, or have the generated code
call safe versions of those operations in the runtime, which do the checks
themselves. He judges the second slower but simpler and less likely to
contain mistakes.[^ghuloum] Either satisfies Vortex. The spec cares that the
check happens and the report is clear, not where the checking code sits.

## When a check can be skipped

The tour's rule is short: "An optimizer may remove a check only after proving
the operation safe." Nothing weaker than a proof will do. Here is a loop where
the proof is easy:

```vortex
fn total(values: &[f32; 4]) -> f32 {
    let mut sum: f32 = 0.0;
    for index in 0..4 {
        sum += values[index];
    }
    return sum;
}
```

The loop variable only ever takes the values 0, 1, 2 and 3, and the array has
four elements, so every access is in bounds. A compiler that works this out
may drop the check. A compiler that does not bother is still correct.

That second point matters for v0.1. The roadmap puts optimization after the
first release, so the simplest honest plan is to keep every check. Proving
things safe is optimization work, and it can come later without changing any
program's meaning.

Do not assume your backend will supply safety for you. If you use LLVM, its
tutorial states plainly that "LLVM IR does not itself guarantee
safety".[^kal10] The LLVM reference says its plain signed division by zero is
undefined behavior, as is the overflow case of the smallest value divided by
-1, and that its array type does not stop an index from running past the end
of the array.[^langref] Every check Vortex needs
must be put into the generated code on purpose, before the operation it
guards.

## Floating point

Floating-point numbers are not on the list of runtime checks. The
[types chapter](../../specification/types-and-values.md#44-floating-point-values)
says their behavior "follows the target's documented IEEE 754 support", and
the tour adds three commitments for v0.1: the compiler does not reorder
floating-point operations in a way that changes their result, there is no
fast-math flag, and floating-point results "are not exact decimal arithmetic".

The no-reordering rule has a real reason behind it. Because every
floating-point result is rounded, the ordinary laws of algebra do not always
hold: David Goldberg shows that `(x + y) + z` and `x + (y + z)` can give quite
different answers.[^goldberg] A compiler that regroups a sum to save time can
therefore change what a program prints.

What about dividing a floating-point number by zero? The required checks name
*integer* division only. Under IEEE 754, as Goldberg explains, zero divided by
zero produces NaN and a nonzero number divided by zero produces
infinity.[^goldberg] The Vortex documents never mention NaN or infinity by
name. The safest plan is to follow IEEE 754 exactly, decide how `print` shows
those special values, and add the question to your open decisions. Goldberg's
paper is long, but its opening sections on rounding and its section on NaN
and infinity are worth an evening.

## What you need to have

<div class="vx-split" markdown="1">
<div markdown="1">

#### Need to have

- A bounds check on every index that is not proven safe, in every dimension: required by Milestone 9 and the arrays chapter.
- Division and remainder checks for zero, and for the smallest signed value divided by -1.
- Overflow checks on integer addition, subtraction, multiplication and negation, including compound assignment and unsigned subtraction.
- Range checks on every cast whose value can fall outside its destination type.
- One runtime error format that names the category and, where available, the source location.
- A documented exit status for a program stopped by a runtime error.
- A written list of open decisions: the conversion matrix, shift counts, NaN and infinity.

</div>
<div class="vx-not-yet" markdown="1">

#### Not yet

- Removing checks by proof: optimization comes after v0.1, and keeping every check is correct.
- Any way for a program to catch or recover from a runtime error: v0.1 has no exception handling.
- Wrapping or saturating arithmetic: the tour says explicit forms "may be added later".
- Fast or relaxed floating-point modes: the tour defers them until their behavior can be made explicit.
- Runtime checks for references: v0.1 enforces reference rules statically, and rejects what it cannot prove.
- Stack traces or detailed crash reports: a category and a location are the requirement.

</div>
</div>

## What you do not need yet

The right-hand column is short on purpose. Almost everything in this stage
is required, because the conformance chapter leaves no room for "we will make
it safe later". The only thing that truly waits is making the checks cheaper.

## How you know it is finished

[Milestone 9](../../roadmap.md#milestone-9-runtime-safety) ends with "Test
each failure and each successful boundary case." A **boundary case** is the
value right at the edge: the last one that works and the first one that
fails. For each check, you want a compiled program on both sides of the edge:

- Index 0 and the last index of every dimension succeed; one past the last
  index fails, and so does a negative index if signed indices are accepted.
- A two-dimensional access with a valid row and an out-of-range column fails,
  even though the flattened position would still be inside the array.
- Division by 1 and by -1 succeed on ordinary values; division by zero fails;
  the smallest `i32` divided by -1 fails.
- `2147483646 + 1` succeeds at run time and `2147483647 + 1` fails, with the
  values arriving through variables or parameters so the compiler cannot fold
  them away.
- `0 - 1` in `u32` fails.
- The largest value that fits a cast's destination succeeds; the next one
  fails.
- Every failing program prints a message with the right category and, where
  available, the right source location, and exits with the documented status.
- Output printed before the failure appears before the error message.
- The same failures written with constant values are rejected at compile
  time, matching the tour's examples.

When the behavior of every one of these matches the
[tour chapter](../../language-tour/06-runtime-and-numerical-rules.md), the
milestone is done.

## Traps

**Checking only the flattened position.** In a `[f32; 2, 3]`, the access
`a[0, 5]` would land on the sixth cell, which exists. It is still out of
bounds, because column 5 does not exist. Check each index against its own
dimension.

**Forgetting the smallest value divided by -1.** It is the one division that
overflows, it has a nonzero divisor, and a zero check will not catch it.

**Forgetting that unsigned numbers can go below zero.** `u32` and `usize`
subtraction is the most common place for this. A size minus one, when the
size is zero, must fail rather than come out enormous.

**Letting the compiler itself overflow.** The constant evaluator calculates
values like `2147483647 + 1` while compiling. If it does that arithmetic
carelessly in the implementation language, the compiler may produce a wrong
constant, or crash, instead of reporting a clean compile-time error.

**Trusting the backend to trap.** Some processors stop on integer division by
zero and some do not. Some backends treat these cases as undefined, which lets
them assume the case never happens. Write the check yourself.

**Skipping a check because it looks obvious.** If the proof is not written
into the compiler, the check stays. "Nobody would index past the end here" is
not a proof.

**Losing the location.** A runtime error that says only "overflow" makes the
programmer search the whole program. Carry the source location of each
checked operation into the report whenever you can.

## How others teach this stage

**Crafting Interpreters.** The chapter "Evaluating Expressions" defines
runtime errors as "failures that the language semantics demand we detect and
report while the program is running".[^ci-eval] In Nystrom's language most
runtime errors are type errors, such as subtracting a string, because types
are only known at run time. In Vortex every type error is caught while
compiling, so the runtime errors that remain are about values, not types.
Nystrom's interpreter reports an error and keeps its interactive session
alive; a compiled Vortex program stops. The chapter also leaves division by
zero as an exercise for the reader, which is a good prompt to think through
the floating-point section above.

**Ghuloum.** Section 3.16, "Error Checking and Safe Primitives", explains why
checks matter at the machine level: an out-of-range vector write quietly
damages other parts of the running system, producing bugs far from their
cause.[^ghuloum] It then describes the two placements of checking code
mentioned above. The Vortex runtime is much smaller than a Scheme system's,
but the argument is the same.

**Kaleidoscope.** The final chapter of the LLVM tutorial lists unsafe things
LLVM IR permits, buffer overruns among them, and says safety has to be built
as a separate layer on top of LLVM.[^kal10] For Vortex, that layer is this
stage.

**The LLVM Language Reference.** If you pick LLVM, read the entries for its
integer division, addition and float-to-integer conversion instructions. They
spell out exactly which inputs are undefined or produce an unusable result,
which is a precise list of the cases Vortex must check before the
instruction runs.[^langref] The reference also describes a family of
arithmetic operations that report whether an overflow occurred, one
well-known way to build overflow checks.[^langref]

**Goldberg.** "What Every Computer Scientist Should Know About Floating-Point
Arithmetic" is the standard background reading on rounding, IEEE 754 special
values, and why floating-point algebra is not school algebra.[^goldberg] Read
it before you write any test that compares floating-point results, which is
exactly what [stage 10](stage-10-matrix-multiplication.md) asks for.

[^ci-eval]: Robert Nystrom, *Crafting Interpreters*, chapter "Evaluating Expressions". <https://craftinginterpreters.com/evaluating-expressions.html>
[^ghuloum]: Abdulaziz Ghuloum, "An Incremental Approach to Compiler Construction", *Proceedings of the 2006 Scheme and Functional Programming Workshop*, University of Chicago Technical Report TR-2006-06, section 3.16, "Error Checking and Safe Primitives". <http://scheme2006.cs.uchicago.edu/11-ghuloum.pdf>
[^kal10]: LLVM Project, "My First Language Frontend with LLVM Tutorial", chapter 10, "Conclusion and other useful LLVM tidbits". <https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl10.html>
[^langref]: LLVM Project, *LLVM Language Reference Manual*, sections on the "sdiv", "add" and "fptosi" instructions, the "Array Type", and the "llvm.sadd.with.overflow" intrinsics. <https://llvm.org/docs/LangRef.html>
[^goldberg]: David Goldberg, "What Every Computer Scientist Should Know About Floating-Point Arithmetic", *ACM Computing Surveys* 23(1), 1991. <https://doi.org/10.1145/103162.103163>
