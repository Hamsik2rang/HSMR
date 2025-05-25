#include "Engine/RHI/Vulkan/CommandHandleVulkan.h"

#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"
#include "Engine/RHI/Vulkan/RenderHandleVulkan.h"

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

CommandPoolVulkan::CommandPoolVulkan(RHIDeviceVulkan device)
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

void CommandBufferVulkan::BeginRenderPass(RenderPass* renderPass, Framebuffer* framebuffer)
{
	static std::vector<VkClearValue> clearValues;

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
	area.offset.x = renderPassInfo.renderArea.x;
	area.offset.y = renderPassInfo.renderArea.y;
	area.extent.width = renderPassInfo.renderArea.width;
	area.extent.height = renderPassInfo.renderArea.height;

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

}

void CommandBufferVulkan::BindResourceSet(ResourceSet* rSet)
{

}

void CommandBufferVulkan::SetViewport(const Viewport& viewport)
{

}

void CommandBufferVulkan::SetScissor(const uint32 x, const uint32 y, const uint32 width, const uint32 height)
{

}

void CommandBufferVulkan::BindIndexBuffer(Buffer* indexBuffer)
{

}

void CommandBufferVulkan::BindVertexBuffers(const Buffer* const* vertexBuffers, const uint32* offsets, const uint8 bufferCount)
{

}

void CommandBufferVulkan::DrawArrays(const uint32 firstVertex, const uint32 vertexCount, const uint32 instanceCount)
{

}

void CommandBufferVulkan::DrawIndexed(const uint32 firstIndex, const uint32 indexCount, const uint32 instanceCount, const uint32 vertexOffset)
{

}

void CommandBufferVulkan::EndRenderPass()
{

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