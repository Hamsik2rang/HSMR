//
//  RenderHandleVulkan.h
//  Engine
//
//  Created by Yongsik Im on 4/27/25.
//
#ifndef __HS_RENDER_HANDLE_VULKAN_H__
#define __HS_RENDER_HANDLE_VULKAN_H__

#include "Precompile.h"

#include "RHI/RenderHandle.h"
#include "RHI/Vulkan/VulkanDevice.h"

HS_NS_BEGIN

struct HS_RHI_API VulkanRenderPass : public RHIRenderPass
{
    VulkanRenderPass(const char* name, const RenderPassInfo& info)
        : RHIRenderPass(name, info)
    {}
    ~VulkanRenderPass() override = default;

public:
    VkRenderPass handle = VK_NULL_HANDLE;
};

struct HS_RHI_API VulkanFramebuffer : public RHIFramebuffer
{
    VulkanFramebuffer(const char* name, const FramebufferInfo& info)
        : RHIFramebuffer(name, info)
    {}
    ~VulkanFramebuffer() override = default;

public:
    VkFramebuffer handle = VK_NULL_HANDLE;
};

struct HS_RHI_API VulkanPipelineBase
{
    VkPipeline handle       = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
};

struct HS_RHI_API VulkanGraphicsPipeline : public RHIGraphicsPipeline, public VulkanPipelineBase
{
    VulkanGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info)
        : RHIGraphicsPipeline(name, info)
    {}
    ~VulkanGraphicsPipeline() override = default;
};

struct HS_RHI_API VulkanComputePipeline : public RHIComputePipeline, public VulkanPipelineBase
{
    VulkanComputePipeline(const char* name, const ComputePipelineInfo& info)
        : RHIComputePipeline(name, info)
    {}
    ~VulkanComputePipeline() override = default;
};

HS_NS_END

#endif // __HS_RENDER_HANDLE_VULKAN_H__