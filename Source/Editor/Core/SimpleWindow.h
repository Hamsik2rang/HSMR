#ifndef __HS_EDITOR_SIMPLE_WINDOW_H__
#define __HS_EDITOR_SIMPLE_WINDOW_H__

#include "Precompile.h"

#include "Engine/Window.h"

#include "Editor/Panel/MenuPanel.h"

/*#include "Renderer/Renderer.h"*/ namespace hs { class Renderer; }
/*#include "Scene/Scene.h"*/ namespace hs { class Scene; }
/*#include "Resource/Mesh.h"*/ namespace hs { class Mesh; }
/*#include "Resource/Material.h"*/ namespace hs { class Material; }
/*#include "Editor/GUI/GUIContext.h"*/ namespace hs { namespace editor { class GUIContext; } }

#include "Scene/Entity.h"

#include <vector>

HS_NS_EDITOR_BEGIN

class EditorCamera;

class HS_EDITOR_API SimpleWindow : public Window
{
public:
    SimpleWindow(Application* ownerApp, const char* name, uint32 width, uint32 height, EWindowFlags flags);
    ~SimpleWindow() override;

    GUIContext* GetGUIContext() override;

private:
    bool onInitialize() override;
    void onNextFrame() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onPresent() override;
    void onShutdown() override;
    
    void onRenderGUI();

    void setupDefaultScene();
    void syncEditorCameraToScene();
    void processCameraInput(float deltaTime);
    
    void drawHelperOverlayGUI();
    void drawLightControlGUI();
    
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
    
    Scoped<MenuPanel> _menuPanel;

    // Owns the meshes/materials loaded via the multi-submesh GLTF path
    // (e.g. Sponza). The single-mesh ObjectManager cache covers DamagedHelmet,
    // but multi-submesh models are not cached there yet.
    std::vector<Scoped<Mesh>> _ownedMeshes;
    std::vector<Scoped<Material>> _ownedMaterials;

    Entity _directionalLightEntity;
};

HS_NS_EDITOR_END

#endif // __HS_EDITOR_SIMPLE_WINDOW_H__
