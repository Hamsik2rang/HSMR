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

void HSBasicClearPass::Execute(id<MTLCommandBuffer> cmdBuffer)
{
    static float r = 0.0f;
    static float g = 0.0f;
    static float b = 0.0f;
    
    g += 0.006f;
    b += 0.009f;
    if(g > 1.0f) g = 0.0f;
    if(b > 1.0f) b = 0.0f;
    
    MTLRenderPassDescriptor* rpDesc = _renderer->GetView().currentRenderPassDescriptor;
    
    rpDesc.colorAttachments[0].clearColor = MTLClearColorMake(r, g, b, 1.0);

    // Create a render pass and immediately end encoding, causing the drawable to be cleared
    id<MTLRenderCommandEncoder> renderEncoder = [cmdBuffer renderCommandEncoderWithDescriptor:rpDesc];

    [renderEncoder endEncoding];
}

void HSBasicClearPass::OnAfterRendering()
{
}
