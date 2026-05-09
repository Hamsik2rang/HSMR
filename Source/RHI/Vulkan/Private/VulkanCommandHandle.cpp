#include "RHI/Vulkan/VulkanCommandHandle.h"

#include "RHI/Vulkan/VulkanUtility.h"
#include "RHI/Vulkan/VulkanContext.h"
#include "RHI/Vulkan/VulkanRenderHandle.h"
#include "RHI/Vulkan/VulkanResourceHandle.h"
#include "RHI/Vulkan/VulkanDevice.h"

HS_NS_BEGIN

namespace
{
struct VulkanTextureStateInfo
{
    VkImageLayout layout       = VK_IMAGE_LAYOUT_UNDEFINED;
    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkAccessFlags access       = 0;
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

} // namespace

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
    , _rhiContext(context)
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

void VulkanCommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
{
    HS_ASSERT(_isBegan, "CommandBuffer has not began");
    HS_ASSERT(_isGraphicsBegan == false, "Graphics Pass is already began");
    HS_ASSERT(_isComputeBegan == false, "Compute Pass is aready began");
    HS_ASSERT(_isBlitBegan == false, "Blit Pass is already began");
    HS_ASSERT(_rhiContext, "VulkanContext is nullptr");

    bool useDynamicRendering     = _rhiContext->GetCapabilities().renderingPath == ERHIRenderingPath::DynamicRendering;
    _currentRenderingInfo        = renderingInfo;
    const VulkanDevice* deviceVK = _rhiContext->GetDevice();
    if (deviceVK->GetCapabilities().renderingPath == ERHIRenderingPath::LegacyRenderPass)
    {
        VulkanRenderingCache::LegacyRenderingHandles handles = _rhiContext->GetRenderingCache()->GetLegacyRenderingHandles(renderingInfo);

        static std::vector<VkClearValue> clearValues;
        uint32 attachmentCount = renderingInfo.colorAttachmentCount + static_cast<uint32>(renderingInfo.useDepthStencilAttachment);
        if (clearValues.size() < attachmentCount)
        {
            clearValues.resize(attachmentCount);
        }

        uint32 attachmentIndex = 0;
        for (; attachmentIndex < renderingInfo.colorAttachmentCount; attachmentIndex++)
        {
            ::memcpy(clearValues[attachmentIndex].color.float32, renderingInfo.colorAttachments[attachmentIndex].attachment.clearValue.color, sizeof(float[4]));
        }

        if (renderingInfo.useDepthStencilAttachment)
        {
            clearValues[attachmentIndex].depthStencil.depth   = renderingInfo.depthStencilAttachment.attachment.clearValue.depthStencil.depth;
            clearValues[attachmentIndex].depthStencil.stencil = renderingInfo.depthStencilAttachment.attachment.clearValue.depthStencil.stencil;
        }

        VkRect2D area{};
        area.offset.x      = renderingInfo.renderArea.x;
        area.offset.y      = renderingInfo.renderArea.y;
        area.extent.width  = renderingInfo.renderArea.width;
        area.extent.height = renderingInfo.renderArea.height;

        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.renderPass      = handles.renderPass;
        beginInfo.framebuffer     = handles.framebuffer;
        beginInfo.renderArea      = area;
        beginInfo.clearValueCount = attachmentCount;
        beginInfo.pClearValues    = clearValues.data();

        vkCmdBeginRenderPass(handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
        return;
    }

    auto fnGetSrcStageAndAccess = [](VkImageLayout layout) -> std::pair<VkPipelineStageFlags, VkAccessFlags>
    {
        switch (layout)
        {
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:         return {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT};
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:         return {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return {VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};
        default:                                               return {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0};
        }
    };

    std::vector<VkRenderingAttachmentInfo> colorAttachments(renderingInfo.colorAttachmentCount);
    for (uint32 i = 0; i < renderingInfo.colorAttachmentCount; i++)
    {
        const RenderingAttachmentInfo& attachmentInfo = renderingInfo.colorAttachments[i];
        VulkanTexture* textureVK                      = static_cast<VulkanTexture*>(attachmentInfo.texture);

        if (renderingInfo.enableAutomaticTransitions)
        {
            auto [srcStage, srcAccess] = fnGetSrcStageAndAccess(textureVK->layoutVk);
            VulkanUtility::TransitionImageLayout(
                handle,
                textureVK,
                textureVK->layoutVk,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                srcStage,
                srcAccess,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
        }

        colorAttachments[i].sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachments[i].imageView   = textureVK->imageViewVk;
        colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachments[i].loadOp      = VulkanUtility::ToLoadOp(attachmentInfo.attachment.loadAction);
        colorAttachments[i].storeOp     = VulkanUtility::ToStoreOp(attachmentInfo.attachment.storeAction);
        ::memcpy(colorAttachments[i].clearValue.color.float32, attachmentInfo.attachment.clearValue.color, sizeof(float[4]));
    }

    VkRenderingAttachmentInfo depthAttachment{};
    if (renderingInfo.useDepthStencilAttachment)
    {
        VulkanTexture* textureVK = static_cast<VulkanTexture*>(renderingInfo.depthStencilAttachment.texture);
        if (renderingInfo.enableAutomaticTransitions)
        {
            auto [srcStage, stcAccess] = fnGetSrcStageAndAccess(textureVK->layoutVk);
            VulkanUtility::TransitionImageLayout(
                handle,
                textureVK,
                textureVK->layoutVk,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                srcStage,
                stcAccess,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT);
        }

        depthAttachment.sType                           = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView                       = textureVK->imageViewVk;
        depthAttachment.imageLayout                     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp                          = VulkanUtility::ToLoadOp(renderingInfo.depthStencilAttachment.attachment.loadAction);
        depthAttachment.storeOp                         = VulkanUtility::ToStoreOp(renderingInfo.depthStencilAttachment.attachment.storeAction);
        depthAttachment.clearValue.depthStencil.depth   = renderingInfo.depthStencilAttachment.attachment.clearValue.depthStencil.depth;
        depthAttachment.clearValue.depthStencil.stencil = renderingInfo.depthStencilAttachment.attachment.clearValue.depthStencil.stencil;
    }

    VkRenderingInfo vkRenderingInfo{};
    vkRenderingInfo.sType                    = VK_STRUCTURE_TYPE_RENDERING_INFO;
    vkRenderingInfo.renderArea.offset.x      = renderingInfo.renderArea.x;
    vkRenderingInfo.renderArea.offset.y      = renderingInfo.renderArea.y;
    vkRenderingInfo.renderArea.extent.width  = renderingInfo.renderArea.width;
    vkRenderingInfo.renderArea.extent.height = renderingInfo.renderArea.height;
    vkRenderingInfo.layerCount               = 1;
    vkRenderingInfo.colorAttachmentCount     = renderingInfo.colorAttachmentCount;
    vkRenderingInfo.pColorAttachments        = colorAttachments.data();
    vkRenderingInfo.pDepthAttachment         = renderingInfo.useDepthStencilAttachment ? &depthAttachment : nullptr;
    vkRenderingInfo.pStencilAttachment       = nullptr;

    static PFN_vkCmdBeginRendering beginRendering = reinterpret_cast<PFN_vkCmdBeginRendering>(vkGetDeviceProcAddr(deviceVK->logicalDevice, "vkCmdBeginRendering"));
    if (beginRendering == nullptr)
    {
        beginRendering = reinterpret_cast<PFN_vkCmdBeginRendering>(vkGetDeviceProcAddr(deviceVK->logicalDevice, "vkCmdBeginRenderingKHR"));
    }
    HS_ASSERT(beginRendering != nullptr, "Dynamic rendering function is not available");
    beginRendering(handle, &vkRenderingInfo);

    _isGraphicsBegan = true;
    _isComputeBegan  = false;
    _isBlitBegan     = false;
}

void VulkanCommandBuffer::EndRendering()
{
    HS_ASSERT(_isGraphicsBegan && _isBegan, "Rendering has not begun");
    HS_ASSERT(_rhiContext, "VulkanContext is nullptr");

    VulkanDevice* deviceVK = _rhiContext->GetDevice();

    if (deviceVK->GetCapabilities().renderingPath == ERHIRenderingPath::LegacyRenderPass)
    {
        vkCmdEndRenderPass(handle);
        return;
    }

    static PFN_vkCmdEndRendering vkCmdEndRendering = reinterpret_cast<PFN_vkCmdEndRendering>(vkGetDeviceProcAddr(deviceVK->logicalDevice, "vkCmdEndRendering"));
    if (vkCmdEndRendering == nullptr)
    {
        vkCmdEndRendering = reinterpret_cast<PFN_vkCmdEndRendering>(vkGetDeviceProcAddr(deviceVK->logicalDevice, "vkCmdEndRenderingKHR"));
    }
    HS_ASSERT(vkCmdEndRendering != nullptr, "Dynamic rendering function is not available");
    vkCmdEndRendering(handle);

    if (_currentRenderingInfo.enableAutomaticTransitions)
    {
        VkImageLayout colorFinalLayout = _currentRenderingInfo.isSwapchainRendering ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        for (const RenderingAttachmentInfo& attachmentInfo : _currentRenderingInfo.colorAttachments)
        {
            VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(attachmentInfo.texture);
            VulkanUtility::TransitionImageLayout(
                handle,
                vulkanTexture,
                vulkanTexture->layoutVk,
                colorFinalLayout,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
        }

        if (_currentRenderingInfo.useDepthStencilAttachment)
        {
            VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(_currentRenderingInfo.depthStencilAttachment.texture);
            VulkanUtility::TransitionImageLayout(
                handle,
                vulkanTexture,
                vulkanTexture->layoutVk,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT);
        }
    }

    _currentRenderingInfo = {};
    _isGraphicsBegan      = false;
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
#ifdef _DEBUG
    // VK_EXT_debug_marker는 deprecated이고 모던 driver에서 노출되지 않는 경우가 많아
    // 표준 대체인 VK_EXT_debug_utils의 BeginDebugUtilsLabel을 사용한다.
    // debug_utils가 enable되지 않은 환경(release-strip된 SDK 등)에서는 silently skip.
    static PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT =
        reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
            vkGetInstanceProcAddr(_rhiContext->GetInstance(), "vkCmdBeginDebugUtilsLabelEXT"));
    if (!vkCmdBeginDebugUtilsLabelEXT)
    {
        return;
    }

    VkDebugUtilsLabelEXT info{};
    info.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    info.pLabelName = label;
    ::memcpy(info.color, color, sizeof(float) * 4);

    vkCmdBeginDebugUtilsLabelEXT(handle, &info);
#endif
}

void VulkanCommandBuffer::PopDebugMark()
{
#ifdef _DEBUG
    static PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT =
        reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
            vkGetInstanceProcAddr(_rhiContext->GetInstance(), "vkCmdEndDebugUtilsLabelEXT"));
    if (!vkCmdEndDebugUtilsLabelEXT)
    {
        return;
    }
    vkCmdEndDebugUtilsLabelEXT(handle);
#endif
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

        VulkanTexture* textureVK          = static_cast<VulkanTexture*>(desc.texture);
        VulkanTextureStateInfo beforeInfo = getTextureStateInfo(desc.before);
        VulkanTextureStateInfo afterInfo  = getTextureStateInfo(desc.after);
        VkImageLayout oldLayout           = textureVK->layoutVk;
        if (oldLayout == afterInfo.layout)
        {
            continue;
        }
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
        {
            beforeInfo.stage  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            beforeInfo.access = 0;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask                   = beforeInfo.access;
        barrier.dstAccessMask                   = afterInfo.access;
        barrier.oldLayout                       = oldLayout;
        barrier.newLayout                       = afterInfo.layout;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = textureVK->handle;
        barrier.subresourceRange.aspectMask     = VulkanUtility::GetImageAspectMask(textureVK->info);
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = VulkanUtility::GetTextureMipLevelCount(textureVK->info);
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = VulkanUtility::GetTextureLayerCount(textureVK->info);

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
        static_cast<uint32>(imageBarriers.size()), imageBarriers.data());
}

void VulkanCommandBuffer::TextureBarrier(RHITexture* texture)
{
    RHITextureBarrierDesc barrier{};
    barrier.texture = texture;
    barrier.before  = ERHITextureState::StorageReadWrite;
    barrier.after   = ERHITextureState::ShaderRead;
    TextureBarrier(&barrier, 1);
}

HS_NS_END
