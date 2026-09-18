# Kernels and parallel execution planned for later

A kernel is a focused function that processes a large amount of numerical data.
It can eventually run on a GPU or another accelerator. Kernels give the compiler
clear opportunities for optimizations such as loop unrolling, tiling, vectorization,
parallel execution, and kernel fusion.

Vortex will add kernels after ordinary CPU functions, arrays, and a correct
matrix multiplication are working. This keeps v0.1 small and achievable.

```vortex
kernel double_values(input: &[f32], output: &mut [f32]) {
    for i in 0..input.len() {
        output[i] = input[i] * 2.0;
    }
}
```
