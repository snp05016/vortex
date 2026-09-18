# Syntax Philosophy

Vortex uses familiar ideas from Rust and C++, while leaving room for its own
numerical and CPU/GPU features.

- Vortex is statically typed. Every value has a known type before the program
  runs.
- Vortex uses `fn` for functions and `let` for local variables.
- Vortex can infer the type of a local variable from its starting value.
- Function parameter and return types are written explicitly in v0.1.
- Vortex uses `print` for simple output and debugging.
- Vortex uses `{}` for code blocks.
