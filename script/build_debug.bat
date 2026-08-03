@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "PROJECT_ROOT=%%~fI"
if not defined MINGW_ROOT set "MINGW_ROOT=C:\mingw64"

set "CC=%MINGW_ROOT%\bin\gcc.exe"
set "GXX=%MINGW_ROOT%\bin\g++.exe"
set "MAKE=%MINGW_ROOT%\bin\mingw32-make.exe"

if not exist "%CC%" (
    echo [ERROR] MinGW GCC was not found at "%CC%".
    echo         Set MINGW_ROOT to your POSIX MinGW installation.
    exit /b 1
)
if not exist "%MAKE%" (
    echo [ERROR] mingw32-make was not found at "%MAKE%".
    exit /b 1
)

set "TOOLCHAIN_INFO=%TEMP%\sdl_mingw_check_%RANDOM%.txt"
"%CC%" -v >"%TOOLCHAIN_INFO%" 2>&1
findstr /C:"Thread model: posix" "%TOOLCHAIN_INFO%" >nul
if errorlevel 1 (
    echo [ERROR] "%CC%" is not a POSIX-thread MinGW toolchain.
    del /q "%TOOLCHAIN_INFO%" >nul 2>&1
    exit /b 1
)
findstr /C:"zlib" "%TOOLCHAIN_INFO%" >nul
if errorlevel 1 (
    echo [ERROR] "%CC%" does not report zlib support.
    del /q "%TOOLCHAIN_INFO%" >nul 2>&1
    exit /b 1
)
del /q "%TOOLCHAIN_INFO%" >nul 2>&1

set "PATH=%MINGW_ROOT%\bin;%PATH%"
cmake -S "%PROJECT_ROOT%" -B "%PROJECT_ROOT%\build\debug" ^
    -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_C_COMPILER="%CC%" ^
    -DCMAKE_CXX_COMPILER="%GXX%" ^
    -DCMAKE_MAKE_PROGRAM="%MAKE%"
if errorlevel 1 exit /b 1

cmake --build "%PROJECT_ROOT%\build\debug" --parallel
if errorlevel 1 exit /b 1

echo [OK] Debug executable: "%PROJECT_ROOT%\build\debug\bin\sdl_hello.exe"
