#!/bin/bash
set -e # this is 

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build"
cmake --build "$ROOT_DIR/build"

echo "======= Running sample.vx ======="
"$ROOT_DIR/build/vortex_debug" "$ROOT_DIR/tests/sample.vx"
echo "======= Running struct.vx ======="
"$ROOT_DIR/build/vortex_debug" "$ROOT_DIR/tests/struct.vx"
echo "======= Running tokens.vx ======="
"$ROOT_DIR/build/vortex_debug" "$ROOT_DIR/tests/tokens.vx"
