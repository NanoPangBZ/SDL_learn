#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECT_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

cmake -S "$PROJECT_ROOT" -B "$PROJECT_ROOT/build/debug" \
    -DCMAKE_BUILD_TYPE=Debug
cmake --build "$PROJECT_ROOT/build/debug" --parallel

printf '%s\n' "[OK] Debug executable: $PROJECT_ROOT/build/debug/bin/sdl_hello"
