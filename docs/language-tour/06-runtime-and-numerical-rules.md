# Runtime and numerical rules

Vortex v0.1 uses checked behavior for common mistakes. A runtime error stops the
program and reports what went wrong.

- Accessing an array outside its bounds is a runtime error.
- Dividing an integer by zero is a runtime error.
- Integer overflow is a runtime error.
- A cast that cannot produce a value in the destination type is a runtime error.
- When the compiler can prove one of these errors will happen, it reports a
  compile-time error instead.

`f32` and `f64` follow IEEE 754 floating-point behavior. In v0.1, the compiler
does not reorder floating-point operations in a way that changes their result.
Faster relaxed numerical modes may be added later when their behavior can be
made explicit.
