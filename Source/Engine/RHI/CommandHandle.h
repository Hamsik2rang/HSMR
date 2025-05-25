//
//  CommandHandle.h
//  Engine
//
//  Created by Yongsik Im on 2/8/25.
//
#ifndef __HS_COMMAND_HANDLE_H__
#define __HS_COMMAND_HANDLE_H__

#include "Precompile.h"

#include "Engine/RHI/RHIDefinition.h"

HS_NS_BEGIN

class RenderPass;
class Framebuffer;
class GraphicsPipeline;
class ResourceSet;

class Semaphore : public RHIHandle
{
public:
    ~Semaphore() override;

protected:
    Semaphore();
};

class Fence : public RHIHandle
{
public:
    ~Fence() override;

protected:
    Fence();
};

class ResourceBarrier : public RHIHandle
{
public:
    ~ResourceBarrier() override;

protected:
    ResourceBarrier();
};

class CommandQueue : public RHIHandle
{
public:
    ~CommandQueue() override;

protected:
    CommandQueue();
};

class CommandPool : public RHIHandle
{
public:
    ~CommandPool() override;

protected:
    CommandPool();
};

class CommandBuffer : public RHIHandle
{
public:
    ~CommandBuffer() override;
    
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual void Reset() = 0;
    
    virtual void BeginRenderPass(RenderPass* renderPass, Framebuffer* framebuffer) = 0;
    virtual void BindPipeline(GraphicsPipeline* pipeline) = 0;
    virtual void BindResourceSet(ResourceSet* rSet) = 0;
    virtual void SetViewport(const Viewport& viewport) = 0;
    virtual void SetScissor(const uint32 x, const uint32 y, const uint32 width, const uint32 height) = 0;
    virtual void BindIndexBuffer(Buffer* indexBuffer) = 0;
    virtual void BindVertexBuffers(const Buffer* const* vertexBuffers, const uint32* offsets, const uint8 bufferCount) = 0;
    virtual void DrawArrays(const uint32 firstVertex, const uint32 vertexCount, const uint32 instanceCount) = 0;
    virtual void DrawIndexed(const uint32 firstIndex, const uint32 indexCount, const uint32 instanceCount, const uint32 vertexOffset) = 0;
    virtual void EndRenderPass() = 0;
    
    virtual void CopyTexture(Texture* srcTexture, Texture* dstTexture) = 0;
    virtual void UpdateBuffer(Buffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize) = 0;
    
    virtual void PushDebugMark(const char* label, float color[4]) = 0;
    virtual void PopDebugMark() = 0;

protected:
    CommandBuffer();
    bool _isBegan = false;

    bool _isGraphicsBegan = false;
    bool _isComputeBegan = false;
    bool _isBlitBegan = false;
};
HS_NS_END

#endif /* __HS_COMMAND_HANDLE_H__ */
