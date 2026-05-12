# HSMR

A cross-platform graphics engine built with C++20

> **WARNING**: This project is currently in active development. It may not build or run correctly on your local environment.

## Overview

HSMR is a modular, cross-platform graphics engine supporting **Vulkan** (Windows) and **Metal** (macOS). The engine features an Entity-Component System, an ImGui-based editor with 3D gizmo manipulation, a runtime shader compilation pipeline, and a forward rendering architecture.

## Prerequisites

- **CMake** 3.22.0 or newer
- **C++20** compatible compiler

### Windows
- Visual Studio 2026(with CMake 4.2.1), 2022 (or 2019, untested)
- Vulkan SDK

### macOS (Apple Silicon)
- Xcode (latest version)

## Build

```bash
# Generate build files
cmake -S . -B Build

# Build (Debug)
cmake --build Build --config Debug

# Build (Release)
cmake --build Build --config Release

# Build a specific target
cmake --build Build --target Client --config Debug
```

The built executable and assets are output to `Build/Debug/` (or `Build/Release/`).

### Editor Launch

```bash
# Simple editor (default)
./Build/Debug/Client

# Advanced editor with full dockspace layout
./Build/Debug/Client -advanced
```

## Third-Party Libraries

| Library | Purpose |
|:--------|:--------|
| [SDL3](https://www.libsdl.org/) | Cross-platform window and input |
| [ImGui](https://github.com/ocornut/imgui) | Immediate-mode GUI |
| [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) | 3D gizmo manipulation |
| [ImPlot](https://github.com/epezent/implot) | Plot/chart widgets |
| [EnTT](https://github.com/skypjack/entt) | Entity-Component System |
| [GLM](https://github.com/g-truc/glm) | Math library |
| [ASSIMP](https://github.com/assimp/assimp) | 3D model loading |
| [STB](https://github.com/nothingismagick/stb) | Image loading |
| [Slang](https://github.com/shader-slang/slang) | Shader compilation |
| [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) | Shader cross-compilation |
| [Tracy](https://github.com/wolfpld/tracy) | Performance profiler |

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
