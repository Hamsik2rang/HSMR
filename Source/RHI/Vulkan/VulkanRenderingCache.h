#ifndef __HS_RENDERING_CACHE_VULKAN_H__
#define __HS_RENDERING_CACHE_VULKAN_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"
#include "RHI/Vulkan/VulkanDevice.h"

#include <unordered_map>

HS_NS_BEGIN

class VulkanContext;

class HS_RHI_API VulkanRenderingCache
{
public:
    struct LegacyRenderingHandles
    {
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
    };

    VulkanRenderingCache() = default;
    ~VulkanRenderingCache();

    void Initialize(VulkanContext* context, VulkanDevice* device);
    void Finalize();
    void Reset();

    VkRenderPass GetCompatibleRenderPass(const PipelineRenderTargetLayout& layout);
    LegacyRenderingHandles GetLegacyRenderingHandles(const RenderingInfo& renderingInfo);

private:
    VkRenderPass createRenderPass(const RenderPassInfo& info);
    VkFramebuffer createFramebuffer(VkRenderPass renderPass, const RenderingInfo& renderingInfo);

    size_t makeFramebufferKey(VkRenderPass renderPass, const RenderingInfo& renderingInfo) const;
    RenderPassInfo makeCompatibleRenderPassInfo(const PipelineRenderTargetLayout& layout) const;

    VulkanContext* _context = nullptr;
    VulkanDevice* _device = nullptr;

    std::unordered_map<size_t, VkRenderPass> _renderPassCache;
    std::unordered_map<size_t, VkRenderPass> _compatibleRenderPassCache;
    std::unordered_map<size_t, VkFramebuffer> _framebufferCache;
};

HS_NS_END

#endif
