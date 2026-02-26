#ifndef __HS_EDITOR_SIMPLE_WINDOW_H__
#define __HS_EDITOR_SIMPLE_WINDOW_H__

#include "Precompile.h"

#include "Engine/Window.h"

/*#include "Renderer/Renderer.h"*/ namespace hs { class Renderer; }
/*#include "Scene/Scene.h"*/ namespace hs { class Scene; }
/*#include "Editor/GUI/GUIContext.h"*/ namespace hs { namespace editor { class GUIContext; } }

HS_NS_EDITOR_BEGIN

class EditorCamera;

class HS_EDITOR_API SimpleWindow : public Window
{
public:
    SimpleWindow(Application* ownerApp, const char* name, uint32 width, uint32 height, EWindowFlags flags);
    ~SimpleWindow() override;

    GUIContext* GetGUIContext();

private:
    bool onInitialize() override;
    void onNextFrame() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onPresent() override;
    void onShutdown() override;

    void setupDefaultScene();
    void syncEditorCameraToScene();
    void processCameraInput(float deltaTime);
    void renderOverlay();

    Scoped<Renderer> _renderer;
    Scoped<Scene> _scene;
    Scoped<EditorCamera> _camera;

    // Camera input state
    uint16 _lastMouseX = 0;
    uint16 _lastMouseY = 0;
    bool _isMouseTracking = false;
    bool _rightClickActive = false;

    // Camera deceleration state
    float _currentCameraSpeed = 0.0f;
    glm::vec3 _moveDir = glm::vec3(0.0f);
};

HS_NS_EDITOR_END

#endif // __HS_EDITOR_SIMPLE_WINDOW_H__
