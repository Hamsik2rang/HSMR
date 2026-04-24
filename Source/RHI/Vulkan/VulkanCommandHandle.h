//
//  CommandHandleVulkan.h
//  Engine
//
//  Created by Yongsik Im on 4/27/25.
//
#ifndef __HS_COMMAND_HANDLE_VULKAN_H__
#define __HS_COMMAND_HANDLE_VULKAN_H__

#include "Precompile.h"

#include "RHI/CommandHandle.h"
#include "RHI/ResourceHandle.h"

#include "RHI/Vulkan/VulkanDevice.h"

HS_NS_BEGIN

class VulkanContext;

struct HS_RHI_API VulkanCommandQueue : public RHICommandQueue
{
    VulkanCommandQueue(const char* name);
    ~VulkanCommandQueue() override;

    uint32 queueIndex;
};

struct HS_RHI_API VulkanCommandPool : public RHICommandPool
{
    VulkanCommandPool(const char* name);
    ~VulkanCommandPool() override;

    VkCommandPool handle;
};

struct HS_RHI_API VulkanCommandBuffer : public RHICommandBuffer
{
    VulkanCommandBuffer(const char* name, VulkanContext* context);
    ~VulkanCommandBuffer() override;
    void Begin() override;
    void End() override;
    void Reset() override;

    void BindPipeline(RHIGraphicsPipeline* pipeline) override;
    void BindResourceSet(RHIResourceSet* rSet) override;
    void SetViewport(const Viewport& viewport) override;
    void SetScissor(const uint32 x, const uint32 y, const uint32 width, const uint32 height) override;
    void BindIndexBuffer(RHIBuffer* indexBuffer) override;
    void BindVertexBuffers(const RHIBuffer* const* vertexBuffers, const uint32* offsets, const uint8 bufferCount) override;
    void DrawArrays(const uint32 firstVertex, const uint32 vertexCount, const uint32 instanceCount) override;
    void DrawIndexed(const uint32 firstIndex, const uint32 indexCount, const uint32 instanceCount, const uint32 vertexOffset) override;

    void BeginRendering(const RenderingInfo& renderingInfo) override;
    void EndRendering() override;

    // Compute commands
    void BindComputePipeline(RHIComputePipeline* pipeline) override;
    void BindComputeResourceSet(RHIResourceSet* rSet) override;
    void Dispatch(uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ) override;
    void EndComputePass() override;

    // Memory barriers
    void TextureBarrier(const RHITextureBarrierDesc* barriers, uint32 count) override;
    void TextureBarrier(RHITexture* texture) override;

    void CopyTexture(RHITexture* srcTexture, RHITexture* dstTexture) override;
    void UpdateBuffer(RHIBuffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize) override;

    void PushDebugMark(const char* label, float color[4]) override;
    void PopDebugMark() override;

    VkCommandBuffer handle                     = VK_NULL_HANDLE;
    VkPipeline curGraphicsPipeline             = VK_NULL_HANDLE;
    VkPipelineLayout curGraphicsPipelineLayout = VK_NULL_HANDLE;
    VkPipeline curComputePipeline              = VK_NULL_HANDLE;
    VkPipelineLayout curComputePipelineLayout  = VK_NULL_HANDLE;

private:
    VulkanContext* _rhiContext = nullptr;
    bool _useDynamicRendering = false;
    RenderingInfo _currentRenderingInfo{};
};

HS_NS_END

#endif
