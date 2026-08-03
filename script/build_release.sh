#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECT_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

cmake -S "$PROJECT_ROOT" -B "$PROJECT_ROOT/build/release" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$PROJECT_ROOT/build/release" --parallel

printf '%s\n' "[OK] Release executable: $PROJECT_ROOT/build/release/bin/sdl_hello"
