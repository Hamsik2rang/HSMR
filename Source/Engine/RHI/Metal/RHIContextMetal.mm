#include "Engine/RHI/Metal/RHIContextMetal.h"

#include "Engine/RHI/Metal/SwapchainMetal.h"
#include "Engine/RHI/Metal/RHIUtilityMetal.h"
#include "Engine/RHI/Metal/RenderHandleMetal.h"
#include "Engine/RHI/Metal/CommandHandleMetal.h"
#include "Engine/RHI/Metal/ResourceHandleMetal.h"

#include "Engine/Core/FileSystem.h"
#include "Engine/Core/EngineContext.h"

#include "Engine/Platform/Mac/PlatformWindowMac.h"

HS_NS_BEGIN

id<MTLDevice> s_device         = nil;
id<MTLCommandQueue> s_cmdQueue = nil; // TODO: Mult-CommandQueue로 변경

bool RHIContextMetal::Initialize()
{
    NSLog(@"System: %@", [NSProcessInfo processInfo]);
    NSLog(@"Available MTL devices: %@", MTLCopyAllDevices());

    s_device   = MTLCreateSystemDefaultDevice();
    s_cmdQueue = [s_device newCommandQueue];

    _device = (__bridge void*)s_device;
}

void RHIContextMetal::Finalize()
{
    //...
}

void RHIContextMetal::Suspend(Swapchain* swapchain)
{
}

void RHIContextMetal::Restore(Swapchain* swapchain)
{
}

uint32 RHIContextMetal::AcquireNextImage(Swapchain* swapchain)
{
    SwapchainMetal* swMetal = static_cast<SwapchainMetal*>(swapchain);

    const uint32 maxFrameCount = swMetal->maxFrameCount;
    swMetal->frameIndex        = (swMetal->frameIndex + 1) % maxFrameCount;

    auto nativeWindow    = swapchain->GetInfo().nativeWindow;
    HSViewController* vc = (HSViewController*)[(__bridge NSWindow*)(nativeWindow->handle) delegate];
    NSView* view         = [vc view];
    CAMetalLayer* layer  = (CAMetalLayer*)[[vc view] layer];

    id<CAMetalDrawable> drawable = [layer nextDrawable];
    swMetal->_drawable = drawable;

    MTLRenderPassDescriptor* rpDesc        = [MTLRenderPassDescriptor renderPassDescriptor];
    rpDesc.colorAttachments[0].clearColor  = MTLClearColorMake(0.2f, 0.2f, 0.2f, 1.0f);
    rpDesc.colorAttachments[0].texture     = drawable.texture;
    rpDesc.colorAttachments[0].loadAction  = MTLLoadActionClear;
    rpDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

    RenderPassMetal* swRenderPassMetal = static_cast<RenderPassMetal*>(swMetal->GetRenderPass());
    swRenderPassMetal->handle          = rpDesc;
}

Swapchain* RHIContextMetal::CreateSwapchain(SwapchainInfo info)
{
    SwapchainMetal* swMetal = new SwapchainMetal(info);

    return static_cast<Swapchain*>(swMetal);
}

void RHIContextMetal::DestroySwapchain(Swapchain* swapchain)
{
    SwapchainMetal* swMetal = static_cast<SwapchainMetal*>(swapchain);
    // Swapchain에 들어간 view, layer등은 모두 reference이기 때문에 별도로 제거할 필요가 없다.
    swMetal->nativeHandle   = nullptr;
    delete swMetal;
}

RenderPass* RHIContextMetal::CreateRenderPass(const RenderPassInfo& info)
{
    RenderPassMetal* rpMetal = new RenderPassMetal(info);

    return static_cast<RenderPass*>(rpMetal);
}

void RHIContextMetal::DestroyRenderPass(RenderPass* renderPass)
{
    RenderPassMetal* rpMetal = static_cast<RenderPassMetal*>(renderPass);

    delete rpMetal;
}

Framebuffer* RHIContextMetal::CreateFramebuffer(const FramebufferInfo& info)
{
    FramebufferMetal* fbMetal = new FramebufferMetal(info);

    if (info.isSwapchainFramebuffer)
    {
        //...
    }

    return static_cast<Framebuffer*>(fbMetal);
}

void RHIContextMetal::DestroyFramebuffer(Framebuffer* framebuffer)
{
    FramebufferMetal* fbMetal = static_cast<FramebufferMetal*>(framebuffer);

    delete fbMetal;
}

GraphicsPipeline* RHIContextMetal::CreateGraphicsPipeline(const GraphicsPipelineInfo& info)
{
    GraphicsPipelineMetal* pipelineMetal = new GraphicsPipelineMetal(info);

    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.label                        = @"Graphics Pipeline";
    pipelineDesc.vertexFunction               = static_cast<ShaderMetal*>(info.shaderDesc.vertexShader)->handle;
    pipelineDesc.fragmentFunction             = static_cast<ShaderMetal*>(info.shaderDesc.fragmentShader)->handle;
    pipelineDesc.rasterizationEnabled         = true;

    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];

    for (size_t i = 0; i < info.vertexInputDesc.attributes.size(); i++)
    {
        const auto& curAttribute = info.vertexInputDesc.attributes[i];

        vertexDesc.attributes[i].offset      = curAttribute.offset;
        vertexDesc.attributes[i].bufferIndex = curAttribute.location;
        vertexDesc.attributes[i].format      = hs_rhi_get_vertex_format_from_size(curAttribute.formatSize);
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

    return static_cast<GraphicsPipeline*>(pipelineMetal);
}

void RHIContextMetal::DestroyGraphicsPipeline(GraphicsPipeline* pipeline)
{
    GraphicsPipelineMetal* pipelineMetal = static_cast<GraphicsPipelineMetal*>(pipeline);

    delete pipelineMetal;
}

Shader* RHIContextMetal::CreateShader(EShaderStage stage, const char* path, const char* entryName, bool isBuiltIn)
{
    FileHandle handle = 0;

    bool result = hs_file_open(std::string(path), EFileAccess::READ_ONLY, handle);
    if (!result)
    {
        HS_LOG(crash, "Can't open path");
        return nullptr;
    }
    size_t byteCodeSize = hs_file_get_size(handle);

    char* buffer    = new char[byteCodeSize];
    size_t readSize = hs_file_read(handle, buffer, byteCodeSize);
    if (readSize != byteCodeSize)
    {
        HS_LOG(crash, "Can't read all contents");
        delete[] buffer;

        return nullptr;
    }

    Shader* shader = CreateShader(stage, buffer, byteCodeSize, entryName, isBuiltIn);

    delete[] buffer;

    return shader;
}

Shader* RHIContextMetal::CreateShader(EShaderStage stage, const char* byteCode, size_t byteCodeSize, const char* entryName, bool isBuitIn)
{
    const static std::string metalLibPath = hs_engine_get_context()->resourceDirectory + std::string("Shader") + HS_DIR_SEPERATOR + "default.metallib";

    ShaderInfo info{};
    info.stage     = stage;
    info.entryName = entryName;
    info.isBuiltIn = isBuitIn;

    ShaderMetal* shaderMetal = new ShaderMetal(byteCode, byteCodeSize, info);

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

    NSString* entry = [NSString stringWithCString:entryName encoding:NSUTF8StringEncoding];

    id<MTLFunction> func = nil;
    switch (stage)
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

    shaderMetal->handle = func;

    return shaderMetal;
}

void RHIContextMetal::DestroyShader(Shader* shader)
{
    ShaderMetal* shaderMetal = static_cast<ShaderMetal*>(shader);

    delete shaderMetal;
}

Buffer* RHIContextMetal::CreateBuffer(void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption)
{
    BufferInfo info{};
    info.usage        = usage;
    info.memoryOption = memoryOption;

    Buffer* result = CreateBuffer(data, dataSize, info);

    return result;
}
Buffer* RHIContextMetal::CreateBuffer(void* data, size_t dataSize, const BufferInfo& info)
{
    BufferMetal* bufferMetal = new BufferMetal(info);

    id<MTLBuffer> mtlBuffer = [s_device newBufferWithBytes:data length:dataSize options:hs_rhi_to_buffer_option(info.memoryOption)];

    if (nil == mtlBuffer)
    {
        HS_LOG(crash, "Fail to create buffer");
    }

    bufferMetal->handle = mtlBuffer;

    return static_cast<Buffer*>(bufferMetal);
}

void RHIContextMetal::DestroyBuffer(Buffer* buffer)
{
    BufferMetal* bufferMetal = static_cast<BufferMetal*>(buffer);

    delete bufferMetal;
}

Texture* RHIContextMetal::CreateTexture(void* image, const TextureInfo& info)
{
    TextureMetal* textureMetal = new TextureMetal(info);

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

    textureMetal->handle = [s_device newTextureWithDescriptor:desc];

    [desc release];

    return static_cast<Texture*>(textureMetal);
}

Texture* RHIContextMetal::CreateTexture(void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage)
{
    TextureInfo info{};
    info.extent.width  = width;
    info.extent.height = height;
    info.extent.depth  = 1;
    info.format        = format;
    info.type          = type;
    info.usage         = usage;

    Texture* result = CreateTexture(image, info);

    return result;
}

void RHIContextMetal::DestroyTexture(Texture* texture)
{
    TextureMetal* textureMetal = static_cast<TextureMetal*>(texture);

    delete textureMetal;
}

Sampler* RHIContextMetal::CreateSampler(const SamplerInfo& info)
{
    SamplerMetal* samplerMetal = new SamplerMetal(info);

    return static_cast<Sampler*>(samplerMetal);
}

void RHIContextMetal::DestroySampler(Sampler* sampler)
{
    SamplerMetal* samplerMetal = static_cast<SamplerMetal*>(sampler);

    delete samplerMetal;
}

ResourceLayout* RHIContextMetal::CreateResourceLayout()
{
    ResourceLayoutMetal* resLayoutMetal = new ResourceLayoutMetal();

    return resLayoutMetal;
}

void RHIContextMetal::DestroyResourceLayout(ResourceLayout* resLayout)
{
    ResourceLayoutMetal* resLayoutMetal = static_cast<ResourceLayoutMetal*>(resLayout);

    delete resLayoutMetal;
}

ResourceSet* RHIContextMetal::CreateResourceSet()
{
    ResourceSetMetal* resSetMetal = new ResourceSetMetal();

    return static_cast<ResourceSet*>(resSetMetal);
}

void RHIContextMetal::DestroyResourceSet(ResourceSet* resSet)
{
    ResourceSetMetal* resSetMetal = static_cast<ResourceSetMetal*>(resSet);

    delete resSetMetal;
}

ResourceSetPool* RHIContextMetal::CreateResourceSetPool()
{
    ResourceSetPoolMetal* resSetPoolMetal = new ResourceSetPoolMetal();

    return static_cast<ResourceSetPool*>(resSetPoolMetal);
}

void RHIContextMetal::DestroyResourceSetPool(ResourceSetPool* resSetPool)
{
    ResourceSetPoolMetal* resSetPoolMetal = static_cast<ResourceSetPoolMetal*>(resSetPool);

    delete resSetPoolMetal;
}

CommandPool* RHIContextMetal::CreateCommandPool(uint32 queueFamilyIndices)
{
    CommandPoolMetal* cmdPoolMetal = new CommandPoolMetal();

    return static_cast<CommandPool*>(cmdPoolMetal);
}

void RHIContextMetal::DestroyCommandPool(CommandPool* cmdPool)
{
    CommandPoolMetal* cmdPoolMetal = static_cast<CommandPoolMetal*>(cmdPool);

    delete cmdPoolMetal;
}

CommandBuffer* RHIContextMetal::CreateCommandBuffer()
{
    CommandBufferMetal* cmdBufferMetal = new CommandBufferMetal(s_device, s_cmdQueue);

    return static_cast<CommandBuffer*>(cmdBufferMetal);
}

void RHIContextMetal::DestroyCommandBuffer(CommandBuffer* cmdBuffer)
{
    CommandBufferMetal* cmdBufferMetal = static_cast<CommandBufferMetal*>(cmdBuffer);

    delete cmdBufferMetal;
}

Fence* RHIContextMetal::CreateFence()
{
    FenceMetal* fenceMetal = new FenceMetal();

    return static_cast<Fence*>(fenceMetal);
}

void RHIContextMetal::DestroyFence(Fence* fence)
{
    FenceMetal* fenceMetal = static_cast<FenceMetal*>(fence);

    delete fenceMetal;
}

Semaphore* RHIContextMetal::CreateSemaphore()
{
    SemaphoreMetal* semaMetal = new SemaphoreMetal();

    return static_cast<Semaphore*>(semaMetal);
}

void RHIContextMetal::DestroySemaphore(Semaphore* semaphore)
{
    SemaphoreMetal* semaMetal = static_cast<SemaphoreMetal*>(semaphore);

    delete semaMetal;
}

void RHIContextMetal::Submit(Swapchain* swapchain, CommandBuffer** cmdBuffers, size_t bufferCount)
{
    //...
}

void RHIContextMetal::Present(Swapchain* swapchain)
{
    SwapchainMetal* swMetal  = static_cast<SwapchainMetal*>(swapchain);
    CommandBuffer* cmdBuffer = swMetal->GetCommandBufferForCurrentFrame();

    CommandBufferMetal* cmdBufferMetal = static_cast<CommandBufferMetal*>(cmdBuffer);
    [cmdBufferMetal->handle presentDrawable:swMetal->_drawable];
    [cmdBufferMetal->handle commit];
    [cmdBufferMetal->handle waitUntilCompleted];
}

void RHIContextMetal::WaitForIdle() const
{
    //...
}

HS_NS_END
