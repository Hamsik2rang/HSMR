#include "RHI/Vulkan/VulkanRenderingCache.h"

#include "RHI/Vulkan/VulkanContext.h"
#include "RHI/Vulkan/VulkanResourceHandle.h"
#include "RHI/Vulkan/VulkanUtility.h"

HS_NS_BEGIN

static VkSampleCountFlagBits toSampleCount(uint8 sampleCount)
{
    return sampleCount == 0 ? VK_SAMPLE_COUNT_1_BIT : static_cast<VkSampleCountFlagBits>(sampleCount);
}

VulkanRenderingCache::~VulkanRenderingCache()
{
    Finalize();
}

void VulkanRenderingCache::Initialize(VulkanContext* context, VulkanDevice* device)
{
    _context = context;
    _device = device;
}

void VulkanRenderingCache::Finalize()
{
    if (_device == nullptr || _device->logicalDevice == VK_NULL_HANDLE)
    {
        return;
    }

    Reset();
}

void VulkanRenderingCache::Reset()
{
    if (_device == nullptr || _device->logicalDevice == VK_NULL_HANDLE)
    {
        _renderPassCache.clear();
        _compatibleRenderPassCache.clear();
        _framebufferCache.clear();
        return;
    }

    for (auto& elem : _framebufferCache)
    {
        if (elem.second != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(_device->logicalDevice, elem.second, nullptr);
        }
    }
    _framebufferCache.clear();

    for (auto& elem : _renderPassCache)
    {
        if (elem.second != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(_device->logicalDevice, elem.second, nullptr);
        }
    }
    _renderPassCache.clear();

    for (auto& elem : _compatibleRenderPassCache)
    {
        if (elem.second != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(_device->logicalDevice, elem.second, nullptr);
        }
    }
    _compatibleRenderPassCache.clear();
}

VkRenderPass VulkanRenderingCache::GetCompatibleRenderPass(const PipelineRenderTargetLayout& layout)
{
    size_t key = std::hash<PipelineRenderTargetLayout>{}(layout);
    auto it = _compatibleRenderPassCache.find(key);
    if (it != _compatibleRenderPassCache.end())
    {
        return it->second;
    }

    LegacyRenderPassInfo info = makeCompatibleRenderPassInfo(layout);
    VkRenderPass renderPass = createRenderPass(info);
    _compatibleRenderPassCache[key] = renderPass;
    return renderPass;
}

VulkanRenderingCache::LegacyRenderingHandles VulkanRenderingCache::GetLegacyRenderingHandles(const RenderingInfo& renderingInfo)
{
    LegacyRenderingHandles handles{};

    LegacyRenderPassInfo renderPassInfo = makeRenderPassInfo(renderingInfo);
    size_t renderPassKey = makeRenderPassKey(renderPassInfo);
    auto renderPassIt = _renderPassCache.find(renderPassKey);
    if (renderPassIt != _renderPassCache.end())
    {
        handles.renderPass = renderPassIt->second;
    }
    else
    {
        handles.renderPass = createRenderPass(renderPassInfo);
        _renderPassCache[renderPassKey] = handles.renderPass;
    }

    size_t framebufferKey = makeFramebufferKey(handles.renderPass, renderingInfo);
    auto framebufferIt = _framebufferCache.find(framebufferKey);
    if (framebufferIt != _framebufferCache.end())
    {
        handles.framebuffer = framebufferIt->second;
    }
    else
    {
        handles.framebuffer = createFramebuffer(handles.renderPass, renderingInfo);
        _framebufferCache[framebufferKey] = handles.framebuffer;
    }

    return handles;
}

VkRenderPass VulkanRenderingCache::createRenderPass(const LegacyRenderPassInfo& info)
{
    uint32 attachmentCount = info.colorAttachmentCount + static_cast<uint32>(info.useDepthStencilAttachment);

    VkImageLayout colorFinalLayout = info.isSwapchainRenderPass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkImageLayout depthStencilFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    std::vector<VkAttachmentDescription> attachments(attachmentCount);
    uint32 index = 0;
    for (; index < info.colorAttachmentCount; index++)
    {
        const Attachment& attachment = info.colorAttachments[index];
        VkAttachmentLoadOp loadOp = RHIUtilityVulkan::ToLoadOp(attachment.loadAction);

        attachments[index].flags = 0;
        attachments[index].format = RHIUtilityVulkan::ToPixelFormat(attachment.format);
        attachments[index].loadOp = loadOp;
        attachments[index].storeOp = RHIUtilityVulkan::ToStoreOp(attachment.storeAction);
        attachments[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[index].initialLayout = (loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[index].finalLayout = colorFinalLayout;
        attachments[index].samples = toSampleCount(attachment.sampleCount);
    }

    if (info.useDepthStencilAttachment)
    {
        const Attachment& attachment = info.depthStencilAttachment;
        VkAttachmentLoadOp loadOp = RHIUtilityVulkan::ToLoadOp(attachment.loadAction);

        attachments[index].flags = 0;
        attachments[index].format = RHIUtilityVulkan::ToPixelFormat(attachment.format);
        attachments[index].loadOp = loadOp;
        attachments[index].storeOp = RHIUtilityVulkan::ToStoreOp(attachment.storeAction);
        attachments[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[index].initialLayout = (loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[index].finalLayout = depthStencilFinalLayout;
        attachments[index].samples = toSampleCount(attachment.sampleCount);
        index++;
    }

    std::vector<VkAttachmentReference> colorAttachments(info.colorAttachmentCount);
    for (uint32 i = 0; i < info.colorAttachmentCount; i++)
    {
        colorAttachments[i].attachment = i;
        colorAttachments[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subPass{};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.colorAttachmentCount = info.colorAttachmentCount;
    subPass.pColorAttachments = colorAttachments.data();

    VkAttachmentReference depthStencilAttachmentRef{};
    if (info.useDepthStencilAttachment)
    {
        depthStencilAttachmentRef.attachment = index - 1;
        depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subPass.pDepthStencilAttachment = &depthStencilAttachmentRef;
    }

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = attachmentCount;
    createInfo.pAttachments = attachments.data();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subPass;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VK_CHECK_RESULT(vkCreateRenderPass(_device->logicalDevice, &createInfo, nullptr, &renderPass));
    return renderPass;
}

VkFramebuffer VulkanRenderingCache::createFramebuffer(VkRenderPass renderPass, const RenderingInfo& renderingInfo)
{
    std::vector<VkImageView> attachments;
    attachments.reserve(renderingInfo.colorAttachmentCount + static_cast<uint32>(renderingInfo.useDepthStencilAttachment));

    for (const RenderingAttachmentInfo& attachmentInfo : renderingInfo.colorAttachments)
    {
        VulkanTexture* textureVK = static_cast<VulkanTexture*>(attachmentInfo.texture);
        attachments.push_back(textureVK->imageViewVk);
    }

    if (renderingInfo.useDepthStencilAttachment)
    {
        VulkanTexture* textureVK = static_cast<VulkanTexture*>(renderingInfo.depthStencilAttachment.texture);
        attachments.push_back(textureVK->imageViewVk);
    }

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = static_cast<uint32>(attachments.size());
    createInfo.pAttachments = attachments.data();
    createInfo.width = renderingInfo.renderArea.width;
    createInfo.height = renderingInfo.renderArea.height;
    createInfo.layers = 1;

    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VK_CHECK_RESULT(vkCreateFramebuffer(_device->logicalDevice, &createInfo, nullptr, &framebuffer));
    return framebuffer;
}

size_t VulkanRenderingCache::makeFramebufferKey(VkRenderPass renderPass, const RenderingInfo& renderingInfo) const
{
    size_t key = HashCombine64(reinterpret_cast<uint64>(renderPass), renderingInfo.renderArea.width, renderingInfo.renderArea.height);
    for (const RenderingAttachmentInfo& attachmentInfo : renderingInfo.colorAttachments)
    {
        VulkanTexture* textureVK = static_cast<VulkanTexture*>(attachmentInfo.texture);
        key = HashCombine64(key, reinterpret_cast<uint64>(textureVK->imageViewVk));
    }

    if (renderingInfo.useDepthStencilAttachment)
    {
        VulkanTexture* textureVK = static_cast<VulkanTexture*>(renderingInfo.depthStencilAttachment.texture);
        key = HashCombine64(key, reinterpret_cast<uint64>(textureVK->imageViewVk));
    }

    return key;
}

size_t VulkanRenderingCache::makeRenderPassKey(const LegacyRenderPassInfo& info) const
{
    size_t key = HashCombine(
        static_cast<uint32>(info.colorAttachmentCount),
        static_cast<uint32>(info.useDepthStencilAttachment),
        static_cast<uint32>(info.isSwapchainRenderPass));

    std::hash<Attachment> attachmentHash;
    for (uint32 i = 0; i < info.colorAttachmentCount; i++)
    {
        key = HashCombine64(key, attachmentHash(info.colorAttachments[i]));
    }

    if (info.useDepthStencilAttachment)
    {
        key = HashCombine64(key, attachmentHash(info.depthStencilAttachment));
    }

    return key;
}

VulkanRenderingCache::LegacyRenderPassInfo VulkanRenderingCache::makeRenderPassInfo(const RenderingInfo& renderingInfo) const
{
    LegacyRenderPassInfo info{};
    info.colorAttachmentCount = renderingInfo.colorAttachmentCount;
    info.useDepthStencilAttachment = renderingInfo.useDepthStencilAttachment;
    info.isSwapchainRenderPass = renderingInfo.isSwapchainRendering;
    info.colorAttachments.reserve(renderingInfo.colorAttachments.size());
    for (const RenderingAttachmentInfo& attachmentInfo : renderingInfo.colorAttachments)
    {
        info.colorAttachments.push_back(attachmentInfo.attachment);
    }
    if (renderingInfo.useDepthStencilAttachment)
    {
        info.depthStencilAttachment = renderingInfo.depthStencilAttachment.attachment;
    }
    return info;
}

VulkanRenderingCache::LegacyRenderPassInfo VulkanRenderingCache::makeCompatibleRenderPassInfo(const PipelineRenderTargetLayout& layout) const
{
    LegacyRenderPassInfo info{};
    info.colorAttachmentCount = layout.colorAttachmentCount;
    info.useDepthStencilAttachment = layout.useDepthStencilAttachment;
    info.isSwapchainRenderPass = layout.isSwapchainRenderPass;

    for (EPixelFormat colorFormat : layout.colorFormats)
    {
        Attachment attachment{};
        attachment.format = colorFormat;
        attachment.loadAction = ELoadAction::DontCare;
        attachment.storeAction = EStoreAction::Store;
        attachment.sampleCount = layout.sampleCount == 0 ? 1 : layout.sampleCount;
        attachment.isDepthStencil = false;
        info.colorAttachments.push_back(attachment);
    }

    if (layout.useDepthStencilAttachment)
    {
        Attachment attachment{};
        attachment.format = layout.depthStencilFormat;
        attachment.loadAction = ELoadAction::DontCare;
        attachment.storeAction = EStoreAction::Store;
        attachment.sampleCount = layout.sampleCount == 0 ? 1 : layout.sampleCount;
        attachment.isDepthStencil = true;
        info.depthStencilAttachment = attachment;
    }

    return info;
}

HS_NS_END
