#!/bin/bash
set -e # this is 

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build"
cmake --build "$ROOT_DIR/build"

echo "======= Running sample.vx ======="
"$ROOT_DIR/build/vortex_debug" "$ROOT_DIR/tests/vx/samples/sample.vx"
echo "======= Running tokens.vx ======="
"$ROOT_DIR/build/vortex_debug" "$ROOT_DIR/tests/vx/samples/tokens.vx"
echo "======= Running declarations_and_types.vx ======="
"$ROOT_DIR/build/vortex_debug" "$ROOT_DIR/tests/vx/valid/declarations_and_types.vx"
