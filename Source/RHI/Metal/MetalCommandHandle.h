//
//  MetalCommandHandle.h
//  RHI
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_COMMAND_HANDLE_METAL_H__
#define __HS_COMMAND_HANDLE_METAL_H__

#include "Precompile.h"

#include "RHI/CommandHandle.h"
#include "RHI/Metal/MetalUtility.h"

HS_NS_BEGIN

class MetalContext;

struct MetalRenderPass;
struct MetalGraphicsPipeline;
struct MetalComputePipeline;
struct MetalFramebuffer;

struct MetalBuffer;
struct MetalTexture;
struct MetalSampler;


struct MetalCommandQuele : public RHICommandQueue
{
    id<MTLCommandQueue> handle;
};

struct MetalCommandPool : public RHICommandPool
{
    MetalCommandPool(const char* name);
    //...
};

class MetalCommandBuffer : public RHICommandBuffer
{
public:
    MetalCommandBuffer(const char* name, id<MTLDevice> device, id<MTLCommandQueue> commandQueue);
    ~MetalCommandBuffer();

    void Begin() override;
    void End() override;
    void Reset() override;

    void BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer, const Area& renderArea) override;
    void BindPipeline(RHIGraphicsPipeline* pipeline) override;
    void BindResourceSet(RHIResourceSet* rSet) override;
    void SetViewport(const Viewport& viewport) override;
    void SetScissor(const uint32 x, const uint32 y, const uint32 width, const uint32 height) override;
    void BindIndexBuffer(RHIBuffer* indexBuffer) override;
    void BindVertexBuffers(const RHIBuffer* const* vertexBuffers, const uint32* offsets, const uint8 bufferCount) override;
    void DrawArrays(const uint32 firstVertex, const uint32 vertexCount, const uint32 instanceCount) override;
    void DrawIndexed(const uint32 firstIndex, const uint32 indexCount, const uint32 instanceCount, const uint32 vertexOffset) override;
    void EndRenderPass() override;

    // Compute commands
    void BindComputePipeline(RHIComputePipeline* pipeline) override;
    void BindComputeResourceSet(RHIResourceSet* rSet) override;
    void Dispatch(uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ) override;

    void CopyTexture(RHITexture* srcTexture, RHITexture* dstTexture) override;
    void UpdateBuffer(RHIBuffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize) override;

    void PushDebugMark(const char* label, float color[4]) override;
    void PopDebugMark() override;

    id<MTLCommandBuffer>        handle;
    id<MTLRenderCommandEncoder> curRenderEncoder;
    id<MTLComputeCommandEncoder> curComputeEncoder;
    MTLRenderPassDescriptor*    curRenderPassDesc;
    MetalRenderPass*            curBindRenderPass;
    MetalFramebuffer*           curBindFramebuffer;
    MetalGraphicsPipeline*      curBindPipeline;
    MetalComputePipeline*       curBindComputePipeline;
    MetalBuffer*                curBindIndexBuffer;

    id<MTLDevice>       device;
    id<MTLCommandQueue> cmdQueue;

private:
    void bindBuffers(EShaderStage stage, uint8 binding, RHIBuffer* const* buffers, const uint32* offsets, uint8 arrayCount);
    void bindTextures(EShaderStage stage, uint8 binding, RHITexture* const* textures, uint8 arrayCount);
    void bindSamplers(EShaderStage stage, uint8 binding, RHISampler* const* samplers, uint8 arrayCount);

    bool _isBegan;
    bool _isRenderPassBegan;
};

HS_NS_END

#endif
