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

/*#include "Renderer/RenderTarget.h"*/namespace HS { class RenderTarget; } // namespace HS
/*#include "Renderer/RenderPath.h"*/ namespace HS { class RenderPath; }
/*#include "RHI/RHIContext.h"*/ namespace HS { class RHIContext; }
/*#include "Editor/GUI/GUIContext.h"*/ namespace HS { namespace Editor { class GUIContext; } }

HS_NS_EDITOR_BEGIN

class Panel;

class HS_EDITOR_API EditorWindow : public Window
{
public:
    EditorWindow(const char* name, uint32 width, uint32 height, EWindowFlags flags);
    ~EditorWindow() override;

    void Render() override;
    void ProcessEvent() override;

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

    std::vector<RenderTarget> _renderTargets;

    RHIContext* _rhiContext;
    RenderPath* _renderer;
    
    Panel* _basePanel;
    Panel* _menuPanel;
    Panel* _scenePanel;
};

HS_NS_EDITOR_END

#endif /* EditorWindow_h */
