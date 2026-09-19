#!/usr/bin/env bash

set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${VORTEX_BUILD_DIR:-${project_dir}/build}"

cmake -S "$project_dir" -B "$build_dir"
cmake --build "$build_dir" --target vortex_lexer_tests
cmake -E chdir "$build_dir" ctest --output-on-failure -R '^lexer_tests$'
