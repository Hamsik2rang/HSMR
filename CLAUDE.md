# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

HSMR (High-Speed Modular Renderer) is a C++17-based cross-platform graphics engine currently in active development. The project supports both Vulkan (Windows) and Metal (macOS) rendering APIs with a modular component-based architecture.

**Current Status**: Active development on `feature/refactor-architecture` branch - memory management and architecture improvements in progress.

## Build System

### Prerequisites
- **CMake 3.22.0 or newer**
- **macOS**: Xcode (latest)
- **Windows**: Visual Studio 2022 (or 2019, untested)

### Build Commands

```bash
# Generate build files (from project root)
cmake -S . -B Build

# Build the entire project
cmake --build Build --config Debug
# or for release
cmake --build Build --config Release

# Build specific target
cmake --build Build --target Client --config Debug

# Regenerate CMake (convenience target available)
cmake --build Build --target RegenerateCMake
```

### Build Configuration
- **C++ Standard**: C++17 (required)
- **Build Types**: Debug, Release, MinSizeRel, RelWithDebInfo
- **Architecture Detection**: Automatic (ARM64 on Apple Silicon, x64 on others)
- **Platform Definitions**: `__APPLE__` or `__WINDOWS__`, plus `__ARM64__` or `__X64__`

## Architecture Overview

### Module Dependency Hierarchy
The engine follows a layered architecture with clear dependency flow:

```
Foundation Layer:    HAL → Core
Resource Layer:      Object → ShaderSystem  
Graphics Pipeline:   RHI → Renderer → Engine
Development Tools:   Editor (ImGui) → Client
Specialized Systems: ECS, Physics, Geometry, Animation
```

### Core Modules (13 total)
- **HAL**: Hardware Abstraction Layer (SystemContext, platform detection)
- **Core**: Foundation utilities (math, memory, containers, logging)
- **Object**: Resource management (Image, Mesh, Material, Shader with proxy pattern)
- **ShaderSystem**: Slang/SPIRV-Cross integration for cross-platform shader compilation
- **RHI**: Rendering Hardware Interface (Vulkan on Windows, Metal on macOS)
- **Renderer**: High-level rendering abstractions
- **Engine**: Core engine systems
- **Editor**: ImGui-based development tools
- **Client**: Main application/executable
- **ECS**: Entity Component System
- **Physics**: Physics simulation
- **Geometry**: Geometric utilities
- **Animation**: Animation system (currently disabled in CMake)

### Key Patterns
- **Platform Abstraction**: HAL layer isolates platform-specific code
- **Proxy Pattern**: ObjectManager uses proxy objects for resource management
- **Factory Pattern**: Platform-specific RHI object creation
- **Singleton Pattern**: SystemContext, ObjectManager (appropriate usage)
- **Reference Counting**: Object lifetime management with atomic operations

## Development Guidelines

### Code Organization
- **Headers**: Public interface in module root (e.g., `Source/Core/Math.h`)
- **Implementation**: Private code in `Private/` subdirectories
- **Platform Code**: Platform-specific implementations in dedicated folders (`Metal/`, `Vulkan/`)
- **Namespace**: All code in `HS` namespace (`HS_NS_BEGIN`/`HS_NS_END` macros)

### Memory Management
The project is transitioning from raw pointers to smart pointers:
- **Preferred**: `HS::Scoped<T>` (alias for `std::unique_ptr<T>`)
- **Factory**: `HS::MakeScoped<T>()` (alias for `std::make_unique<T>()`)
- **Legacy**: Some raw `new[]`/`delete[]` operations remain (being refactored)

### API Visibility
The project uses platform-specific visibility macros:
- **macOS**: `__attribute__((__visibility__("default")))`
- **Windows**: `__declspec(dllexport/dllimport)`
- **Modules**: Each module has its own API macro (e.g., `HS_CORE_API`, `HS_RHI_API`)

### Compiler Settings
- **RTTI**: Disabled (`-fno-rtti`)
- **Standards**: C11 for C, C++17 for C++
- **Warnings**: Specific warning suppressions for inline new/delete
- **ARC**: Objective-C ARC disabled for mixed C++/ObjC code

## Development Workflow

### Current Branch Strategy
- **Main Branch**: `main`
- **Feature Branch**: `feature/refactor-architecture` (active development)
- Work should typically be done on feature branches

### Common Build Issues
1. **Missing Dependencies**: Check `Dependency/` folder for platform-specific libraries
2. **Platform Detection**: Verify correct architecture detection in CMake output
3. **Shared Libraries**: Object module builds as shared library (.dylib/.dll)

### Testing
Currently no formal testing framework is configured. Manual testing through Client application.

### Third-Party Dependencies
- **ASSIMP**: Asset loading library
- **STB**: Image processing utilities
- **ImGui**: Immediate mode GUI (for Editor)
- **Slang**: Shader compilation
- **SPIRV-Cross**: Shader cross-compilation
- **Vulkan SDK**: Graphics API (Windows)
- **Metal**: Graphics API (macOS, system framework)

## Key Files to Understand

### Configuration
- `Source/Precompile.h`: Global type definitions, API macros, platform detection
- `CMakeLists.txt`: Root build configuration with platform-specific paths
- Individual `CMakeLists.txt`: Per-module build configuration

### Core Systems
- `Source/HAL/SystemContext.h`: Application context and resource paths
- `Source/Object/ObjectManager.h`: Central resource management system
- `Source/RHI/RHIDefinition.h`: Cross-platform graphics API abstraction definitions

### Platform Abstraction
- `Source/RHI/Metal/` vs `Source/RHI/Vulkan/`: Platform-specific implementations
- Conditional compilation based on platform macros

This engine is designed for extensibility and cross-platform compatibility while maintaining performance through direct API usage rather than abstraction overhead.