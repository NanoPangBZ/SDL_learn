# SDL Learn

## Overview

This is a cross-platform SDL3 HelloWorld sample project. It focuses on building SDL from source with CMake and producing a mostly statically linked executable on Windows with MinGW.

## Layout

- `SDL/`: SDL source code
- `src/`: sample application source
- `script/`: build scripts
- `.vscode/`: editor tasks and debug configuration for Windows
- `build/`: build output directory
- `doc/`: supplemental documentation and notes

## Build

### Windows

1. Open the repository root.
2. Run `script\\build_debug.bat` or `script\\build_release.bat`.
3. For debugging, press F5. The task will build the Debug target first, then start GDB.

### Linux / macOS / Other Unix-like Systems

1. Make sure CMake, a C compiler, and SDL build dependencies are installed.
2. Run `script/build_debug.sh` or `script/build_release.sh`.

## Static Linking Notes

- The Windows scripts prefer the MinGW toolchain.
- The build aims to statically link SDL and MinGW runtime dependencies.
- Windows system DLLs and the UCRT remain operating-system dependencies and are not meant to be bundled into the executable.

## More Docs

Further documentation will be added under `doc/`.
