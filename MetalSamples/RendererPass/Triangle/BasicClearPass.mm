//
//  BasicClearPass.mm
//  MetalSamples
//
//  Created by Yongsik Im on 1/30/25.
//

#include "BasicClearPass.h"
#include "Renderer.h"

id<MTLTexture> _renderTarget;

HSBasicClearPass::HSBasicClearPass(const char* name, HSRenderer* renderer, uint32_t renderingOrder)
    : HSRendererPass(name, renderer, renderingOrder)
{
}

HSBasicClearPass::~HSBasicClearPass()
{
}

void HSBasicClearPass::OnInit()
{
}

void HSBasicClearPass::OnBeforeRendering(uint32_t submitIndex)
{
    _submitIndex = submitIndex;
}

void HSBasicClearPass::Configure(id<MTLTexture> renderTarget)
{
    _renderTarget = renderTarget;
}

void HSBasicClearPass::Execute(id<MTLRenderCommandEncoder> renderEncoder)
{
    
}

void HSBasicClearPass::OnAfterRendering()
{
}

void HSBasicClearPass::OnGUI()
{
    ImGui::Begin("BasicClearPass");
    
    ImGui::Text("It is Basic Clear Pass");
    
    ImGui::End();
}
