#include "RHI/Metal/MetalContext.h"

#include "RHI/Metal/MetalSwapchain.h"
#include "RHI/Metal/MetalUtility.h"
#include "RHI/Metal/MetalRenderHandle.h"
#include "RHI/Metal/MetalCommandHandle.h"
#include "RHI/Metal/MetalResourceHandle.h"

#include "HAL/SystemContext.h"
#include "HAL/FileSystem.h"
#include "HAL/NativeWindow.h"

#include <Metal/Metal.h>

@class HSViewController;

HS_NS_BEGIN

id<MTLDevice> s_device         = nil;
id<MTLCommandQueue> s_cmdQueue = nil; // TODO: Mult-CommandQueue로 변경

bool MetalContext::Initialize()
{
    NSLog(@"System: %@", [NSProcessInfo processInfo]);
    NSLog(@"Available MTL devices: %@", MTLCopyAllDevices());

    s_device   = MTLCreateSystemDefaultDevice();
    s_cmdQueue = [s_device newCommandQueue];

    _device = (__bridge void*)s_device;
}

void MetalContext::Finalize()
{
    //...
}

void MetalContext::Suspend(Swapchain* swapchain)
{
}

void MetalContext::Restore(Swapchain* swapchain)
{
}

uint32 MetalContext::AcquireNextImage(Swapchain* swapchain)
{
    SwapchainMetal* swMetal = static_cast<SwapchainMetal*>(swapchain);

    const uint32 maxFrameCount = swMetal->_maxFrameCount;
    swMetal->_frameIndex       = (swMetal->_frameIndex + 1) % maxFrameCount;

    auto nativeWindow    = swapchain->GetInfo().nativeWindow;
    HSViewController* vc = (HSViewController*)[(__bridge NSWindow*)(nativeWindow->handle) delegate];
    NSView* view         = [vc view];
    CAMetalLayer* layer  = (CAMetalLayer*)[[vc view] layer];

    id<CAMetalDrawable> drawable = [layer nextDrawable];
    swMetal->_drawable           = drawable;

    MTLRenderPassDescriptor* rpDesc        = [MTLRenderPassDescriptor renderPassDescriptor];
    rpDesc.colorAttachments[0].clearColor  = MTLClearColorMake(0.2f, 0.2f, 0.2f, 1.0f);
    rpDesc.colorAttachments[0].texture     = drawable.texture;
    rpDesc.colorAttachments[0].loadAction  = MTLLoadActionClear;
    rpDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

    MetalRenderPass* swMetalRenderPass = static_cast<MetalRenderPass*>(swMetal->GetRenderPass());
    swMetalRenderPass->handle          = rpDesc;
}

Swapchain* MetalContext::CreateSwapchain(SwapchainInfo info)
{
    SwapchainMetal* swMetal = new SwapchainMetal(info);

    return static_cast<Swapchain*>(swMetal);
}

void MetalContext::DestroySwapchain(Swapchain* swapchain)
{
    SwapchainMetal* swMetal = static_cast<SwapchainMetal*>(swapchain);
    // Swapchain에 들어간 view, layer등은 모두 reference이기 때문에 별도로 제거할 필요가 없다.
    swMetal->nativeHandle   = nullptr;
    delete swMetal;
}

RHIRenderPass* MetalContext::CreateRenderPass(const char* name, const RenderPassInfo& info)
{
    MetalRenderPass* rpMetal = new MetalRenderPass(name, info);

    return static_cast<RHIRenderPass*>(rpMetal);
}

void MetalContext::DestroyRenderPass(RHIRenderPass* renderPass)
{
    MetalRenderPass* rpMetal = static_cast<MetalRenderPass*>(renderPass);

    delete rpMetal;
}

RHIFramebuffer* MetalContext::CreateFramebuffer(const char* name, const FramebufferInfo& info)
{
    MetalFramebuffer* fbMetal = new MetalFramebuffer(name, info);

    if (info.isSwapchainFramebuffer)
    {
        //...
    }

    return static_cast<RHIFramebuffer*>(fbMetal);
}

void MetalContext::DestroyFramebuffer(RHIFramebuffer* framebuffer)
{
    MetalFramebuffer* fbMetal = static_cast<MetalFramebuffer*>(framebuffer);

    delete fbMetal;
}

RHIGraphicsPipeline* MetalContext::CreateGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info)
{
    MetalGraphicsPipeline* pipelineMetal = new MetalGraphicsPipeline(name, info);

    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.label                        = @"Graphics Pipeline";
    pipelineDesc.rasterizationEnabled         = true;

    for (const auto& shader : info.shaderDesc.stages)
    {
        switch (shader->info.stage)
        {
            case EShaderStage::VERTEX:
                pipelineDesc.vertexFunction = static_cast<MetalShader*>(shader)->handle;
            case EShaderStage::FRAGMENT:
                pipelineDesc.fragmentFunction = static_cast<MetalShader*>(shader)->handle;
            default:
                HS_LOG(crash, "Not supported yet");
        }
    }

    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];

    for (size_t i = 0; i < info.vertexInputDesc.attributes.size(); i++)
    {
        const auto& curAttribute = info.vertexInputDesc.attributes[i];

        vertexDesc.attributes[i].offset      = curAttribute.offset;
        vertexDesc.attributes[i].bufferIndex = curAttribute.binding;
        vertexDesc.attributes[i].format      = hs_rhi_to_vertex_format(curAttribute.format);
    }

    for (size_t i = 0; i < info.vertexInputDesc.layouts.size(); i++)
    {
        const auto& curLayout = info.vertexInputDesc.layouts[i];

        vertexDesc.layouts[i].stride       = curLayout.stride;
        vertexDesc.layouts[i].stepRate     = static_cast<uint8>(curLayout.stepRate);
        vertexDesc.layouts[i].stepFunction = curLayout.useInstancing ? MTLVertexStepFunctionPerInstance : MTLVertexStepFunctionPerVertex;
    }

    pipelineDesc.vertexDescriptor = vertexDesc;

    for (size_t i = 0; i < info.renderPass->info.colorAttachmentCount; i++)
    {
        const Attachment& attachment = info.renderPass->info.colorAttachments[i];

        MTLRenderPipelineColorAttachmentDescriptor* colorDesc = pipelineDesc.colorAttachments[i];

        colorDesc.pixelFormat                 = hs_rhi_to_pixel_format(attachment.format);
        colorDesc.blendingEnabled             = info.colorBlendDesc.attachments[i].blendEnable;
        colorDesc.sourceRGBBlendFactor        = hs_rhi_to_blend_factor(info.colorBlendDesc.attachments[i].srcColorFactor);
        colorDesc.destinationRGBBlendFactor   = hs_rhi_to_blend_factor(info.colorBlendDesc.attachments[i].dstColorFactor);
        colorDesc.rgbBlendOperation           = hs_rhi_to_blend_operation(info.colorBlendDesc.attachments[i].colorBlendOp);
        colorDesc.sourceAlphaBlendFactor      = hs_rhi_to_blend_factor(info.colorBlendDesc.attachments[i].srcAlphaFactor);
        colorDesc.destinationAlphaBlendFactor = hs_rhi_to_blend_factor(info.colorBlendDesc.attachments[i].dstAlphaFactor);
        colorDesc.alphaBlendOperation         = hs_rhi_to_blend_operation(info.colorBlendDesc.attachments[i].alphaBlendOp);
    }

    if (info.depthStencilDesc.depthTestEnable)
    {
        const Attachment& depthStencilAttachment = info.renderPass->info.depthStencilAttachment;
        MTLPixelFormat depthStencilFormat        = hs_rhi_to_pixel_format(depthStencilAttachment.format);
        // TODO: 스텐실 처리 추가
    }

    NSError* error               = nil;
    pipelineMetal->pipelineState = [s_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];

    if (error)
    {
        HS_LOG(crash, "Failed to create Graphics Pipeline");
    }

    if (info.renderPass->info.useDepthStencilAttachment)
    {
        MTLDepthStencilDescriptor* depthStencilDesc = [MTLDepthStencilDescriptor new];
        bool stencilTest                            = info.depthStencilDesc.stencilTestEnable;

        if (!info.depthStencilDesc.depthTestEnable)
        {
            depthStencilDesc.depthCompareFunction = MTLCompareFunctionAlways;
        }
        else
        {
            depthStencilDesc.depthCompareFunction = hs_rhi_to_compare_function(info.depthStencilDesc.depthCompareOp);
            depthStencilDesc.depthWriteEnabled    = info.depthStencilDesc.depthWriteEnable;
        }

        if (stencilTest)
        {
            // TODO: 스텐실 처리
        }

        pipelineMetal->depthStencilState = [s_device newDepthStencilStateWithDescriptor:depthStencilDesc];
        [depthStencilDesc release];
    }

    return static_cast<RHIGraphicsPipeline*>(pipelineMetal);
}

void MetalContext::DestroyGraphicsPipeline(RHIGraphicsPipeline* pipeline)
{
    MetalGraphicsPipeline* pipelineMetal = static_cast<MetalGraphicsPipeline*>(pipeline);

    delete pipelineMetal;
}

RHIShader* MetalContext::CreateShader(const char* name, const ShaderInfo& info, const char* path)
{
    FileHandle handle = 0;

    bool result = FileSystem::Open(std::string(path), EFileAccess::READ_ONLY, handle);
    if (!result)
    {
        HS_LOG(crash, "Can't open path");
        return nullptr;
    }
    size_t byteCodeSize = FileSystem::GetSize(handle);

    char* buffer    = new char[byteCodeSize];
    size_t readSize = FileSystem::Read(handle, buffer, byteCodeSize);
    if (readSize != byteCodeSize)
    {
        HS_LOG(crash, "Can't read all contents");
        delete[] buffer;

        return nullptr;
    }

    RHIShader* shader = CreateShader(name, info, buffer, byteCodeSize);

    delete[] buffer;

    return shader;
}

RHIShader* MetalContext::CreateShader(const char* name, const ShaderInfo& info,  const char* byteCode, size_t byteCodeSize)
{
    const static std::string metalLibPath = SystemContext::Get()->assetDirectory + std::string("Shaders") + HS_DIR_SEPERATOR + "default.metallib";

    MetalShader* MetalShader = new struct MetalShader(name, info);

    NSError* error = nil;
    NSURL* url     = [NSURL fileURLWithPath:[NSString stringWithCString:metalLibPath.c_str() encoding:NSUTF8StringEncoding]];

    //        NSString*          source = [NSString stringWithCString:byteCode encoding:NSUTF8StringEncoding];
    //        MTLCompileOptions* option = [MTLCompileOptions new];

    //        id<MTLLibrary> library = [s_device newLibraryWithSource:source options:option error:&error];
    id<MTLLibrary> library = [s_device newLibraryWithURL:url error:&error];
    if (nil == library)
    {
        HS_LOG(crash, "Fail to cretae MTLLibrary");
        return nullptr;
    }

    NSString* entry = [NSString stringWithCString:info.entryName encoding:NSUTF8StringEncoding];

    id<MTLFunction> func = nil;
    switch (info.stage)
    {
        case EShaderStage::VERTEX:
        {
            func = [library newFunctionWithName:entry];
        }
        break;
        case EShaderStage::FRAGMENT:
        {
            func = [library newFunctionWithName:entry];
        }
        break;
        case EShaderStage::COMPUTE:
        {
            //...
        }
            //            break;
        default:
        {
            HS_LOG(crash, "This stage is Not supported yet");
        }
        break;
    }

    if (nil == func)
    {
        HS_LOG(crash, "Fail to create MTLFunction");
        return nullptr;
    }

    MetalShader->handle = func;

    return MetalShader;
}

void MetalContext::DestroyShader(RHIShader* shader)
{
    MetalShader* MetalShader = static_cast<struct MetalShader*>(shader);

    delete MetalShader;
}

RHIBuffer* MetalContext::CreateBuffer(const char* name, const void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption)
{
    BufferInfo info{};
    info.usage        = usage;
    info.memoryOption = memoryOption;

    RHIBuffer* result = CreateBuffer(name, data, dataSize, info);

    return result;
}

RHIBuffer* MetalContext::CreateBuffer(const char* name, const void* data, size_t dataSize, const BufferInfo& info)
{
    MetalBuffer* MetalBuffer = new struct MetalBuffer(name, info);

    id<MTLBuffer> mtlBuffer = [s_device newBufferWithBytes:data length:dataSize options:hs_rhi_to_buffer_option(info.memoryOption)];

    if (nil == mtlBuffer)
    {
        HS_LOG(crash, "Fail to create buffer");
    }

    MetalBuffer->handle = mtlBuffer;

    return static_cast<RHIBuffer*>(MetalBuffer);
}

void MetalContext::DestroyBuffer(RHIBuffer* buffer)
{
    MetalBuffer* MetalBuffer = static_cast<struct MetalBuffer*>(buffer);

    delete MetalBuffer;
}

RHITexture* MetalContext::CreateTexture(const char* name, void* image, const TextureInfo& info)
{
    MetalTexture* MetalTexture = new struct MetalTexture(name, info);

    MTLTextureDescriptor* desc = [MTLTextureDescriptor new];
    desc.width                 = info.extent.width;
    desc.height                = info.extent.height;
    desc.depth                 = info.extent.depth;
    desc.arrayLength           = info.arrayLength;
    desc.mipmapLevelCount      = info.mipLevel;
    desc.usage                 = hs_rhi_to_texture_usage(info.usage);
    desc.sampleCount           = 1;
    desc.storageMode           = MTLStorageModeManaged;
    desc.pixelFormat           = hs_rhi_to_pixel_format(info.format);
    desc.textureType           = hs_rhi_to_texture_type(info.type);

    MetalTexture->handle = [s_device newTextureWithDescriptor:desc];

    [desc release];

    return static_cast<RHITexture*>(MetalTexture);
}

RHITexture* MetalContext::CreateTexture(const char* name, void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage)
{
    TextureInfo info{};
    info.extent.width  = width;
    info.extent.height = height;
    info.extent.depth  = 1;
    info.format        = format;
    info.type          = type;
    info.usage         = usage;

    RHITexture* result = CreateTexture(name, image, info);

    return result;
}

void MetalContext::DestroyTexture(RHITexture* texture)
{
    MetalTexture* MetalTexture = static_cast<struct MetalTexture*>(texture);

    delete MetalTexture;
}

RHISampler* MetalContext::CreateSampler(const char* name, const SamplerInfo& info)
{
    MetalSampler* MetalSampler = new struct MetalSampler(name, info);

    return static_cast<RHISampler*>(MetalSampler);
}

void MetalContext::DestroySampler(RHISampler* sampler)
{
    MetalSampler* MetalSampler = static_cast<struct MetalSampler*>(sampler);

    delete MetalSampler;
}

RHIResourceLayout* MetalContext::CreateResourceLayout(const char* name, ResourceBinding* bindings, uint32 bindingCount)
{
    MetalResourceLayout* resLayoutMetal = new MetalResourceLayout(name, bindings, bindingCount);

    return resLayoutMetal;
}

void MetalContext::DestroyResourceLayout(RHIResourceLayout* resLayout)
{
    MetalResourceLayout* resLayoutMetal = static_cast<MetalResourceLayout*>(resLayout);

    delete resLayoutMetal;
}

RHIResourceSet* MetalContext::CreateResourceSet(const char* name, RHIResourceLayout* resourceLayout)
{
    MetalResourceSet* resSetMetal = new MetalResourceSet(name);

    return static_cast<RHIResourceSet*>(resSetMetal);
}

void MetalContext::DestroyResourceSet(RHIResourceSet* resSet)
{
    MetalResourceSet* resSetMetal = static_cast<MetalResourceSet*>(resSet);

    delete resSetMetal;
}

RHIResourceSetPool* MetalContext::CreateResourceSetPool(const char* name, uint32 bufferSize, uint32 textureSize)
{
    MetalResourceSetPool* resSetPoolMetal = new MetalResourceSetPool(name);

    return static_cast<RHIResourceSetPool*>(resSetPoolMetal);
}

void MetalContext::DestroyResourceSetPool(RHIResourceSetPool* resSetPool)
{
    MetalResourceSetPool* resSetPoolMetal = static_cast<MetalResourceSetPool*>(resSetPool);

    delete resSetPoolMetal;
}

RHICommandPool* MetalContext::CreateCommandPool(const char* name, uint32 queueFamilyIndices)
{
    MetalCommandPool* cmdPoolMetal = new MetalCommandPool(name);

    return static_cast<RHICommandPool*>(cmdPoolMetal);
}

void MetalContext::DestroyCommandPool(RHICommandPool* cmdPool)
{
    MetalCommandPool* cmdPoolMetal = static_cast<MetalCommandPool*>(cmdPool);

    delete cmdPoolMetal;
}

RHICommandBuffer* MetalContext::CreateCommandBuffer(const char* name)
{
    MetalCommandBuffer* cmdMetalBuffer = new MetalCommandBuffer(name, s_device, s_cmdQueue);

    return static_cast<RHICommandBuffer*>(cmdMetalBuffer);
}

void MetalContext::DestroyCommandBuffer(RHICommandBuffer* cmdBuffer)
{
    MetalCommandBuffer* cmdMetalBuffer = static_cast<MetalCommandBuffer*>(cmdBuffer);

    delete cmdMetalBuffer;
}

void MetalContext::Submit(Swapchain* swapchain, RHICommandBuffer** cmdBuffers, size_t bufferCount)
{
    //...
}

void MetalContext::Present(Swapchain* swapchain)
{
    SwapchainMetal* swMetal  = static_cast<SwapchainMetal*>(swapchain);
    RHICommandBuffer* cmdBuffer = swMetal->GetCommandBufferForCurrentFrame();

    MetalCommandBuffer* cmdMetalBuffer = static_cast<MetalCommandBuffer*>(cmdBuffer);
    [cmdMetalBuffer->handle presentDrawable:swMetal->_drawable];
    [cmdMetalBuffer->handle commit];
    [cmdMetalBuffer->handle waitUntilCompleted];
}

void MetalContext::WaitForIdle() const
{
    //...
}

HS_NS_END
