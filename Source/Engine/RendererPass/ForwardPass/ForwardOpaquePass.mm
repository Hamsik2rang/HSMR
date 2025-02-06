//
//  BasicClearPass.mm
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//

#include "ForwardOpaquePass.h"
#include "Engine/Renderer/Renderer.h"

HS_NS_BEGIN

ForwardOpaquePass::ForwardOpaquePass(const char* name, Renderer* renderer, uint32_t renderingOrder)
    : ForwardPass(name, renderer, renderingOrder)
{
}

ForwardOpaquePass::~ForwardOpaquePass()
{
}

void ForwardOpaquePass::Initialize()
{
}

void ForwardOpaquePass::Finalize()
{
}

void ForwardOpaquePass::OnBeforeRendering(uint32_t submitIndex)
{
    _submitIndex = submitIndex;
}

void ForwardOpaquePass::Configure(id<MTLTexture> renderTarget)
{
    _currentRenderTarget = renderTarget;
}

void ForwardOpaquePass::Execute(id<MTLRenderCommandEncoder> renderEncoder)
{
}

void ForwardOpaquePass::OnAfterRendering()
{
}

HS_NS_END
