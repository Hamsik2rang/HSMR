#include "RHI/Vulkan/VulkanCommandHandle.h"

#include "RHI/Vulkan/VulkanUtility.h"
#include "RHI/Vulkan/VulkanContext.h"
#include "RHI/Vulkan/VulkanRenderHandle.h"
#include "RHI/Vulkan/VulkanResourceHandle.h"

HS_NS_BEGIN

namespace
{
struct VulkanTextureStateInfo
{
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkAccessFlags access = 0;
};

VulkanTextureStateInfo getTextureStateInfo(ERHITextureState state)
{
    switch (state)
    {
    case ERHITextureState::ShaderRead:
        return {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT};
    case ERHITextureState::ColorAttachmentWrite:
        return {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};
    case ERHITextureState::DepthAttachmentRead:
        return {VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT};
    case ERHITextureState::DepthAttachmentWrite:
        return {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};
    case ERHITextureState::StorageReadWrite:
        return {VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT};
    case ERHITextureState::TransferRead:
        return {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT};
    case ERHITextureState::TransferWrite:
        return {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT};
    case ERHITextureState::Present:
        return {VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0};
    case ERHITextureState::Undefined:
    default:
        return {};
    }
}

VkImageAspectFlags getAspectMask(const TextureInfo& info)
{
    switch (info.format)
    {
    case EPixelFormat::Depth32:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case EPixelFormat::Stencil8:
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    case EPixelFormat::Depth24Stencil8:
    case EPixelFormat::Depth32Stencil8:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}
}

VulkanCommandQueue::VulkanCommandQueue(const char* name)
    : RHICommandQueue(name)
{
}

VulkanCommandQueue::~VulkanCommandQueue()
{
}

VulkanCommandPool::VulkanCommandPool(const char* name)
    : RHICommandPool(name)
{
}

VulkanCommandPool::~VulkanCommandPool()
{
}

VulkanCommandBuffer::VulkanCommandBuffer(const char* name, VulkanContext* context)
    : RHICommandBuffer(name)
    , _context(context)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
}

void VulkanCommandBuffer::Begin()
{
    HS_ASSERT(false == _isBegan, "CommandBuffer has already began.");
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pNext            = nullptr;
    beginInfo.pInheritanceInfo = nullptr; // TODO: Secondary Buffer 고려

    vkBeginCommandBuffer(handle, &beginInfo);
    _isBegan = true;
}

void VulkanCommandBuffer::End()
{
    HS_ASSERT(_isBegan, "CommandBuffer has not began");
    vkEndCommandBuffer(handle);

    _isBegan = false;
}

void VulkanCommandBuffer::Reset()
{
    vkResetCommandBuffer(handle, 0);
    _isBegan = false;
}

void VulkanCommandBuffer::BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer, const Area& renderArea)
{
    static std::vector<VkClearValue> clearValues;

    HS_ASSERT(renderPass && framebuffer, "both renderPass and framebuffer should't be nullptr");
    HS_ASSERT(_isBegan, "CommandBuffer has not began");
    HS_ASSERT(_isGraphicsBegan == false, "Graphics Pass is already began");
    HS_ASSERT(_isComputeBegan == false, "Compute Pass is aready began");
    HS_ASSERT(_isBlitBegan == false, "Blit Pass is already began");

    VulkanRenderPass* renderPassVK         = static_cast<VulkanRenderPass*>(renderPass);
    const RenderPassInfo& renderPassInfo   = renderPassVK->info;
    VulkanFramebuffer* framebufferVK       = static_cast<VulkanFramebuffer*>(framebuffer);
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
        clearValues[attachmentIndex].depthStencil.depth   = renderPassInfo.depthStencilAttachment.clearValue.depthStencil.depth;
        clearValues[attachmentIndex].depthStencil.stencil = renderPassInfo.depthStencilAttachment.clearValue.depthStencil.stencil;
        attachmentIndex++;
    }

    VkRect2D area{};
    area.offset.x      = renderArea.x;
    area.offset.y      = renderArea.y;
    area.extent.width  = renderArea.width;
    area.extent.height = renderArea.height;

    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.clearValueCount = attachmentCount;
    beginInfo.pClearValues    = clearValues.data();
    beginInfo.renderArea      = area;
    beginInfo.renderPass      = renderPassVK->handle;
    beginInfo.framebuffer     = framebufferVK->handle;
    beginInfo.pNext           = nullptr;

    VkSubpassBeginInfo subpassBeginInfo{};

    vkCmdBeginRenderPass(handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    _isGraphicsBegan = true;
    _isComputeBegan  = false;
    _isBlitBegan     = false;
}

void VulkanCommandBuffer::BindPipeline(RHIGraphicsPipeline* pipeline)
{
    HS_ASSERT(_isGraphicsBegan && _isBegan, "RenderPass has not begun");
    HS_ASSERT(pipeline, "Pipeline is nullptr");
    VulkanGraphicsPipeline* pipelineVK = static_cast<VulkanGraphicsPipeline*>(pipeline);
    curGraphicsPipeline                = pipelineVK->handle;
    curGraphicsPipelineLayout          = pipelineVK->layout;
    vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineVK->handle);
}

void VulkanCommandBuffer::BindResourceSet(RHIResourceSet* rSet)
{
    HS_ASSERT(_isGraphicsBegan && _isBegan, "RenderPass has not begun");
    HS_ASSERT(rSet, "Resource set is nullptr");

    VulkanResourceSet* rSetVK = static_cast<VulkanResourceSet*>(rSet);
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, curGraphicsPipelineLayout, 0, 1, &rSetVK->handle, 0, nullptr);
}

void VulkanCommandBuffer::SetViewport(const Viewport& viewport)
{
    HS_ASSERT(_isGraphicsBegan && _isBegan, "RenderPass has not begun");
    VkViewport viewportVk{};
    viewportVk.x        = viewport.x;
    viewportVk.y        = viewport.y;
    viewportVk.width    = viewport.width;
    viewportVk.height   = viewport.height;
    viewportVk.minDepth = viewport.zNear;
    viewportVk.maxDepth = viewport.zFar;

    vkCmdSetViewport(handle, 0, 1, &viewportVk);
}

void VulkanCommandBuffer::SetScissor(const uint32 x, const uint32 y, const uint32 width, const uint32 height)
{
    HS_ASSERT(_isGraphicsBegan && _isBegan, "RenderPass has not begun");
    VkRect2D rectVk{};
    rectVk.extent.width  = width;
    rectVk.extent.height = height;
    rectVk.offset.x      = x;
    rectVk.offset.y      = y;
    vkCmdSetScissor(handle, 0, 1, &rectVk);
}

void VulkanCommandBuffer::BindIndexBuffer(RHIBuffer* indexBuffer)
{
    HS_ASSERT(_isGraphicsBegan && _isBegan, "RenderPass has not begun");
    HS_ASSERT(indexBuffer, "Index Buffer is nullptr");

    VulkanBuffer* indexBufferVK = static_cast<VulkanBuffer*>(indexBuffer);

    vkCmdBindIndexBuffer(handle, indexBufferVK->handle, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
}

void VulkanCommandBuffer::BindVertexBuffers(const RHIBuffer* const* vertexBuffers, const uint32* offsets, const uint8 bufferCount)
{
    std::vector<VkBuffer> vertexBufferHandles(bufferCount);
    std::vector<VkDeviceSize> vertexOffsets(bufferCount);
    for (uint8 i = 0; i < bufferCount; i++)
    {
        HS_ASSERT(vertexBuffers[i], "Vertex Buffer is nullptr at index %d", i);
        const VulkanBuffer* vertexBufferVK = static_cast<const VulkanBuffer*>(vertexBuffers[i]);
        vertexBufferHandles[i]             = vertexBufferVK->handle;
        vertexOffsets[i]                   = static_cast<VkDeviceSize>(offsets[i]);
    }

    vkCmdBindVertexBuffers(handle, 0, bufferCount, vertexBufferHandles.data(), vertexOffsets.data());
}

void VulkanCommandBuffer::DrawArrays(const uint32 firstVertex, const uint32 vertexCount, const uint32 instanceCount)
{
    vkCmdDraw(handle, vertexCount, instanceCount, firstVertex, 0);
}

void VulkanCommandBuffer::DrawIndexed(const uint32 firstIndex, const uint32 indexCount, const uint32 instanceCount, const uint32 vertexOffset)
{
    vkCmdDrawIndexed(handle, indexCount, instanceCount, firstIndex, vertexOffset, 0);
}

void VulkanCommandBuffer::EndRenderPass()
{
    HS_ASSERT(_isGraphicsBegan && _isBegan, "RenderPass has not begun");
    vkCmdEndRenderPass(handle);
    _isGraphicsBegan = false;
}

void VulkanCommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
{
    HS_ASSERT(_isBegan, "CommandBuffer has not began");
    HS_ASSERT(_isGraphicsBegan == false, "Graphics Pass is already began");
    HS_ASSERT(_isComputeBegan == false, "Compute Pass is aready began");
    HS_ASSERT(_isBlitBegan == false, "Blit Pass is already began");
    HS_ASSERT(_context, "VulkanContext is nullptr");

    _useDynamicRendering = _context->GetCapabilities().renderingPath == ERHIRenderingPath::DynamicRendering;
    _currentRenderingInfo = renderingInfo;
    _context->CmdBeginRendering(handle, renderingInfo);

    _isGraphicsBegan = true;
    _isComputeBegan  = false;
    _isBlitBegan     = false;
}

void VulkanCommandBuffer::EndRendering()
{
    HS_ASSERT(_isGraphicsBegan && _isBegan, "Rendering has not begun");
    HS_ASSERT(_context, "VulkanContext is nullptr");
    _context->CmdEndRendering(handle, _currentRenderingInfo);
    _currentRenderingInfo = RenderingInfo{};
    _isGraphicsBegan = false;
}

void VulkanCommandBuffer::CopyTexture(RHITexture* srcTexture, RHITexture* dstTexture)
{
}

void VulkanCommandBuffer::UpdateBuffer(RHIBuffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize)
{
    HS_ASSERT(buffer, "Buffer is nullptr");
    HS_ASSERT(srcData, "Source data is nullptr");
    HS_ASSERT(dataSize > 0, "Data size must be greater than 0");
    HS_ASSERT(dataSize <= 65536, "vkCmdUpdateBuffer is limited to 65536 bytes");

    VulkanBuffer* bufferVK = static_cast<VulkanBuffer*>(buffer);

    // vkCmdUpdateBuffer updates buffer contents inline within the command buffer
    // Note: dataSize must be less than or equal to 65536 bytes
    // Note: dstOffset and dataSize must be multiples of 4
    vkCmdUpdateBuffer(handle, bufferVK->handle, static_cast<VkDeviceSize>(dstOffset), static_cast<VkDeviceSize>(dataSize), srcData);
}

void VulkanCommandBuffer::PushDebugMark(const char* label, float color[4])
{
}

void VulkanCommandBuffer::PopDebugMark()
{
}

void VulkanCommandBuffer::BindComputePipeline(RHIComputePipeline* pipeline)
{
    HS_ASSERT(_isBegan, "CommandBuffer has not began");
    HS_ASSERT(pipeline, "Compute Pipeline is nullptr");

    VulkanComputePipeline* pipelineVK = static_cast<VulkanComputePipeline*>(pipeline);
    curComputePipeline                = pipelineVK->handle;
    curComputePipelineLayout          = pipelineVK->layout;

    vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineVK->handle);

    _isComputeBegan  = true;
    _isGraphicsBegan = false;
}

void VulkanCommandBuffer::BindComputeResourceSet(RHIResourceSet* rSet)
{
    HS_ASSERT(_isBegan, "CommandBuffer has not began");
    HS_ASSERT(_isComputeBegan, "Compute pipeline is not bound");
    HS_ASSERT(rSet, "Resource set is nullptr");

    VulkanResourceSet* rSetVK = static_cast<VulkanResourceSet*>(rSet);
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_COMPUTE, curComputePipelineLayout, 0, 1, &rSetVK->handle, 0, nullptr);
}

void VulkanCommandBuffer::Dispatch(uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ)
{
    HS_ASSERT(_isBegan, "CommandBuffer has not began");
    HS_ASSERT(_isComputeBegan, "Compute pipeline is not bound");

    vkCmdDispatch(handle, groupCountX, groupCountY, groupCountZ);
}

void VulkanCommandBuffer::EndComputePass()
{
    HS_ASSERT(_isBegan, "CommandBuffer has not began");

    curComputePipeline       = VK_NULL_HANDLE;
    curComputePipelineLayout = VK_NULL_HANDLE;
    _isComputeBegan          = false;
}

void VulkanCommandBuffer::TextureBarrier(const RHITextureBarrierDesc* barriers, uint32 count)
{
    HS_ASSERT(_isBegan, "CommandBuffer has not began");
    if (barriers == nullptr || count == 0)
    {
        return;
    }

    std::vector<VkImageMemoryBarrier> imageBarriers;
    imageBarriers.reserve(count);
    VkPipelineStageFlags srcStages = 0;
    VkPipelineStageFlags dstStages = 0;

    for (uint32 i = 0; i < count; i++)
    {
        const RHITextureBarrierDesc& desc = barriers[i];
        if (desc.texture == nullptr || desc.before == desc.after)
        {
            continue;
        }

        VulkanTexture* textureVK = static_cast<VulkanTexture*>(desc.texture);
        VulkanTextureStateInfo beforeInfo = getTextureStateInfo(desc.before);
        VulkanTextureStateInfo afterInfo = getTextureStateInfo(desc.after);
        VkImageLayout oldLayout = textureVK->layoutVk != VK_IMAGE_LAYOUT_UNDEFINED ? textureVK->layoutVk : beforeInfo.layout;
        if (oldLayout == afterInfo.layout)
        {
            continue;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = beforeInfo.access;
        barrier.dstAccessMask = afterInfo.access;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = afterInfo.layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = textureVK->handle;
        barrier.subresourceRange.aspectMask = getAspectMask(textureVK->info);
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = textureVK->info.mipLevel;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = textureVK->info.arrayLength;

        imageBarriers.push_back(barrier);
        srcStages |= beforeInfo.stage;
        dstStages |= afterInfo.stage;
        textureVK->layoutVk = afterInfo.layout;
    }

    if (imageBarriers.empty())
    {
        return;
    }

    vkCmdPipelineBarrier(
        handle,
        srcStages == 0 ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : srcStages,
        dstStages == 0 ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : dstStages,
        0,
        0, nullptr,
        0, nullptr,
        static_cast<uint32>(imageBarriers.size()), imageBarriers.data()
    );
}

void VulkanCommandBuffer::TextureBarrier(RHITexture* texture)
{
    RHITextureBarrierDesc barrier{};
    barrier.texture = texture;
    barrier.before = ERHITextureState::StorageReadWrite;
    barrier.after = ERHITextureState::ShaderRead;
    TextureBarrier(&barrier, 1);
}

HS_NS_END
