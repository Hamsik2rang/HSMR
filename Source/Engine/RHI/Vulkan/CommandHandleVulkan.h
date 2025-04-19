#ifndef __HS_COMMAND_HANDLE_VULKAN_H__
#define __HS_COMMAND_HANDLE_VULKAN_H__


#include "Precompile.h"

#include "Engine/RHI/CommandHandle.h"
#include "Engine/RHI/ResourceHandle.h"


HS_NS_BEGIN

class CommandBufferVulkan : public CommandBuffer
{
public:
	CommandBufferVulkan();
	~CommandBufferVulkan() override;
	void Begin() override;
	void End() override;
	void Reset() override;

	void BeginRenderPass(RenderPass* renderPass, Framebuffer* framebuffer) override;

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

	void PushDebugMark(const char* label, float color[4]);
	void PopDebugMark();

	VkCommandBuffer commandBufferVK = VK_NULL_HANDLE;
};
HS_NS_END

#endif