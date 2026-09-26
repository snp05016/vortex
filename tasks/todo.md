# Expression parser completion

- [x] Implement postfix calls, indexing, and field access.
- [x] Implement binary precedence, associativity, and ranges.
- [x] Correct array and struct construction edge cases.
- [x] Add focused expression parser tests.
- [x] Build, run all tests, and review the diff.

Risk: preserve the user's in-progress declaration, statement, type, token, and sample-file changes.

# Array type parser repair

- [x] Remove the invalid repeat-array type helper.
- [x] Parse array types recursively through `parse_type()`.
- [x] Reject missing dimensions and trailing dimension commas.
- [x] Add focused array-type tests and restore the green build.

# Parser documentation comments

- [x] document every parser and parser-error function in lowercase natural language.
- [x] include a vortex example and one important behavioral note for each function.
- [x] verify comment-only changes preserve formatting, build, and tests.
