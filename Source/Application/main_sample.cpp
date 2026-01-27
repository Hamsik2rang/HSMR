//
//  main_sample.cpp
//  HSMR
//
//  Sample main file demonstrating the lightweight prototyping framework
//  This file shows how to create a minimal rendering application
//
//  NOTE: This is an example file. To use the framework:
//  1. Replace Client/main.cpp with this code, OR
//  2. Create a new executable target that uses PrototypeApplication
//

#include "Application/PrototypeApplication.h"
#include "Core/Log.h"

using namespace hs;

// Custom application class for your rendering technique
class MyPrototypeApp : public PrototypeApplication
{
protected:
    void OnInit() override
    {
        HS_LOG(info, "Custom prototype app initialized");

        // Add custom initialization here
        // - Load additional resources
        // - Setup custom render passes
        // - Initialize technique-specific data
    }

    void OnShutdown() override
    {
        HS_LOG(info, "Custom prototype app shutting down");

        // Cleanup custom resources
    }

    void OnUpdate(float deltaTime) override
    {
        // Update custom logic here
        // - Animation updates
        // - Simulation steps
        // - Input handling
    }

    void OnRender() override
    {
        // Custom rendering code here
        // This is called after the main scene render
        // - Post-processing effects
        // - Debug visualization
    }

    void OnGUI() override
    {
        // Custom ImGui windows here
        if (ImGui::Begin("My Technique Controls"))
        {
            ImGui::Text("Custom rendering technique controls");

            // Add your technique-specific UI here
            // - Parameter sliders
            // - Toggle buttons
            // - Debug displays

            ImGui::Separator();
            ImGui::Text("Keyboard Shortcuts:");
            ImGui::BulletText("W/E/R - Gizmo mode (Translate/Rotate/Scale)");
            ImGui::BulletText("P - Toggle profiler");
            ImGui::BulletText("G - Toggle gizmo");
            ImGui::BulletText("Left Click - Select object");
            ImGui::BulletText("Right Drag - Orbit camera");
            ImGui::BulletText("Middle Drag - Pan camera");
            ImGui::BulletText("Scroll - Zoom camera");
        }
        ImGui::End();
    }

    void OnSceneLoaded(Scene* scene) override
    {
        HS_LOG(info, "Scene loaded with %zu objects", scene->GetObjectCount());

        // Setup technique-specific scene data
        // - Generate shadow maps
        // - Build acceleration structures
        // - Precompute lighting data
    }
};

// Main entry point
int main(int argc, char** argv)
{
    MyPrototypeApp app;

    // Initialize with config file (optional)
    const char* configPath = nullptr;
    if (argc > 1)
    {
        configPath = argv[1];
    }
    else
    {
        configPath = "Assets/config.json";
    }

    if (!app.Init(configPath))
    {
        HS_LOG(error, "Failed to initialize application");
        return -1;
    }

    // Optionally load a specific scene from command line
    if (argc > 2)
    {
        if (!app.LoadScene(argv[2]))
        {
            HS_LOG(warning, "Failed to load scene: %s", argv[2]);
        }
    }

    // Run the application
    app.Run();

    return 0;
}

/*
Usage Examples:

1. Basic usage (loads default scene from config):
   ./HSMR

2. Custom config file:
   ./HSMR my_config.json

3. Custom config and scene:
   ./HSMR my_config.json my_scene.json

Scene JSON Format:
{
  "name": "Scene Name",
  "camera": { "position": [x,y,z], "target": [x,y,z], "fov": 60, "near": 0.1, "far": 1000 },
  "shaders": [{ "name": "...", "path": "...", "stages": ["vertex", "fragment"] }],
  "textures": [{ "name": "...", "path": "..." }],
  "models": [{ "name": "...", "path": "...", "settings": { "generateNormals": true } }],
  "objects": [{
    "name": "...",
    "model": "model_name",
    "shader": "shader_name",
    "transform": { "position": [x,y,z], "rotation": [rx,ry,rz], "scale": [sx,sy,sz] },
    "material": { "baseColor": [r,g,b,a], "metallic": 0.0, "roughness": 0.5 }
  }]
}

Profiler Features:
- Frame time graph with FPS tracking
- CPU timing for code sections: HS_CPU_TIMER(profiler, "SectionName")
- GPU timing (placeholder): HS_GPU_TIMER(profiler, "RenderPass")
- Timing tables with min/max/average statistics

Gizmo Controls:
- W: Translate mode
- E: Rotate mode
- R: Scale mode
- L: Toggle local/world space
- X: Toggle snap

Camera Controls:
- Right mouse drag: Orbit around target
- Middle mouse drag: Pan
- Scroll wheel: Dolly (zoom)
- WASD: Move camera freely
*/
