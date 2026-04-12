//
//  EditorWindow.h
//  Editor
//
//  Created by Yongsik Im on 2/7/25.
//

#ifndef __HS_EDITOR_WINDOW_H__
#define __HS_EDITOR_WINDOW_H__

#include "Precompile.h"

#include "Engine/Window.h"
#include "Renderer/RendererDefinition.h"
#include "Renderer/RenderTarget.h"

/*#include "Renderer/RenderTarget.h"*/namespace hs { class RenderTarget; } // namespace hs
/*#include "Renderer/Renderer.h"*/ namespace hs { class Renderer; }
/*#include "Editor/Renderer/EditorRenderer.h"*/ namespace hs { class EditorRenderer; }
/*#include "Scene/Scene.h"*/ namespace hs { class Scene; }

/*#include "Editor/GUI/GUIContext.h"*/ namespace hs { namespace editor { class GUIContext; } }
/*#include "Editor/GUI/GUIContext.h"*/ namespace hs { namespace editor { class Panel; } }

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API EditorWindow : public Window
{
public:
    EditorWindow(Application* ownerApp, const char* name, uint32 width, uint32 height, EWindowFlags flags);
    ~EditorWindow() override;

    GUIContext* GetGUIContext();

private:
    void registerPanel(Panel* panel, bool* visibilityBinding = nullptr, Panel* parent = nullptr);
    void cleanupPanels();
    void setupPanels();
    void setupDefaultScene();
    bool loadInitialScene();
    void persistDefaultSceneAsset();

    bool onInitialize() override;
    void onNextFrame() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onPresent() override;
    void onShutdown() override;

    void onRenderGUI();

    void syncEditorCameraToScene();
    void updateSceneCamera(float deltaTime);
    void processShortcuts();

    void setupResources();

    Scoped<Renderer> _renderer;
    Scoped<EditorRenderer> _overlayRenderer;

    Scoped<Panel> _basePanel;
    Scoped<Panel> _menuPanel;
    Scoped<Panel> _scenePanel;
    Scoped<Panel> _gamePanel;
    Scoped<Panel> _sceneStatusPanel;
    Scoped<Panel> _hierarchyPanel;
    Scoped<Panel> _inspectorPanel;
    Scoped<Panel> _resourcePanel;
    Scoped<Panel> _profilerPanel;
    std::vector<Panel*> _registeredPanels;

    Scoped<Scene> _scene;
    std::vector<RenderTarget> _gameRenderTargets;
    std::string _currentScenePath;
};

HS_NS_EDITOR_END

#endif /* EditorWindow_h */
