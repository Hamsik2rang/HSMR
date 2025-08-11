//
//  CommandHandleMetal.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_COMMAND_HANDLE_METAL_H__
#define __HS_COMMAND_HANDLE_METAL_H__

#include "Precompile.h"

#include "Core/RHI/CommandHandle.h"
#include "Core/RHI/Metal/RHIUtilityMetal.h"

HS_NS_BEGIN

class RHIContextMetal;

class PipelineMetal;
class RenderPassMetal;
class GraphicsPipelineMetal;
class FramebufferMetal;
class BufferMetal;

class BufferMetal;
class TextureMetal;
class SamplerMetal;


struct CommandQueueMetal : public CommandQueue
{
    id<MTLCommandQueue> handle;
};

struct CommandPoolMetal : public CommandPool
{
    //...
};

class CommandBufferMetal : public CommandBuffer
{
public:
    CommandBufferMetal(const char* name, id<MTLDevice> device, id<MTLCommandQueue> commandQueue);
    ~CommandBufferMetal();

    void Begin() override;
    void End() override;
    void Reset() override;

    void BeginRenderPass(RenderPass* renderPass, Framebuffer* framebuffer, const Area& renderArea) override;
    void BindPipeline(GraphicsPipeline* pipeline) override;
    void BindResourceSet(ResourceSet* rSet) override;
    void SetViewport(const Viewport& viewport) override;
    void SetScissor(const uint32 x, const uint32 y, const uint32 width, const uint32 height) override;
    void BindIndexBuffer(Buffer* indexBuffer) override;
    void BindVertexBuffers(const Buffer* const* vertexBuffers, const uint32* offsets, const uint8 bufferCount) override;
    void DrawArrays(const uint32 firstVertex, const uint32 vertexCount, const uint32 instanceCount) override;
    void DrawIndexed(const uint32 firstIndex, const uint32 indexCount, const uint32 instanceCount, const uint32 vertexOffset) override;
    void EndRenderPass() override;

    void CopyTexture(Texture* srcTexture, Texture* dstTexture) override;
    void UpdateBuffer(Buffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize) override;

    void PushDebugMark(const char* label, float color[4]) override;
    void PopDebugMark() override;

    id<MTLCommandBuffer>        handle;
    id<MTLRenderCommandEncoder> curRenderEncoder;
    MTLRenderPassDescriptor*    curRenderPassDesc;
    RenderPassMetal*            curBindRenderPass;
    FramebufferMetal*           curBindFramebuffer;
    GraphicsPipelineMetal*      curBindPipeline;
    BufferMetal*                curBindIndexBuffer;

    id<MTLDevice>       device;
    id<MTLCommandQueue> cmdQueue;

private:
    void bindBuffers(EShaderStage stage, uint8 binding, Buffer* const* buffers, const uint32* offsets, uint8 arrayCount);
    void bindTextures(EShaderStage stage, uint8 binding, Texture* const* textures, uint8 arrayCount);
    void bindSamplers(EShaderStage stage, uint8 binding, Sampler* const* samplers, uint8 arrayCount);

    bool _isBegan;
    bool _isRenderPassBegan;
};

HS_NS_END

#endif
