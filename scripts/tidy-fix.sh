#!/usr/bin/env bash
# applies clang-tidy fixes to the given .c/.cpp files (and the project headers
# they include), then checks they still compile. if they don't, the
# files the fixes touched are restored and the script exits 1. files that don't compile beforehand are
# skipped, so a half-written file on save is left alone.
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."
llvm_bin="${LLVM_BIN:-/opt/homebrew/opt/llvm/bin}"

if [ ! -x "$llvm_bin/run-clang-tidy" ] || [ ! -f build/compile_commands.json ]; then
    echo "tidy-fix: skipping (need $llvm_bin/run-clang-tidy and build/compile_commands.json)." >&2
    exit 0
fi

compiles() {
    "$llvm_bin/clang-tidy" -p build --quiet --allow-no-checks -checks='-*' "$1" >/dev/null 2>&1
}

sources=()
for file in "$@"; do
    file="${file#"$PWD"/}"
    case "$file" in
        *.c|*.cc|*.cpp|*.cxx) compiles "$file" && sources+=("$file") ;;
    esac
done
[ ${#sources[@]} -eq 0 ] && exit 0

regex="($(printf '%s|' "${sources[@]}" | sed 's/[.]/\\./g; s/|$//'))$"
tidy() {
    "$llvm_bin/run-clang-tidy" -p build -fix -quiet \
        -clang-tidy-binary "$llvm_bin/clang-tidy" \
        -clang-apply-replacements-binary "$llvm_bin/clang-apply-replacements" \
        "$@" "$regex" >/dev/null 2>&1 || true
}

backup="$(mktemp -d)"
trap 'rm -rf "$backup"' EXIT
cp -R src tests "$backup/"

# checks that edit the same lines run in separate passes, or their fixes collide.
late='misc-const-correctness,readability-else-after-return,readability-braces-around-statements'
tidy -checks="-${late//,/,-}"
tidy -checks='-*,misc-const-correctness,readability-else-after-return'
tidy -checks='-*,readability-braces-around-statements'

for file in "${sources[@]}"; do
    if ! compiles "$file"; then
        # restore only the files the fixes touched.
        (cd "$backup" && find src tests -type f) | while IFS= read -r changed; do
            cmp -s "$backup/$changed" "$changed" || cp "$backup/$changed" "$changed"
        done
        echo "tidy-fix: $file did not compile after the fixes; restored the files they touched." >&2
        exit 1
    fi
done

# clang-tidy only half-formats around its edits; tidy up every file it touched.
formatter="${CLANG_FORMAT:-clang-format}"
(cd "$backup" && find src tests -type f) | while IFS= read -r changed; do
    cmp -s "$backup/$changed" "$changed" || "$formatter" -i --style=file "$changed"
done
