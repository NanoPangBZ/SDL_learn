# SDL Learn

一个基于 SDL3 源码的跨平台 C 语言 HelloWorld 工程，用于学习 SDL 本身、CMake 构建和跨平台项目组织方式。

[English](doc/README.en-US.md)

## 简介

本项目直接将 SDL 源码放在 `SDL/` 目录中，通过 CMake 将 SDL 与示例程序一起构建。项目不依赖系统中预先安装的 SDL，便于在不同开发环境中复现构建过程。

示例程序使用 SDL 初始化视频子系统，创建窗口并运行事件循环。程序源码位于 `src/main.c`，使用 C 编写，而不是 C++。

## 主要特点

- 使用 SDL3 源码构建，不依赖外部 SDL 安装。
- 使用 CMake 管理工程，支持 Windows、Linux、macOS 等类 Unix 环境。
- Windows 构建脚本默认使用 MinGW 工具链。
- Windows 下尽量将 SDL、MinGW 运行时和 C++ 运行时静态链接到可执行文件。
- 提供 Debug、Release 构建脚本。
- 支持在 VS Code 或 Cursor 中按 `F5` 自动构建并使用 GDB 调试。

## 目录结构

```text
.
├── .vscode/              # Windows 下的构建任务和 GDB 调试配置
├── build/                # CMake 构建输出目录
├── SDL/                  # SDL3 源码
├── doc/                  # 项目文档
├── script/               # 构建脚本
├── src/                  # 示例程序源码
├── CMakeLists.txt        # CMake 工程配置
└── README.md             # 项目说明
```

## Windows 构建

请确保已安装并配置以下工具：

- CMake
- MinGW-w64，且使用 POSIX 线程模型和 zlib 异常处理模型
- GDB（如果需要调试）

在项目根目录执行：

```bat
script\build_debug.bat
script\build_release.bat
```

构建结果分别位于：

```text
build\debug\bin\sdl_hello.exe
build\release\bin\sdl_hello.exe
```

在 VS Code 或 Cursor 中打开项目后，按 `F5` 会调用 `.vscode/tasks.json` 中的 Debug 构建任务，然后启动 GDB 调试。

## Linux、macOS 及其他类 Unix 环境

请先安装 CMake 和可用的 C 编译器，然后在项目根目录执行：

```sh
./script/build_debug.sh
./script/build_release.sh
```

Unix 构建脚本使用当前环境中的 C 编译器和 CMake，不强制指定 Windows 的 MinGW 工具链。

## 静态链接说明

Windows 脚本会优先使用 MinGW toolchain，并尝试静态链接 SDL、MinGW GCC 运行时、C++ 运行时和 POSIX 线程运行时，使生成的程序尽量减少对目标计算机开发环境的依赖。

Windows 系统 DLL、Windows API 和 UCRT 仍属于操作系统层依赖，不能简单地全部打包进可执行文件。不同 Windows 版本上的兼容性仍应通过实际目标环境验证。

## 进一步阅读

- [中文详细文档](doc/README.zh-CN.md)
- [English documentation](doc/README.en-US.md)

## 许可证

本项目中的 SDL 源码遵循 SDL 自身附带的许可证。具体许可条款请查看 `SDL/LICENSE.txt` 及 SDL 源码中的相关说明。
