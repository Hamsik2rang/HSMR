//
//  BasicClearPass.mm
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//

#include "ForwardOpaquePass.h"
#include "Engine/Renderer/Renderer.h"

#import <Metal/Metal.h>

HS_NS_BEGIN

struct RendererPass::RHIResource{
    id<MTLDevice> device;
    
    
};

ForwardOpaquePass::ForwardOpaquePass(const char* name, Renderer* renderer, ERenderingOrder renderingOrder)
    : ForwardPass(name, renderer, renderingOrder)
{
    createResourceHandles();
}

ForwardOpaquePass::~ForwardOpaquePass()
{
}

void ForwardOpaquePass::OnBeforeRendering(uint32_t submitIndex)
{
    _submitIndex = submitIndex;
}

void ForwardOpaquePass::Configure(RenderTexture* renderTarget)
{
    _currentRenderTexture = renderTarget;
}

void ForwardOpaquePass::Execute(RenderCommandEncoder* renderEncoder)
{
}

void ForwardOpaquePass::OnAfterRendering()
{
}

void ForwardOpaquePass::createResourceHandles()
{
    float vertices[]
    {
        0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 1.0f
    };
    
//    _vertexBuffer = new Buffer(vertices, sizeof(vertices), EBufferUsage::VERTEX, EBufferMemoryOption::MAPPED);
    
    
}

HS_NS_END
