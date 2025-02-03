//
//  BasicClearPass.mm
//  MetalSamples
//
//  Created by Yongsik Im on 1/30/25.
//

#include "ForwardOpaquePass.h"
#include "Renderer.h"

id<MTLTexture> _renderTarget;

HSForwardOpaquePass::HSForwardOpaquePass(const char* name, HSRenderer* renderer, uint32_t renderingOrder)
    : HSForwardPass(name, renderer, renderingOrder)
{
}

HSForwardOpaquePass::~HSForwardOpaquePass()
{
}

void HSForwardOpaquePass::Initialize()
{
}

void HSForwardOpaquePass::Finalize()
{
}

void HSForwardOpaquePass::OnBeforeRendering(uint32_t submitIndex)
{
    _submitIndex = submitIndex;
}

void HSForwardOpaquePass::Configure(id<MTLTexture> renderTarget)
{
    _renderTarget = renderTarget;
}

void HSForwardOpaquePass::Execute(id<MTLRenderCommandEncoder> renderEncoder)
{
}

void HSForwardOpaquePass::OnAfterRendering()
{
}
