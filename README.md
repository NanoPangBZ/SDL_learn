# SDL3 CMake Hello World

This project builds SDL 3.4.10 directly from the source in `SDL/`. The sample
application uses only SDL's cross-platform API.

## Layout

```text
.vscode/       Cursor and VS Code build/debug configuration (Windows)
build/         Out-of-source build output
SDL/           Vendored SDL 3.4.10 source
src/           Application source
script/        Debug and Release build scripts
CMakeLists.txt Top-level CMake project
```

## Windows requirements

- CMake 3.20 or newer
- MinGW-w64 at `C:\mingw64` with GCC, GDB, and `mingw32-make`
- A MinGW build with the POSIX thread model and zlib support
- Cursor or VS Code with the Microsoft C/C++ extension for F5 debugging

If MinGW is installed elsewhere, set `MINGW_ROOT` before running a batch file.
The `.vscode` files intentionally use `C:\mingw64`, as requested.

## Build on Windows

```bat
script\build_debug.bat
script\build_release.bat
```

The executable is written to `build\debug\bin\sdl_hello.exe` or
`build\release\bin\sdl_hello.exe`. MinGW and SDL dependencies are linked
statically, so no MinGW or SDL DLL needs to be copied beside the executable.
Windows system DLLs such as `kernel32.dll` and `user32.dll` remain normal OS
dependencies and are available on supported Windows installations.

## Build on Linux or macOS

```sh
chmod +x script/build_debug.sh script/build_release.sh
./script/build_debug.sh
./script/build_release.sh
```

The shell scripts use the platform's default CMake generator and C compiler.
SDL selects its native backend during configuration.

## Debug in Cursor or VS Code

Open this directory as the workspace and press `F5`. The launch configuration
runs `script\build_debug.bat` first and then starts the resulting executable
with `C:\mingw64\bin\gdb.exe`.

Close the window or press Escape to exit the sample.
