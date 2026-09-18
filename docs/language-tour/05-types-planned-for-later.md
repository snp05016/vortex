# Types planned for later

The first version of Vortex should stay small. The following types are useful,
but they can be added after the basic compiler works.

## Slices

A slice is a view into part of an array. Its length is known while the program
is running rather than when it is compiled.

```text
&[T]      shared slice
&mut [T]  mutable slice
```

The slice views existing memory. It does not own or copy the values.

## Vectors

A `Vector<T>` owns a continuous area of memory whose length can grow or shrink
while the program is running:

```vortex
let values: Vector<f32> = Vector::new();
values.push(1.0);
values.push(2.0);
```

Unlike a fixed-size array, a vector does not need to have a length known during
compilation. Use a vector when the amount of data is only known while the
program runs.

## Tensors and matrices

Vortex will need a multidimensional tensor type for its main numerical
workloads. A possible design is:

```vortex
let vector: Tensor<f32, [128]>;
let matrix: Tensor<f32, [128, 256]>;
let batch: Tensor<f32, [16, 128, 256]>;
```

`Matrix` may be a convenient name for a two-dimensional tensor:

```text
Matrix<T, M, N> means Tensor<T, [M, N]>
```

The exact tensor syntax has not been decided yet. Fixed-size arrays are enough
for the first matrix-multiplication program, so the higher-level tensor design
can wait until after v0.1.
