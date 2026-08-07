# SDL Learn

## 项目说明

这是一个基于 SDL3 的跨平台 HelloWorld 示例工程，重点演示如何用 CMake 构建 SDL 源码，并在 Windows 下通过 MinGW 生成尽量静态链接的可执行文件。

## 目录结构

- `SDL/`：SDL 源码
- `src/`：示例程序源码
- `script/`：构建脚本
- `.vscode/`：Windows 下的编辑器任务和调试配置
- `build/`：构建输出目录
- `doc/`：补充文档和说明

## 构建说明

### Windows

1. 打开仓库根目录。
2. 运行 `script\\build_debug.bat` 或 `script\\build_release.bat`。
3. 调试时直接按 F5，任务会先执行 Debug 构建，再启动 GDB。

### Linux / macOS / 其他类 Unix

1. 确保系统已安装 CMake、C 编译器和 SDL 构建依赖。
2. 运行 `script/build_debug.sh` 或 `script/build_release.sh`。

## 关于静态链接

- Windows 下的脚本会优先使用 MinGW toolchain。
- 构建目标尽量静态链接 SDL 和 MinGW 运行时依赖。
- Windows 系统 DLL 和 UCRT 仍然属于操作系统层依赖，无法也不应强行打包进可执行文件。

## 进一步文档

详细说明会逐步放到 `doc/` 目录下。
