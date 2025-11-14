//
//  CommandHandle.h
//  RHI
//
//  Created by Yongsik Im on 2/8/25.
//
#ifndef __HS_COMMAND_HANDLE_H__
#define __HS_COMMAND_HANDLE_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"

HS_NS_BEGIN

class RHIRenderPass;
class RHIFramebuffer;
class RHIGraphicsPipeline;
class RHIResourceSet;

class HS_API RHICommandQueue : public RHIHandle
{
public:
    ~RHICommandQueue() override;

protected:
    RHICommandQueue(const char* name);
};

class HS_API RHICommandPool : public RHIHandle
{
public:
    ~RHICommandPool() override;

protected:
    RHICommandPool(const char* name);
};

class HS_API RHICommandBuffer : public RHIHandle
{
public:
    ~RHICommandBuffer() override;
    
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual void Reset() = 0;
    
    virtual void BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer, const Area& renderArea) = 0;
    virtual void BindPipeline(RHIGraphicsPipeline* pipeline) = 0;
    virtual void BindResourceSet(RHIResourceSet* rSet) = 0;
    virtual void SetViewport(const Viewport& viewport) = 0;
    virtual void SetScissor(const uint32 x, const uint32 y, const uint32 width, const uint32 height) = 0;
    virtual void BindIndexBuffer(RHIBuffer* indexBuffer) = 0;
    virtual void BindVertexBuffers(const RHIBuffer* const* vertexBuffers, const uint32* offsets, const uint8 bufferCount) = 0;
    virtual void DrawArrays(const uint32 firstVertex, const uint32 vertexCount, const uint32 instanceCount) = 0;
    virtual void DrawIndexed(const uint32 firstIndex, const uint32 indexCount, const uint32 instanceCount, const uint32 vertexOffset) = 0;
    virtual void EndRenderPass() = 0;
    
    virtual void CopyTexture(RHITexture* srcTexture, RHITexture* dstTexture) = 0;
    virtual void UpdateBuffer(RHIBuffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize) = 0;
    
    virtual void PushDebugMark(const char* label, float color[4]) = 0;
    virtual void PopDebugMark() = 0;

protected:
    RHICommandBuffer(const char* name);
    bool _isBegan = false;
    //TODO: 커맨드 종류별로 버퍼 분할하기
    bool _isGraphicsBegan = false;
    bool _isComputeBegan = false;
    bool _isBlitBegan = false;
};
HS_NS_END

#endif /* __HS_COMMAND_HANDLE_H__ */
