# Compiler tests

C++ unit tests live in `cpp/`. Vortex fixtures live directly in `vx/`.
Name fixtures that must fail parsing `invalid_<case>.vx`; all other `.vx`
files must parse successfully. The fixture runner discovers them automatically.
These are parser expectations: symbol-resolution diagnostics are recorded for
inspection and do not change a parser fixture's expected result.

Run from the repository root:

```sh
cmake -S . -B build
cmake --build build
cmake -E chdir build ctest --output-on-failure
```

The fixture test writes `output/<case>/` at the repository root, containing:

```text
sample.vx
ast.txt
symbols.txt
```

For parse failures, `ast.txt` contains the diagnostic and `symbols.txt` says
resolution was skipped. Output is regenerated on every run and ignored by Git.
The flat `.vx.out` files in `vx/` are historical lexer snapshots.

`./build/vortex_debug tests/vx/sample.vx` writes the same three files under
`output/sample/`, relative to the current directory. Running without an input
writes the built-in examples under `output/`. Reusing a case name replaces its
previous generated files.
