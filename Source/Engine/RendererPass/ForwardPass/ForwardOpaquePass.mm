#include "ForwardOpaquePass.h"


#include "Engine/Core/EngineContext.h"
#include "Engine/Core/FileSystem.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/RHIUtility.h"
#include "Engine/Renderer/CommandHandle.h"

#import <Metal/Metal.h>

HS_NS_BEGIN

struct VSINPUT_basic
{
    vector_float4 position;
    vector_float4 color;
};

RHI_RESOURCE_BEGIN(ForwardOpaquePass)
id<MTLDevice>               device;

id<MTLBuffer>   vertexBuffer;
id<MTLFunction> vertexFunction;
id<MTLFunction> fragmentFunction;
id<MTLLibrary> library;

id<MTLRenderPipelineState> pipelineState;

RHI_RESOURCE_END(ForwardOpaquePass)

ForwardOpaquePass::ForwardOpaquePass(const char* name, Renderer* renderer, ERenderingOrder renderingOrder)
    : ForwardPass(name, renderer, renderingOrder)
    , _rhiRes(new RHIResource())
{
    _rhiRes->device = (__bridge id<MTLDevice>)renderer->GetDevice();
    createResourceHandles();
}

ForwardOpaquePass::~ForwardOpaquePass()
{
    // ...
    delete _rhiRes;
}

void ForwardOpaquePass::OnBeforeRendering(uint32_t submitIndex)
{
    _submitIndex = submitIndex;
}

void ForwardOpaquePass::Configure(RenderTexture* renderTarget)
{
    _currentRenderTexture = renderTarget;
}

void ForwardOpaquePass::Execute(void* cmdEncoder, RenderPass* renderPass)
{
    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)cmdEncoder;

    if (nil == _rhiRes->pipelineState)
    {
        createPipelineHandles(renderPass);
    }

    MTLRenderPassDescriptor* rpDesc = (__bridge MTLRenderPassDescriptor*)renderPass->handle;
    HS_ASSERT(rpDesc != nil, "RenderPass is null");

    [encoder pushDebugGroup:@"Forward - Opaque"];

    MTLViewport vp = {0.0, 0.0, static_cast<double>(_currentRenderTexture->width), static_cast<double>(_currentRenderTexture->height), 0.0, 1.0};

    [encoder setViewport:vp];

    [encoder setRenderPipelineState:_rhiRes->pipelineState];

    [encoder setVertexBuffer:_rhiRes->vertexBuffer offset:0 atIndex:0];

    [encoder drawPrimitives:MTLPrimitiveTypeTriangle
                vertexStart:0
                vertexCount:3];

    [encoder popDebugGroup];
}

void ForwardOpaquePass::OnAfterRendering()
{
}

void ForwardOpaquePass::createResourceHandles()
{
    @autoreleasepool
    {
        VSINPUT_basic vertices[]{
            {
                {0.5f, -0.5f, 0.0f, 1.0f},
                {1.0f, 0.0f, 0.0f, 1.0f},
            },
            {
                {0.5f, 0.5f, 0.0f, 1.0f},
                {0.0, 1.0f, 0.0f, 1.0f},
            },
            {
                {-0.5f, 0.5f, 0.0f, 1.0f},
                {0.0f, 0.0f, 1.0f, 1.0f}
            }
        };
        
        

        _rhiRes->vertexBuffer = [_rhiRes->device newBufferWithBytes:vertices length:sizeof(vertices) options:MTLStorageModeManaged];

        std::string libPath = hs_file_get_default_shader_library();
        NSError*    error   = nil;
        NSURL* url = [NSURL fileURLWithPath:[NSString stringWithCString:libPath.c_str() encoding:NSUTF8StringEncoding]] ;

        _rhiRes->library = [_rhiRes->device newLibraryWithURL:url error:&error];
        if (nil == _rhiRes->library)
        {
            if(error)
            {
                auto bp = true;
            }
            HS_LOG(crash, "Library is not loaded");
        }

        _rhiRes->vertexFunction   = [_rhiRes->library newFunctionWithName:@"VertexShader_Basic"];
        _rhiRes->fragmentFunction = [_rhiRes->library newFunctionWithName:@"FragmentShader_Basic"];
    }
};

void ForwardOpaquePass::createPipelineHandles(RenderPass* renderPass)
{
    NSError* error = nil;

//      rp = hs_rhi_to_render_pass(renderPass);

    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.label                        = @"ForwardOpaquePass Pipeline";
    pipelineDesc.vertexFunction               = _rhiRes->vertexFunction;
    pipelineDesc.fragmentFunction             = _rhiRes->fragmentFunction;
    pipelineDesc.rasterizationEnabled = true;
    pipelineDesc.rasterSampleCount = 1;
    pipelineDesc.colorAttachments[0].pixelFormat = hs_rhi_to_pixel_format(renderPass->info.colorAttachments[0].format);
    
    _rhiRes->pipelineState = [_rhiRes->device newRenderPipelineStateWithDescriptor:pipelineDesc
                                                                             error:&error];
    if (nil == _rhiRes->pipelineState)
    {
        HS_LOG(crash, "PipelineState isn't created");
    }
}

HS_NS_END
