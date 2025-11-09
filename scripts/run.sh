#!/usr/bin/env bash
set -euo pipefail

SRC="${1:-examples/exit_paren.hy}"

cmake -S . -B build >/dev/null
cmake --build build -j >/dev/null

./build/hydro "$SRC"
./out
echo "exit code: $?"
