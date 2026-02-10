# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

## FIRST PRINCIPLE: Learning-First Development

**This is a learning/portfolio project.** The owner is building this engine to:
1. **Learn graphics programming** - Understanding rendering techniques deeply
2. **Understand game engine architecture** - Experiencing design decisions firsthand
3. **Build a portfolio for job applications** - Must be able to explain every implementation decision

### PROTECTED DOMAINS - DO NOT IMPLEMENT

The following areas are **OFF-LIMITS for AI implementation**. If the user requests work in these areas, or if a task would require touching these domains, **REFUSE and recommend the user implement it themselves**.

| Domain | Examples | Reason |
|:-------|:---------|:-------|
| **Rendering Algorithms** | PBR lighting, BRDF, shadow mapping, post-processing effects | Core learning objective |
| **RenderGraph Design** | Pass dependencies, resource lifetime, automatic barrier insertion | Architecture understanding |
| **GPU-Driven Rendering** | Indirect draw, GPU culling, compute shader pipelines | Modern technique learning |
| **Bindless Resources** | Descriptor indexing, buffer device address | Modern technique learning |
| **Multi-threaded Rendering** | Parallel command buffer recording, job system design | Architecture experience |
| **RHI Abstraction Design** | Vulkan/Metal common interface design decisions | Design capability proof |
| **Memory Management Strategy** | GPU memory allocation policies, streaming design | System-level understanding |
| **Synchronization Design** | Semaphore/Fence strategies, frame overlapping | Vulkan core concept |
| **Shader Logic** | HLSL/Slang shader algorithms (not boilerplate) | Frequently asked in interviews |

### Response Template for Protected Domains

When a request touches protected domains, respond with:

```
This falls under a protected learning domain for this project.

**Domain**: [Identify which domain]
**Why it's protected**: [Brief explanation]

I recommend you implement this yourself to maximize learning value.
I can help with:
- Explaining concepts or approaches
- Reviewing your implementation
- Debugging after you've written the code
- Writing boilerplate/support code around your core implementation
```

### ALLOWED AI Tasks

These are fine to delegate to AI:
- Boilerplate code (CMakeLists.txt, includes, forward declarations)
- Compilation/linking error fixes
- Vulkan validation layer error analysis
- API call templates (filling `VkCreateInfo` structs, etc.)
- Utility code (containers, string parsing, file I/O)
- Platform-specific branching
- Code refactoring (without changing design)
- Documentation and comments
- Test code

---

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
Foundation Layer:    Platform → Core
Resource Layer:      Object → ShaderSystem  
Graphics Pipeline:   RHI → Renderer → Engine
Development Tools:   Editor (ImGui) → Client
Specialized Systems: ECS, Physics, Geometry, Animation
```

### Core Modules (13 total)
- **Platform**: Hardware Abstraction Layer (SystemContext, platform detection)
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
- **Platform Abstraction**: Platform layer isolates platform-specific code
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
- `Source/Platform/SystemContext.h`: Application context and resource paths
- `Source/Object/ObjectManager.h`: Central resource management system
- `Source/RHI/RHIDefinition.h`: Cross-platform graphics API abstraction definitions

### Platform Abstraction
- `Source/RHI/Metal/` vs `Source/RHI/Vulkan/`: Platform-specific implementations
- Conditional compilation based on platform macros

This engine is designed for extensibility and cross-platform compatibility while maintaining performance through direct API usage rather than abstraction overhead.

# [AIDevelop Bot Rules]
# Auto-generated by AIDevelop bot — do not remove this section.

## Working Directory Restriction
- You MUST only read, write, and modify files within this project directory.
- Do NOT access or modify files outside of this folder.
- Shell commands must only operate within this directory.

## Allowed Actions
- File operations (read, write, edit, search) — within this folder only
- Shell commands — within this folder only
- Web search and web fetch — allowed
