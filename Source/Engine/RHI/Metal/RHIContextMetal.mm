#include "Engine/RHI/Metal/RHIContextMetal.h"

#include "Engine/RHI/Metal/SwapchainMetal.h"
#include "Engine/RHI/Metal/RHIUtilityMetal.h"
#include "Engine/RHI/Metal/RenderHandleMetal.h"
#include "Engine/RHI/Metal/CommandHandleMetal.h"
#include "Engine/RHI/Metal/ResourceHandleMetal.h"

#include "Engine/Core/FileSystem.h"

HS_NS_BEGIN

id<MTLDevice>       s_device   = nil;
id<MTLCommandQueue> s_cmdQueue = nil; // TODO: Mult-CommandQueue로 변경

bool RHIContextMetal::Initialize()
{
    s_device   = MTLCreateSystemDefaultDevice();
    s_cmdQueue = [s_device newCommandQueue];
}

void RHIContextMetal::Finalize()
{
    //...
}

uint32 RHIContextMetal::NextFrame(Swapchain* swapchain)
{
    SwapchainMetal* swMetal       = static_cast<SwapchainMetal*>(swapchain);
    const uint32    maxFrameIndex = swMetal->GetMaxFrameIndex();
    swMetal->frameIndex           = (swMetal->frameIndex + 1) % maxFrameIndex;

    swMetal->drawable = [swMetal->layer nextDrawable];
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

    return static_cast<Framebuffer*>(fbMetal);
}

void RHIContextMetal::DestroyFramebuffer(Framebuffer* framebuffer)
{
    FramebufferMetal* fbMetal = static_cast<FramebufferMetal*>(framebuffer);

    delete fbMetal;
}

GraphicsPipeline* RHIContextMetal::CreateGraphicsPipeline(const GraphicsPipelineInfo& info)
{
    GraphicsPipelineMetal* gpMetal = new GraphicsPipelineMetal(info);

    return static_cast<GraphicsPipeline*>(gpMetal);
}

void RHIContextMetal::DestroyGraphicsPipeline(GraphicsPipeline* pipeline)
{
    GraphicsPipelineMetal* gpMetal = static_cast<GraphicsPipelineMetal*>(pipeline);

    delete gpMetal;
}

Shader* RHIContextMetal::CreateShader(EShaderStage stage, const char* path, const char* entryName, bool isBuiltIn)
{
    FileHandle handle;

    bool result = hs_file_open(std::string(path), EFileAccess::READ_ONLY, handle);
    if (!result)
    {
        HS_LOG(crash, "Can't open path");
        return nullptr;
    }
    size_t byteCodeSize = hs_file_get_size(handle);

    char*  buffer   = new char[byteCodeSize];
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
    ShaderInfo info{};
    info.statge    = stage;
    info.entryName = entryNamel;
    info.isBuiltIn = isBuitIn;

    ShaderMetal* shaderMetal = new ShaderMetal(byteCode, byteCodeSize, info);
}

void RHIContextMetal::DestroyShader(Shader* shader)
{
    ShaderMetal* shaderMetal = static_cast<ShaderMatel*>(shader);

    delete shaderMetal;
}

Buffer* RHIContextMetal::CreateBuffer()
{
    BufferMetal* bufferMetal = new BufferMetal();
}

void RHIContextMetal::DestroyBuffer(Buffer* buffer)
{
}

Texture* RHIContextMetal::CreateTextrue(void* image, const TextureInfo& info)
{
}

Texture* RHIContextMetal::CreateTexture(void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage)
{
}

void RHIContextMetal::DestroyTexture(Texture* texture)
{
}

HS_NS_END
