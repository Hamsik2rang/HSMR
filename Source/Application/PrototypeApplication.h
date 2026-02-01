//
//  PrototypeApplication.h
//  HSMR
//
//  Lightweight prototyping application for rendering techniques
//
#ifndef __HS_APPLICATION_PROTOTYPE_APPLICATION_H__
#define __HS_APPLICATION_PROTOTYPE_APPLICATION_H__

#include "Precompile.h"
#include "Camera.h"
#include "Scene.h"
#include "MousePicker.h"

// Forward declarations
namespace hs
{
class Window;
class RHIContext;
class Swapchain;
class RenderPath;
}

HS_NS_BEGIN

// Forward declarations
class Profiler;
class GizmoController;
class ShaderWatcher;

// Application configuration loaded from JSON
struct HS_APPLICATION_API AppConfig
{
    std::string windowTitle = "HSMR Prototype";
    uint32 windowWidth = 1920;
    uint32 windowHeight = 1080;
    bool vsync = true;
    bool fullscreen = false;

    // Default scene to load
    std::string defaultScene;

    // Asset paths
    std::string assetPath = "Assets/";
    std::string shaderPath = "Shader/";
};

// Lightweight prototyping application
class HS_APPLICATION_API PrototypeApplication
{
public:
    PrototypeApplication();
    virtual ~PrototypeApplication();

    // Lifecycle
    bool Init(const char* configPath = nullptr);
    bool LoadScene(const char* scenePath);
    void Run();
    void Shutdown();

    // Access components
    Window* GetWindow() const { return _window; }
    Camera* GetCamera() const { return _camera.get(); }
    Scene* GetScene() const { return _scene.get(); }
//    Profiler* GetProfiler() const { return _profiler.get(); }
    GizmoController* GetGizmo() const { return _gizmo.get(); }

    // Settings
    bool IsProfilerVisible() const { return _showProfiler; }
    void SetProfilerVisible(bool visible) { _showProfiler = visible; }

    bool IsGizmoEnabled() const { return _showGizmo; }
    void SetGizmoEnabled(bool enabled) { _showGizmo = enabled; }

    // Input
    void OnMouseClick(float x, float y, int button);
    void OnMouseMove(float x, float y, float dx, float dy);
    void OnMouseScroll(float delta);
    void OnKeyDown(int key);
    void OnKeyUp(int key);

    // Frame timing
    float GetDeltaTime() const { return _deltaTime; }
    float GetTotalTime() const { return _totalTime; }
    uint64 GetFrameCount() const { return _frameCount; }

protected:
    // Override points for custom behavior
    virtual void OnInit() {}
    virtual void OnShutdown() {}
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnRender() {}
    virtual void OnGUI() {}
    virtual void OnSceneLoaded(Scene* scene) {}

    // Internal update methods
    void updateInput();
    void updateCamera(float deltaTime);
    void renderGUI();

    // Configuration
    bool loadConfig(const char* configPath);

    // Window creation
    bool createWindow();

    // RHI initialization
    bool initRHI();

    // ImGui initialization
    bool initImGui();

private:
    // Configuration
    AppConfig _config;

    // Core components
    Window* _window = nullptr;
    RHIContext* _rhiContext = nullptr;
    Swapchain* _swapchain = nullptr;

    // Application components
    Scoped<Camera> _camera;
    Scoped<Scene> _scene;
    Scoped<MousePicker> _picker;
//    Scoped<Profiler> _profiler;
    Scoped<GizmoController> _gizmo;
//    Scoped<ShaderWatcher> _shaderWatcher;

    // Rendering
    Scoped<RenderPath> _renderer;

    // UI state
    bool _showProfiler = false;
    bool _showGizmo = true;
    bool _showHierarchy = true;
    bool _showInspector = true;

    // Camera control state
    bool _isOrbiting = false;
    bool _isPanning = false;
    glm::vec3 _orbitTarget = glm::vec3(0.0f);
    float _cameraSpeed = 5.0f;

    // Input state
    bool _mouseButtons[3] = { false, false, false };
    float _lastMouseX = 0.0f;
    float _lastMouseY = 0.0f;
    bool _keys[256] = { false };

    // Timing
    float _deltaTime = 0.0f;
    float _totalTime = 0.0f;
    uint64 _frameCount = 0;
    float _lastFrameTime = 0.0f;

    bool _isRunning = false;
};

HS_NS_END

#endif // __HS_APPLICATION_PROTOTYPE_APPLICATION_H__
