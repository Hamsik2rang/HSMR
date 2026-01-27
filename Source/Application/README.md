# HSMR Lightweight Prototyping Framework

A lightweight framework for quickly prototyping and testing rendering techniques with built-in profiling and object manipulation tools.

## Quick Start

### 1. Add Required ThirdParty Libraries

Download and add the following libraries to `Source/ThirdParty/`:

**ImPlot** (for profiler graphs):
- Download from: https://github.com/epezent/implot
- Copy to `Source/ThirdParty/ImPlot/`:
  - `implot.h`
  - `implot_internal.h`
  - `implot.cpp`
  - `implot_items.cpp`

**ImGuizmo** (for 3D gizmo manipulation):
- Download from: https://github.com/CedricGuillemet/ImGuizmo
- Copy to `Source/ThirdParty/ImGuizmo/`:
  - `ImGuizmo.h`
  - `ImGuizmo.cpp`

> Note: The framework works without these libraries but with reduced functionality.

### 2. Build

```bash
cmake -S . -B Build
cmake --build Build --config Debug
```

### 3. Run

```bash
./Build/Debug/HSMR Assets/config.json
```

## Usage

### Creating a Custom Application

```cpp
#include "Application/PrototypeApplication.h"

class MyApp : public hs::PrototypeApplication
{
protected:
    void OnInit() override { /* Custom init */ }
    void OnUpdate(float dt) override { /* Custom update */ }
    void OnRender() override { /* Custom render */ }
    void OnGUI() override { /* Custom ImGui */ }
};

int main()
{
    MyApp app;
    app.Init("Assets/config.json");
    app.LoadScene("Assets/Samples/simple_scene.json");
    app.Run();
    return 0;
}
```

### Scene JSON Format

```json
{
  "name": "My Scene",
  "camera": {
    "position": [0, 2, -5],
    "target": [0, 0, 0],
    "fov": 60,
    "near": 0.1,
    "far": 1000
  },
  "objects": [
    {
      "name": "My Object",
      "model": "cube",
      "transform": {
        "position": [0, 0, 0],
        "rotation": [0, 0, 0],
        "scale": [1, 1, 1]
      }
    }
  ]
}
```

## Features

### Profiler (`Source/Profiler/`)
- Frame time tracking with history graphs
- CPU timing with `HS_CPU_TIMER(profiler, "SectionName")`
- GPU timing support (requires RHI query implementation)
- Min/max/average statistics
- ImPlot integration for visualization

### Gizmo Controller (`Source/Gizmo/`)
- Translate/Rotate/Scale modes (W/E/R keys)
- Local/World space toggle (L key)
- Snap toggle (X key)
- ImGuizmo integration

### Mouse Picker (`Source/Application/`)
- Ray-AABB intersection testing
- Click-to-select objects
- Supports picking through scene hierarchy

### Shader Hot Reload (`Source/Application/`)
- Automatic file change detection
- Configurable check interval
- Callback system for pipeline recreation

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| W | Translate mode |
| E | Rotate mode |
| R | Scale mode |
| L | Toggle local/world space |
| X | Toggle snap |
| P | Toggle profiler |
| G | Toggle gizmo |
| WASD | Move camera |
| Q/E | Camera up/down |

## Camera Controls

| Input | Action |
|-------|--------|
| Right drag | Orbit camera |
| Middle drag | Pan camera |
| Scroll | Zoom camera |
| Left click | Select object |

## Directory Structure

```
Source/
├── Application/          # Main application framework
│   ├── PrototypeApplication.h/cpp
│   ├── Camera.h/cpp
│   ├── Scene.h/cpp
│   ├── SceneObject.h/cpp
│   ├── MousePicker.h/cpp
│   └── ShaderWatcher.h/cpp
├── Profiler/             # Profiling system
│   ├── Profiler.h/cpp
│   └── GPUQuery.h/cpp
├── Gizmo/                # Object manipulation
│   └── GizmoController.h/cpp
└── ThirdParty/           # External libraries
    ├── ImPlot/           # (download required)
    └── ImGuizmo/         # (download required)

Assets/
├── config.json           # Application configuration
└── Samples/              # Sample scene files
    ├── simple_scene.json
    ├── pbr_test.json
    └── shadow_mapping.json
```

## Configuration

`Assets/config.json`:
```json
{
  "window": {
    "title": "HSMR Prototype",
    "width": 1920,
    "height": 1080,
    "vsync": true
  },
  "defaultScene": "Assets/Samples/simple_scene.json"
}
```
