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
    GUIRenderer();
    ~GUIRenderer();
    
    bool Initialize(const NativeWindowHandle* nativeHandle) override;
    void NextFrame() override;
    void Render(const RenderParameter& params, RenderTexture* renderTarget) override;
    void Present() override;
    void Shutdown() override;
    
    
};

HS_NS_EDITOR_END

#endif
