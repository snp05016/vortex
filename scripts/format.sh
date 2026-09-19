#!/usr/bin/env bash

set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
formatter="${CLANG_FORMAT:-clang-format}"

if ! command -v "$formatter" >/dev/null 2>&1; then
    echo "clang-format was not found. Install it or set CLANG_FORMAT to its path." >&2
    exit 1
fi

find "$project_dir/src" "$project_dir/tests" -type f \
    \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' \
       -o -name '*.h' -o -name '*.hh' -o -name '*.hpp' -o -name '*.hxx' \) \
    -print0 |
    while IFS= read -r -d '' source_file; do
        "$formatter" -i --style=file "$source_file"
    done

echo "Formatted all C and C++ files under src/ and tests/."
