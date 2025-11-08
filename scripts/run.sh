#!/usr/bin/env bash
set -euo pipefail
./build/hydro examples/return_ok.hy
./out
echo "exit code: $?"
