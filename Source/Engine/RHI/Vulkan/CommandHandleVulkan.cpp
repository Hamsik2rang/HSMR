#include "Engine/RHI/Vulkan/CommandHandleVulkan.h"

#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"
#include "Engine/RHI/Vulkan/RenderHandleVulkan.h"
#include "Engine/RHI/Vulkan/ResourceHandleVulkan.h"

HS_NS_BEGIN

SemaphoreVulkan::SemaphoreVulkan(RHIDeviceVulkan device)
	: deviceVulkan(device)
{
	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.pNext = nullptr;

	vkCreateSemaphore(device, &createInfo, nullptr, &handle);
}

SemaphoreVulkan::~SemaphoreVulkan()
{
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(deviceVulkan, handle, nullptr);
		handle = VK_NULL_HANDLE;
	}
}

FenceVulkan::FenceVulkan(RHIDeviceVulkan device)
	: deviceVulkan(device)
{
	VkFenceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	createInfo.pNext = nullptr;
	vkCreateFence(deviceVulkan, &createInfo, nullptr, &handle);
}

FenceVulkan::~FenceVulkan()
{
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyFence(deviceVulkan, handle, nullptr);
		handle = VK_NULL_HANDLE;
	}
}

CommandQueueVulkan::CommandQueueVulkan()
{

}

CommandQueueVulkan::~CommandQueueVulkan()
{

}

CommandPoolVulkan::CommandPoolVulkan()
{

}

CommandPoolVulkan::~CommandPoolVulkan()
{

}

CommandBufferVulkan::CommandBufferVulkan()
{

}

CommandBufferVulkan::~CommandBufferVulkan()
{

}

void CommandBufferVulkan::Begin()
{
	HS_ASSERT(false == _isBegan, "CommandBuffer has already began.");
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pNext = nullptr;
	beginInfo.pInheritanceInfo = nullptr; //TODO: Secondary Buffer 고려

	vkBeginCommandBuffer(handle, &beginInfo);
	_isBegan = true;
}

void CommandBufferVulkan::End()
{
	HS_ASSERT(_isBegan, "CommandBuffer has not began");
	vkEndCommandBuffer(handle);

	_isBegan = false;
}

void CommandBufferVulkan::Reset()
{
	vkResetCommandBuffer(handle, 0);
	_isBegan = false;
}

void CommandBufferVulkan::BeginRenderPass(RenderPass* renderPass, Framebuffer* framebuffer, const Area& renderArea)
{
	static std::vector<VkClearValue> clearValues;

	HS_ASSERT(renderPass && framebuffer, "both renderPass and framebuffer should't be nullptr");
	HS_ASSERT(_isBegan, "CommandBuffer has not began");
	HS_ASSERT(_isGraphicsBegan == false, "Graphics Pass is already began");
	HS_ASSERT(_isComputeBegan == false, "Compute Pass is aready began");
	HS_ASSERT(_isBlitBegan == false, "Blit Pass is already began");

	RenderPassVulkan* renderPassVK = static_cast<RenderPassVulkan*>(renderPass);
	const RenderPassInfo& renderPassInfo = renderPassVK->info;
	FramebufferVulkan* framebufferVK = static_cast<FramebufferVulkan*>(framebuffer);
	const FramebufferInfo& framebufferInfo = framebufferVK->info;

	HS_ASSERT(framebufferInfo.renderPass == renderPass, "RenderPass and Framebuffer are not matched.");

	uint8 attachmentCount = static_cast<uint8>(renderPassInfo.colorAttachmentCount) + static_cast<uint8>(renderPassInfo.useDepthStencilAttachment);
	if (clearValues.size() < attachmentCount)
	{
		clearValues.resize(attachmentCount);
	}

	size_t attachmentIndex = 0;
	for (; attachmentIndex < renderPassInfo.colorAttachmentCount; attachmentIndex++)
	{
		::memcpy(clearValues[attachmentIndex].color.float32, renderPassInfo.colorAttachments[attachmentIndex].clearValue.color, sizeof(float[4]));
	}

	if (renderPassInfo.useDepthStencilAttachment)
	{
		clearValues[attachmentIndex].depthStencil.depth = renderPassInfo.depthStencilAttachment.clearValue.depthStencil.depth;
		clearValues[attachmentIndex].depthStencil.stencil = renderPassInfo.depthStencilAttachment.clearValue.depthStencil.stencil;
		attachmentIndex++;
	}

	VkRect2D area{};
	area.offset.x = renderArea.x;
	area.offset.y = renderArea.y;
	area.extent.width = renderArea.width;
	area.extent.height = renderArea.height;

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.clearValueCount = attachmentCount;
	beginInfo.pClearValues = clearValues.data();
	beginInfo.renderArea = area;
	beginInfo.renderPass = renderPassVK->handle;
	beginInfo.framebuffer = framebufferVK->handle;
	beginInfo.pNext = nullptr;

	VkSubpassBeginInfo subpassBeginInfo{};

	vkCmdBeginRenderPass(handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	_isGraphicsBegan = true;
	_isComputeBegan = false;
	_isBlitBegan = false;
}

void CommandBufferVulkan::BindPipeline(GraphicsPipeline* pipeline)
{
	HS_ASSERT(_isGraphicsBegan && _isBegan, "RenderPass has not begun");
	HS_ASSERT(pipeline, "Pipeline is nullptr");
	GraphicsPipelineVulkan* pipelineVK = static_cast<GraphicsPipelineVulkan*>(pipeline);
	vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineVK->handle);
}

void CommandBufferVulkan::BindResourceSet(ResourceSet* rSet)
{
	// TODO: Implementation it.
}

void CommandBufferVulkan::SetViewport(const Viewport& viewport)
{
	HS_ASSERT(_isGraphicsBegan && _isBegan, "RenderPass has not begun");
	VkViewport viewportVk{};
	viewportVk.x = viewport.x;
	viewportVk.y = viewport.y;
	viewportVk.width = viewport.width;
	viewportVk.height = viewport.height;
	viewportVk.minDepth = viewport.zNear;
	viewportVk.maxDepth = viewport.zFar;

	vkCmdSetViewport(handle, 0, 1, &viewportVk);
}

void CommandBufferVulkan::SetScissor(const uint32 x, const uint32 y, const uint32 width, const uint32 height)
{
	HS_ASSERT(_isGraphicsBegan && _isBegan, "RenderPass has not begun");
	VkRect2D rectVk{};
	rectVk.extent.width = width;
	rectVk.extent.height = height;
	rectVk.offset.x = x;
	rectVk.offset.y = y;
	vkCmdSetScissor(handle, 0, 1, &rectVk);
}

void CommandBufferVulkan::BindIndexBuffer(Buffer* indexBuffer)
{
	HS_ASSERT(_isGraphicsBegan && _isBegan, "RenderPass has not begun");
	HS_ASSERT(indexBuffer, "Index Buffer is nullptr");

	BufferVulkan* indexBufferVK = static_cast<BufferVulkan*>(indexBuffer);


}

void CommandBufferVulkan::BindVertexBuffers(const Buffer* const* vertexBuffers, const uint32* offsets, const uint8 bufferCount)
{
	std::vector<VkBuffer> vertexBufferHandles(bufferCount);
	std::vector<VkDeviceSize> vertexOffsets(bufferCount);
	for (uint8 i = 0; i < bufferCount; i++)
	{
		HS_ASSERT(vertexBuffers[i], "Vertex Buffer is nullptr at index %d", i);
		const BufferVulkan* vertexBufferVK = static_cast<const BufferVulkan*>(vertexBuffers[i]);
		vertexBufferHandles[i] = vertexBufferVK->handle;
		vertexOffsets[i] = static_cast<VkDeviceSize>(offsets[i]);
	}

	vkCmdBindVertexBuffers(handle, 0, bufferCount, vertexBufferHandles.data(), vertexOffsets.data());
}

void CommandBufferVulkan::DrawArrays(const uint32 firstVertex, const uint32 vertexCount, const uint32 instanceCount)
{
	vkCmdDraw(handle, vertexCount, instanceCount, firstVertex, 0);
}

void CommandBufferVulkan::DrawIndexed(const uint32 firstIndex, const uint32 indexCount, const uint32 instanceCount, const uint32 vertexOffset)
{
	// TODO:
}

void CommandBufferVulkan::EndRenderPass()
{
	HS_ASSERT(_isGraphicsBegan && _isBegan, "RenderPass has not begun");
	vkCmdEndRenderPass(handle);
	_isGraphicsBegan = false;
}

void CommandBufferVulkan::CopyTexture(Texture* srcTexture, Texture* dstTexture)
{

}

void CommandBufferVulkan::UpdateBuffer(Buffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize)
{

}

void CommandBufferVulkan::PushDebugMark(const char* label, float color[4])
{

}

void CommandBufferVulkan::PopDebugMark()
{

}



HS_NS_END