# HSMR

**High-Speed Modular Renderer** - A cross-platform graphics engine built with C++20

> **WARNING**: This project is currently in active development. It may not build or run correctly on your local environment.

## Overview

HSMR is a modular, cross-platform graphics engine supporting **Vulkan** (Windows) and **Metal** (macOS). The engine features an Entity-Component System, an ImGui-based editor with 3D gizmo manipulation, a runtime shader compilation pipeline, and a forward rendering architecture.

## Features

### Rendering
- **Cross-platform RHI** (Rendering Hardware Interface) abstracting Vulkan and Metal
- **Forward rendering pipeline** with render pass abstraction and PSO caching
- **Blinn-Phong lighting** with normal mapping support
- **Lighting system** supporting Directional, Point, and Spot lights
- **Runtime shader compilation** via Slang with automatic reflection
- **Shader reflection** for automated GPU resource layout creation
- **Material system** with texture map slots (diffuse, specular, normal, etc.)

### Scene & ECS
- **EnTT-based Entity-Component System** with hierarchical scene graph
- **Core components**: Transform, MeshRenderer, Camera, Light, Tag
- **Scene serialization** in JSON format (save/load)
- **Hierarchical transforms** with dirty-flag caching and parent-child propagation
- **AABB bounds** with ray intersection testing for mouse picking

### Editor
- **ImGui dockable panel layout** with Hierarchy, Inspector, Scene viewport, Profiler
- **ImGuizmo 3D gizmo** for Translate/Rotate/Scale manipulation (local/world space, snap)
- **Mouse picking** via ray-AABB intersection
- **Entity hierarchy** with search, rename, drag & drop reparenting
- **Component inspectors** for Transform, MeshRenderer, Camera, Light
- **Asset database** with folder scanning and lazy resource loading
- **Tracy profiler** integration for CPU/GPU/Memory profiling

### Asset Pipeline
- **GLTF/GLB model loading** via ASSIMP with PBR material extraction
- **Image loading** (JPG, PNG, TGA) via STB
- **Fallback resources** (procedural cube, sphere, plane, colored test textures)
- **ShaderSystem** with Slang compilation, SPIRV-Cross cross-compilation, and disk caching

## Architecture

```
Foundation:     Platform ──→ Core
Resources:      Resource ──→ ShaderSystem
Graphics:       RHI ──→ Renderer ──→ Engine
ECS:            Scene (EnTT)
Tools:          Editor (ImGui) ──→ Client
```

### Module Overview

| Module | Description |
|:-------|:------------|
| **Platform** | SDL3 window management, platform detection, system context |
| **Core** | Math (GLM), logging, memory, file I/O, containers |
| **RHI** | Vulkan/Metal abstraction (buffers, textures, pipelines, command buffers) |
| **ShaderSystem** | Slang runtime compilation, reflection, SPIRV/Metal output |
| **Resource** | Image, Mesh, Material, Shader, Model resource management |
| **Renderer** | Render passes, pipeline caching, render resource manager |
| **Scene** | ECS (EnTT), scene graph, transform hierarchy, components |
| **Engine** | Core application loop, window management |
| **Editor** | ImGui panels, gizmos, asset browser, profiler |
| **Client** | Entry point and application bootstrapping |

## Prerequisites

- **CMake** 3.22.0 or newer
- **C++20** compatible compiler

### Windows
- Visual Studio 2022 (or 2019, untested)
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
