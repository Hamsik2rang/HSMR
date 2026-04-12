#include "RHI/Metal/MetalContext.h"
#include "RHI/Metal/MetalDefinition.h"

#include "RHI/Metal/MetalSwapchain.h"
#include "RHI/Metal/MetalUtility.h"
#include "RHI/Metal/MetalRenderHandle.h"
#include "RHI/Metal/MetalCommandHandle.h"
#include "RHI/Metal/MetalResourceHandle.h"

#include "Core/SystemContext.h"
#include "Core/HAL/FileSystem.h"
#include "Core/Native/NativeWindow.h"

#include <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

HS_NS_BEGIN

static bool isCPUAccessibleBuffer(EBufferMemoryOption memoryOption)
{
    switch (memoryOption)
    {
    case EBufferMemoryOption::Dynamic:
    case EBufferMemoryOption::Mapped:
        return true;
    default:
        return false;
    }
}

id<MTLDevice> s_device         = nil;
id<MTLCommandQueue> s_cmdQueue = nil; // TODO: Mult-CommandQueue로 변경

bool MetalContext::Initialize()
{
    NSLog(@"System: %@", [NSProcessInfo processInfo]);
    NSLog(@"Available MTL devices: %@", MTLCopyAllDevices());

    s_device   = MTLCreateSystemDefaultDevice();
    s_cmdQueue = [s_device newCommandQueue];

    _device = (__bridge void*)s_device;

    _capabilities.platform = ERHIPlatform::Metal;
    _capabilities.renderingPath = ERHIRenderingPath::DynamicRendering;
    _capabilities.resourceBindingTier = ERHIResourceBindingTier::LegacyDescriptorSet;
    _capabilities.deviceName = [[s_device name] UTF8String];
    _capabilities.supportsDynamicRendering = true;
#if defined(__MAC_11_0) || defined(__IPHONE_14_0)
    if ([s_device respondsToSelector:@selector(argumentBuffersSupport)])
    {
        MTLArgumentBuffersTier tier = [s_device argumentBuffersSupport];
        _capabilities.supportsArgumentBufferTier2 = (tier == MTLArgumentBuffersTier2);
        _capabilities.supportsBindless = _capabilities.supportsArgumentBufferTier2;
        _capabilities.resourceBindingTier = _capabilities.supportsArgumentBufferTier2
            ? ERHIResourceBindingTier::Bindless
            : ERHIResourceBindingTier::LegacyDescriptorSet;
    }
#endif

    return true;
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
    MetalSwapchain* swMetal = static_cast<MetalSwapchain*>(swapchain);

    const uint32 maxFrameCount = swMetal->_maxFrameCount;
    swMetal->_frameIndex       = (swMetal->_frameIndex + 1) % maxFrameCount;

    auto nativeWindow = swapchain->GetInfo().nativeWindow;

    // Use graphicsLayer directly from NativeWindow (works with both SDL and native paths)
    CAMetalLayer* layer = (__bridge CAMetalLayer*)(nativeWindow->graphicsLayer);

    id<CAMetalDrawable> drawable = [layer nextDrawable];
    swMetal->_drawable           = drawable;

    MetalTexture* colorTexture = static_cast<MetalTexture*>(swMetal->GetCurrentColorTexture());
    colorTexture->handle = drawable.texture;

    return swMetal->_frameIndex;
}

Swapchain* MetalContext::CreateSwapchain(SwapchainInfo info)
{
    MetalSwapchain* swMetal = new MetalSwapchain(info);

    return static_cast<Swapchain*>(swMetal);
}

void MetalContext::DestroySwapchain(Swapchain* swapchain)
{
    MetalSwapchain* swMetal = static_cast<MetalSwapchain*>(swapchain);
    // Swapchain에 들어간 view, layer등은 모두 reference이기 때문에 별도로 제거할 필요가 없다.
    swMetal->nativeHandle   = nullptr;
    delete swMetal;
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
        case EShaderStage::Vertex:
            [pipelineDesc setVertexFunction:static_cast<MetalShader*>(shader)->handle];
            break;
        case EShaderStage::Fragment:
            [pipelineDesc setFragmentFunction:static_cast<MetalShader*>(shader)->handle];
            break;
        default:
            HS_LOG(crash, "Not supported yet");
            break;
        }
    }

    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];

    for (size_t i = 0; i < info.vertexInputDesc.attributes.size(); i++)
    {
        const auto& curAttribute = info.vertexInputDesc.attributes[i];
        HS_ASSERT(curAttribute.binding < kMetalReservedVertexBufferSlotCount,
                  "Metal vertex attribute binding exceeds reserved vertex buffer slots");
        const NSUInteger bufferIndex = MetalVertexBufferSlotForBinding(curAttribute.binding);
        HS_ASSERT(bufferIndex < kMetalMaxVertexBufferSlotCount, "Metal vertex attribute buffer index out of range");

        vertexDesc.attributes[i].offset      = curAttribute.offset;
        vertexDesc.attributes[i].bufferIndex = bufferIndex;
        vertexDesc.attributes[i].format      = MetalUtility::ToVertexFormat(curAttribute.format);
    }

    for (size_t i = 0; i < info.vertexInputDesc.layouts.size(); i++)
    {
        const auto& curLayout = info.vertexInputDesc.layouts[i];
        HS_ASSERT(curLayout.binding < kMetalReservedVertexBufferSlotCount,
                  "Metal vertex layout binding exceeds reserved vertex buffer slots");
        NSUInteger layoutIdx  = MetalVertexBufferSlotForBinding(curLayout.binding);
        HS_ASSERT(layoutIdx < kMetalMaxVertexBufferSlotCount, "Metal vertex layout buffer index out of range");

        vertexDesc.layouts[layoutIdx].stride       = curLayout.stride;
        vertexDesc.layouts[layoutIdx].stepRate     = static_cast<uint8>(curLayout.stepRate);
        vertexDesc.layouts[layoutIdx].stepFunction = curLayout.useInstancing ? MTLVertexStepFunctionPerInstance : MTLVertexStepFunctionPerVertex;
    }

    pipelineDesc.vertexDescriptor = vertexDesc;

    PipelineRenderTargetLayout renderTargetLayout = info.renderTargetLayout;

    for (size_t i = 0; i < renderTargetLayout.colorAttachmentCount; i++)
    {
        MTLRenderPipelineColorAttachmentDescriptor* colorDesc = pipelineDesc.colorAttachments[i];

        colorDesc.pixelFormat                 = MetalUtility::ToPixelFormat(renderTargetLayout.colorFormats[i]);
        colorDesc.blendingEnabled             = info.colorBlendDesc.attachments[i].blendEnable;
        colorDesc.sourceRGBBlendFactor        = MetalUtility::ToBlendFactor(info.colorBlendDesc.attachments[i].srcColorFactor);
        colorDesc.destinationRGBBlendFactor   = MetalUtility::ToBlendFactor(info.colorBlendDesc.attachments[i].dstColorFactor);
        colorDesc.rgbBlendOperation           = MetalUtility::ToBlendOperation(info.colorBlendDesc.attachments[i].colorBlendOp);
        colorDesc.sourceAlphaBlendFactor      = MetalUtility::ToBlendFactor(info.colorBlendDesc.attachments[i].srcAlphaFactor);
        colorDesc.destinationAlphaBlendFactor = MetalUtility::ToBlendFactor(info.colorBlendDesc.attachments[i].dstAlphaFactor);
        colorDesc.alphaBlendOperation         = MetalUtility::ToBlendOperation(info.colorBlendDesc.attachments[i].alphaBlendOp);
    }

    if (info.depthStencilDesc.depthTestEnable)
    {
        MTLPixelFormat depthStencilFormat        = MetalUtility::ToPixelFormat(renderTargetLayout.depthStencilFormat);
        pipelineDesc.depthAttachmentPixelFormat  = depthStencilFormat;
        // TODO: 스텐실 처리 추가
    }
    
    NSError* error               = nil;
    pipelineMetal->pipelineState = [s_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];

    if (error)
    {
        HS_LOG(crash, "Failed to create Graphics Pipeline");
    }

    if (renderTargetLayout.useDepthStencilAttachment)
    {
        MTLDepthStencilDescriptor* depthStencilDesc = [MTLDepthStencilDescriptor new];
        bool stencilTest                            = info.depthStencilDesc.stencilTestEnable;

        if (!info.depthStencilDesc.depthTestEnable)
        {
            depthStencilDesc.depthCompareFunction = MTLCompareFunctionAlways;
        }
        else
        {
            depthStencilDesc.depthCompareFunction = MetalUtility::ToCompareFunction(info.depthStencilDesc.depthCompareOp);
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

RHIComputePipeline* MetalContext::CreateComputePipeline(const char* name, const ComputePipelineInfo& info)
{
    MetalComputePipeline* pipelineMetal = new MetalComputePipeline(name, info);

    MetalShader* shaderMetal        = static_cast<MetalShader*>(info.computeShader);
    id<MTLFunction> computeFunction = shaderMetal->handle;

    NSError* error               = nil;
    pipelineMetal->pipelineState = [s_device newComputePipelineStateWithFunction:computeFunction error:&error];

    if (error != nil)
    {
        HS_LOG(error, "Failed to create compute pipeline: %s", [[error localizedDescription] UTF8String]);
        delete pipelineMetal;
        return nullptr;
    }

    return static_cast<RHIComputePipeline*>(pipelineMetal);
}

void MetalContext::DestroyComputePipeline(RHIComputePipeline* pipeline)
{
    MetalComputePipeline* pipelineMetal = static_cast<MetalComputePipeline*>(pipeline);

    delete pipelineMetal;
}

RHIShader* MetalContext::CreateShader(const char* name, const ShaderInfo& info, const char* path)
{
    FileHandle handle = 0;

    bool result = FileSystem::Open(std::string(path), EFileAccess::ReadOnly, handle);
    if (!result)
    {
        HS_LOG(crash, "Can't open path");
        return nullptr;
    }
    size_t byteCodeSize = FileSystem::GetSize(handle);

    char* buffer    = new char[byteCodeSize + 1]{'\0'};
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

RHIShader* MetalContext::CreateShader(const char* name, const ShaderInfo& info, const char* byteCode, size_t byteCodeSize)
{
    //    const static std::string metalLibPath = SystemContext::Get()->assetDirectory + std::string("Shaders") + HS_DIR_SEPERATOR + "default.metallib";
    //
    //    MetalShader* MetalShader = new struct MetalShader(name, info);
    //
    NSError* error   = nil;
    //    NSURL* url     = [NSURL fileURLWithPath:[NSString stringWithCString:metalLibPath.c_str() encoding:NSUTF8StringEncoding]];
    //
    // id<MTLLibrary> library = [s_device newLibraryWithSource:source options:nil error:&error];
    NSString* source = nil;
    if (byteCode[byteCodeSize - 1] != '\0')
    {
        char* byteCodeWithNull = new char[byteCodeSize + 1]{'\0'};
        memcpy(byteCodeWithNull, byteCode, byteCodeSize);
        source = [NSString stringWithCString:byteCodeWithNull encoding:NSUTF8StringEncoding];
    }
    else
    {
        source = [NSString stringWithCString:byteCode encoding:NSUTF8StringEncoding];
    }

    if (nil == source)
    {
        HS_LOG(crash, "Shader source is nil!");
    }
    id<MTLLibrary> library = [s_device newLibraryWithSource:source options:nil error:&error];

    if (nil == library)
    {
        HS_LOG(crash, "Fail to cretae MTLLibrary, error code: %d");
        return nullptr;
    }
    NSString* entry = [NSString stringWithCString:info.entryName encoding:NSUTF8StringEncoding];

    id<MTLFunction> func = nil;
    switch (info.stage)
    {
    case EShaderStage::Vertex:
    {
        func = [library newFunctionWithName:entry];
    }
    break;
    case EShaderStage::Fragment:
    {
        func = [library newFunctionWithName:entry];
    }
    break;
    case EShaderStage::Compute:
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

    MetalShader* mtlShader = new MetalShader(name, info);
    mtlShader->handle      = func;

    return static_cast<RHIShader*>(mtlShader);
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
    MetalBuffer* mtlBuffer = new struct MetalBuffer(name, info);

    HS_ASSERT(dataSize > 0, "Buffer size must be greater than 0");

    const MTLResourceOptions resourceOptions = MetalUtility::ToBufferOption(info.memoryOption);
    id<MTLBuffer> handle = [s_device newBufferWithLength:dataSize options:resourceOptions];
 
    if (nil == handle)
    {
        HS_LOG(crash, "Fail to create buffer");
    }

    mtlBuffer->handle   = handle;
    mtlBuffer->byte     = isCPUAccessibleBuffer(info.memoryOption) ? [handle contents] : nullptr;
    mtlBuffer->byteSize = dataSize;

    if (data != nullptr)
    {
        UpdateBuffer(static_cast<RHIBuffer*>(mtlBuffer), 0, data, dataSize);
    }

    return static_cast<RHIBuffer*>(mtlBuffer);
}

void MetalContext::DestroyBuffer(RHIBuffer* buffer)
{
    MetalBuffer* mtlBuffer = static_cast<struct MetalBuffer*>(buffer);

    delete mtlBuffer;
}

void MetalContext::UpdateBuffer(RHIBuffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize)
{
    HS_ASSERT(buffer, "Buffer is nullptr");
    HS_ASSERT(srcData, "Source data is nullptr");
    HS_ASSERT(dataSize > 0, "Data size must be greater than 0");
    HS_ASSERT(dstOffset + dataSize <= buffer->byteSize, "Buffer update range is out of bounds");

    MetalBuffer* mtlBuffer = static_cast<struct MetalBuffer*>(buffer);
    id<MTLBuffer> handle   = mtlBuffer->handle;
    switch (buffer->info.memoryOption)
    {
    case EBufferMemoryOption::Static:
    {
        id<MTLBuffer> stagingBuffer = [s_device newBufferWithLength:dataSize options:MTLResourceStorageModeShared];
        memcpy([stagingBuffer contents], srcData, dataSize);

        id<MTLCommandBuffer> cmdBuffer        = [s_cmdQueue commandBuffer];
        id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];

        [blitEncoder copyFromBuffer:stagingBuffer
                       sourceOffset:0
                           toBuffer:handle
                  destinationOffset:dstOffset
                               size:dataSize];
        
        [blitEncoder endEncoding];
        [cmdBuffer commit];
        [cmdBuffer waitUntilCompleted];
        
        [stagingBuffer release];
        
        break;
    }

    case EBufferMemoryOption::Dynamic:
    case EBufferMemoryOption::Mapped:
    {
        void* rawPtr = static_cast<uint8_t*>([handle contents]) + dstOffset;
        memcpy(rawPtr, srcData, dataSize);

        if (buffer->info.memoryOption == EBufferMemoryOption::Mapped)
        {
            [handle didModifyRange:NSMakeRange(dstOffset, dataSize)];
        }

        break;
    }
    default:
    {
        HS_LOG(crash, "Unsupported Buffer Memory Option!");
        break;
    }
    }
}

RHITexture* MetalContext::CreateTexture(const char* name, void* image, const TextureInfo& info)
{
    MetalTexture* mtlTexture = new struct MetalTexture(name, info);

    MTLTextureDescriptor* desc = [MTLTextureDescriptor new];
    desc.width                 = info.extent.width;
    desc.height                = info.extent.height;
    desc.depth                 = info.extent.depth;
    desc.arrayLength           = info.arrayLength == 0 ? 1 : info.arrayLength;
    desc.mipmapLevelCount      = info.mipLevel;
    desc.usage                 = MetalUtility::ToTextureUsage(info.usage);
    desc.sampleCount           = 1;
    desc.pixelFormat           = MetalUtility::ToPixelFormat(info.format);
    desc.textureType           = MetalUtility::ToTextureType(info.type);

    // Use Private storage mode for compute shader storage textures (GPU-only access)
    // Use Managed for textures that need CPU readback
    if ((info.usage & ETextureUsage::Storage) != static_cast<ETextureUsage>(0))
    {
        desc.storageMode = MTLStorageModePrivate;
    }
    else
    {
        desc.storageMode = MTLStorageModeManaged;
    }

    mtlTexture->handle = [s_device newTextureWithDescriptor:desc];

    if (nullptr != image)
    {
        if (MTLStorageModePrivate == desc.storageMode)
        {
            id<MTLBuffer> stagingBuffer = [s_device newBufferWithBytes:image length:info.byteSize options:MTLResourceStorageModeShared];
            
            id<MTLCommandBuffer> cmdBuffer = [s_cmdQueue commandBuffer];
            id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];
            
            NSUInteger bytesPerRow = info.byteSize / info.extent.depth / info.extent.height;
            NSUInteger bytesPerImage = bytesPerRow * info.extent.height;
            
            [blitEncoder copyFromBuffer: stagingBuffer
                           sourceOffset:0
                      sourceBytesPerRow:bytesPerRow
                    sourceBytesPerImage:bytesPerImage
                             sourceSize:MTLSizeMake(info.extent.width, info.extent.height, info.extent.depth)
                              toTexture:mtlTexture->handle
                       destinationSlice:0
                       destinationLevel:0
                      destinationOrigin:MTLOriginMake(0, 0, 0)];
            
            [blitEncoder endEncoding];
            [cmdBuffer commit];
            [cmdBuffer waitUntilCompleted];
            
            [stagingBuffer release];
        }
        else
        {

            NSUInteger bytesPerRow = info.byteSize / info.extent.depth / info.extent.height;
            MTLRegion region       = {
                {0, 0, 0},
                {info.extent.width, info.extent.height, info.extent.depth}
            };

            [mtlTexture->handle replaceRegion:region mipmapLevel:0 withBytes:image bytesPerRow:bytesPerRow];
        }
    }
    [desc release];

    return static_cast<RHITexture*>(mtlTexture);
}

RHITextureMemoryRequirements MetalContext::GetTextureMemoryRequirements(const TextureInfo& info)
{
    return {};
}

RHIHeap* MetalContext::CreateHeap(const RHIHeapInfo& info)
{
    return nullptr;
}

void MetalContext::DestroyHeap(RHIHeap* heap)
{
}

RHITexture* MetalContext::CreateTexture(const char* name, const TextureInfo& info, RHIHeap* heap, uint64 offset)
{
    return nullptr;
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
    MetalTexture* mtlTexture = static_cast<struct MetalTexture*>(texture);

    delete mtlTexture;
}

RHISampler* MetalContext::CreateSampler(const char* name, const SamplerInfo& info)
{
    MetalSampler* mtlSampler = new struct MetalSampler(name, info);

    MTLSamplerDescriptor* desc = [[MTLSamplerDescriptor alloc] init];

    desc.minFilter    = MetalUtility::ToMinMagFilter(info.minFilter);
    desc.magFilter    =  MetalUtility::ToMinMagFilter(info.magFilter);
    desc.mipFilter    =  MetalUtility::ToMipFilter(info.mipmapMode);
    desc.sAddressMode = MetalUtility::ToSamplerAddressMode(info.addressU);
    desc.tAddressMode = MetalUtility::ToSamplerAddressMode(info.addressV);
    desc.rAddressMode = MetalUtility::ToSamplerAddressMode(info.addressW);
    desc.label        = [NSString stringWithUTF8String:name];

    mtlSampler->handle = [s_device newSamplerStateWithDescriptor:desc];

    return static_cast<RHISampler*>(mtlSampler);
}

void MetalContext::DestroySampler(RHISampler* sampler)
{
    MetalSampler* mtlSampler = static_cast<struct MetalSampler*>(sampler);

    delete mtlSampler;
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
    if (resourceLayout)
    {
        resSetMetal->layouts.push_back(resourceLayout);
    }

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
    MetalSwapchain* swMetal     = static_cast<MetalSwapchain*>(swapchain);
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
