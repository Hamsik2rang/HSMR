//
//  GUIRenderer.h
//  HSMR
//
//  Created by Yongsik Im on 2/6/25.
//
#ifndef __HS_GUI_RENDERER_H__
#define __HS_GUI_RENDERER_H__

#include "Precompile.h"

#include "Engine/Renderer/Renderer.h"
#include "Editor/GUI/GUIContext.h"
#include "Engine/Renderer/RenderDefinition.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

class GUIRenderer : public Renderer
{
public:
    GUIRenderer(RHIContext* rhIContext);
    ~GUIRenderer();
    
//    void NextFrame(Swapchain* swapchain) override;
    //void Render(const RenderParameter& params, RenderTexture* renderTarget) override;
    
//    void Present(Swapchain* swapchain) override;
    void Shutdown() override;
private:
    
    
};

HS_NS_EDITOR_END

#endif
