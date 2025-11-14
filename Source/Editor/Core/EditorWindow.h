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
#include "Engine/Renderer/RendererDefinition.h"
#include "Engine/Renderer/RenderTarget.h"

/*#include "Renderer/RenderTarget.h"*/namespace hs { class RenderTarget; } // namespace hs
/*#include "Renderer/RenderPath.h"*/ namespace hs { class RenderPath; }
/*#include "RHI/RHIContext.h"*/ namespace hs { class RHIContext; }
/*#include "Editor/GUI/GUIContext.h"*/ namespace hs { namespace Editor { class GUIContext; } }
/*#include "Editor/GUI/GUIContext.h"*/ namespace hs { namespace Editor { class Panel; } }
/*#include "Editor/GUI/GUIContext.h"*/ namespace hs { namespace Editor { class EditorCamera; } }

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API EditorWindow : public Window
{
public:
    EditorWindow(Application* ownerApp, const char* name, uint32 width, uint32 height, EWindowFlags flags);
    ~EditorWindow() override;

    GUIContext* GetGUIContext();

private:
    void setupPanels();

    bool onInitialize() override;
    void onNextFrame() override;
    void onUpdate() override;
    void onRender() override;
    void onPresent() override;
    void onShutdown() override;
    void onResize() override;
    void onSuspend() override;
   
    void onRestore() override;

    void onRenderGUI();

    void updateEditorCamera();

    std::vector<RenderTarget> _renderTargets;

    RHIContext* _rhiContext;  // Note: RHIContext is managed by global context, don't own
    Scoped<RenderPath> _renderer;
    
    Scoped<Panel> _basePanel;
    Scoped<Panel> _menuPanel;
    Scoped<Panel> _scenePanel;
    Scoped<Panel> _profilerPanel;
    Scoped<Panel> _hierarchyPanel;

	Scoped<EditorCamera> _editorCamera;
};

HS_NS_EDITOR_END

#endif /* EditorWindow_h */
