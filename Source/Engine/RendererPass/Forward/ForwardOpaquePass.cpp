#include "ForwardOpaquePass.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Core/FileSystem.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/RHI/RenderHandle.h"
#include "Engine/RHI/CommandHandle.h"

#include "Engine/Object/Material.h"

HS_NS_BEGIN

ForwardOpaquePass::ForwardOpaquePass(const char* name, Renderer* renderer, ERenderingOrder renderingOrder)
    : ForwardPass(name, renderer, renderingOrder)
{
    createResourceHandles();
}

ForwardOpaquePass::~ForwardOpaquePass()
{
}

void ForwardOpaquePass::OnBeforeRendering(uint32_t frameIndex)
{
    _frameIndex = frameIndex;
}

void ForwardOpaquePass::Configure(RenderTarget* renderTarget)
{
    _currentRenderTarget = renderTarget;

    const RenderTargetInfo& rtInfo = _currentRenderTarget->GetInfo();

    _renderPassInfo.colorAttachmentCount = 1;
    Attachment ca{};
    ca.format         = rtInfo.colorTextureInfos[0].format;
    ca.clearValue     = ClearValue(0.0f, 0.0f, 0.0f, 1.0f);
    ca.isDepthStencil = false;
    ca.loadAction     = ELoadAction::CLEAR;
    ca.storeAction    = EStoreAction::STORE;
    _renderPassInfo.colorAttachments.push_back(ca);

    if (rtInfo.useDepthStencilTexture)
    {
        Attachment dsa{};
        dsa.format                             = rtInfo.depthStencilInfo.format;
        dsa.clearValue                         = ClearValue(1.0f, 0.0f);
        dsa.isDepthStencil                     = true;
        dsa.loadAction                         = ELoadAction::CLEAR;
        dsa.storeAction                        = EStoreAction::STORE;
        _renderPassInfo.depthStencilAttachment = dsa;
    }
}

void ForwardOpaquePass::Execute(CommandBuffer* commandBuffer, RenderPass* renderPass)
{
    if (nullptr == _gPipeline)
    {
        createPipelineHandles(renderPass);
    }

    Framebuffer* framebuffer = _renderer->GetHandleCache()->GetFramebuffer(renderPass, _currentRenderTarget);

    float debugColor[4]{0.2f, 0.5f, 0.8f, 1.0f};

    commandBuffer->PushDebugMark("Opaque Pass", debugColor);

    commandBuffer->BeginRenderPass(renderPass, framebuffer);

    commandBuffer->BindPipeline(_gPipeline);

    uint32 offsets[1]{0};
    commandBuffer->BindVertexBuffers(&_vertexBuffer, offsets, 1);

    commandBuffer->SetViewport(Viewport{0.0f, 0.0f, static_cast<float>(framebuffer->info.width), static_cast<float>(framebuffer->info.height), 0.0f, 1.0f});

    commandBuffer->SetScissor(0, 0, framebuffer->info.width, framebuffer->info.height);

    commandBuffer->DrawArrays(0, 6, 1);

    commandBuffer->EndRenderPass();

    commandBuffer->PopDebugMark();

    //    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)cmdEncoder;
    //
    //    if (nil == _rhiRes->pipelineState)
    //    {
    //        createPipelineHandles(renderPass);
    //    }
    //
    //    MTLRenderPassDescriptor* rpDesc = (__bridge MTLRenderPassDescriptor*)renderPass->handle;
    //    HS_ASSERT(rpDesc != nil, "RenderPass is null");
    //
    //    [encoder pushDebugGroup:@"Forward - Opaque"];
    //
    //    MTLViewport vp = {0.0, 0.0, static_cast<double>(_currentFramebuffer->info.width), static_cast<double>(_currentFramebuffer->info.height), 0.0, 1.0};
    //
    //    [encoder setViewport:vp];
    //
    //    [encoder setRenderPipelineState:_rhiRes->pipelineState];
    //
    //    [encoder setVertexBuffer:_rhiRes->vertexBuffer offset:0 atIndex:0];
    //
    //    [encoder drawPrimitives:MTLPrimitiveTypeTriangle
    //                vertexStart:0
    //                vertexCount:3];
    //
    //    [encoder popDebugGroup];
}

void ForwardOpaquePass::OnAfterRendering()
{
}

void ForwardOpaquePass::createResourceHandles()
{
    RHIContext* rhiContext = _renderer->GetRHIContext();

    VSINPUT_BASIC vertices[]{
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
            {0.0f, 0.0f, 1.0f, 1.0f},
        },
    };

    _vertexBuffer = rhiContext->CreateBuffer(vertices, sizeof(vertices), EBufferUsage::VERTEX, EBufferMemoryOption::MAPPED);

    std::string libPath = hs_file_get_resource_path(std::string("Shader/MSL/Basic.metal"));
    _vertexShader       = rhiContext->CreateShader(EShaderStage::VERTEX, libPath.c_str(), HS_TO_STRING(VSENTRY_BASIC), strlen(HS_TO_STRING(VSENTRY_BASIC)));
    if (_vertexShader == nullptr)
    {
        HS_LOG(crash, "Shader is nullptr");
    }
    _fragmentShader = rhiContext->CreateShader(EShaderStage::FRAGMENT, libPath.c_str(), HS_TO_STRING(FSENTRY_BASIC), strlen(HS_TO_STRING(FSENTRY_BASIC)));
    if (_fragmentShader == nullptr)
    {
        HS_LOG(crash, "Shader is nullptr");
    }
};

void ForwardOpaquePass::createPipelineHandles(RenderPass* renderPass)
{
    RHIContext* rhiContext = _renderer->GetRHIContext();

    DepthStencilStateDescriptor dsDesc{};
    dsDesc.depthTestEnable  = false;
    dsDesc.depthWriteEnable = false;

    VertexInputStateDescriptor  viDesc{};
    VertexInputLayoutDescriptor viLayout{};
    viLayout.binding       = 0;
    viLayout.stride        = sizeof(VSINPUT_BASIC);
    viLayout.stepRate      = 1;
    viLayout.useInstancing = false;

    viDesc.layouts.push_back(viLayout);

    VertexInputAttributeDescriptor viAttr{};
    viAttr.location   = 0;
    viAttr.binding    = 0;
    viAttr.formatSize = sizeof(float) * 4; // float4 Pos
    viAttr.offset     = 0;

    viDesc.attributes.push_back(viAttr);

    viAttr.location   = 0;
    viAttr.binding    = 0;
    viAttr.formatSize = sizeof(float) * 4; // float4 Color
    viAttr.offset     = sizeof(float) * 4;

    viDesc.attributes.push_back(viAttr);

    ColorBlendStateDescriptor cbDesc{};
    cbDesc.attachments.resize(renderPass->info.colorAttachmentCount);
    for (size_t i = 0; i < renderPass->info.colorAttachmentCount; i++)
    {
        cbDesc.attachments[i].blendEnable = false;
    }

    RasterizerStateDescriptor rsDesc{};
    rsDesc.cullMode                = ECullMode::BACK;
    rsDesc.depthBiasEnable         = false;
    rsDesc.frontFace               = EFrontFace::CCW;
    rsDesc.polygonMode             = EPolygonMode::FILL;
    rsDesc.rasterizerDiscardEnable = false;
    rsDesc.depthClampEnable        = false;

    ShaderProgramDescriptor spDesc{};
    spDesc.vertexShader   = _vertexShader;
    spDesc.fragmentShader = _fragmentShader;

    InputAssemblyStateDescriptor iaDesc{};
    iaDesc.primitiveTopology = EPrimitiveTopology::TRIANGLE_LIST;

    GraphicsPipelineInfo gpInfo{};
    gpInfo.shaderDesc        = spDesc;
    gpInfo.inputAssemblyDesc = iaDesc;
    gpInfo.vertexInputDesc   = viDesc;
    gpInfo.rasterizerDesc    = rsDesc;
    gpInfo.depthStencilDesc  = dsDesc;
    gpInfo.colorBlendDesc    = cbDesc;

    gpInfo.renderPass = renderPass;

    _gPipeline = rhiContext->CreateGraphicsPipeline(gpInfo);

    //    NSError* error = nil;
    //
    ////      rp = hs_rhi_to_render_pass(renderPass);
    //
    //    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
    //    vertexDesc.attributes[0].offset = 0;
    //    vertexDesc.attributes[0].format = MTLVertexFormatFloat4;
    //    vertexDesc.attributes[0].bufferIndex = 0;
    //
    //    vertexDesc.attributes[1].format = MTLVertexFormatFloat4;
    //    vertexDesc.attributes[1].offset = sizeof(vector_float4);
    //    vertexDesc.attributes[1].bufferIndex = 0;
    //
    //    vertexDesc.layouts[0].stride = sizeof(vector_float4) * 2;
    //    vertexDesc.layouts[0].stepRate = 1;
    //    vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    //
    //    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    //    pipelineDesc.label                        = @"ForwardOpaquePass Pipeline";
    //    pipelineDesc.vertexFunction               = _rhiRes->vertexFunction;
    //    pipelineDesc.fragmentFunction             = _rhiRes->fragmentFunction;
    //    pipelineDesc.rasterizationEnabled = true;
    //    pipelineDesc.rasterSampleCount = 1;
    //    pipelineDesc.colorAttachments[0].pixelFormat = hs_rhi_to_pixel_format(renderPass->info.colorAttachments[0].format);
    //    pipelineDesc.vertexDescriptor = vertexDesc;
    //
    //    _rhiRes->pipelineState = [_rhiRes->device newRenderPipelineStateWithDescriptor:pipelineDesc
    //                                                                             error:&error];
    //    if (nil == _rhiRes->pipelineState)
    //    {
    //        HS_LOG(crash, "PipelineState isn't created");
    //    }
}

HS_NS_END
