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

struct RenderPassVulkan : public RHIRenderPass
{
	RenderPassVulkan(const char* name, const RenderPassInfo& info) : RHIRenderPass(name, info) {}
	~RenderPassVulkan() override = default;

	VkRenderPass handle = VK_NULL_HANDLE;
};

struct FramebufferVulkan : public RHIFramebuffer
{
	FramebufferVulkan(const char* name, const FramebufferInfo& info) : RHIFramebuffer(name, info) {}
	~FramebufferVulkan() override = default;

	VkFramebuffer handle = VK_NULL_HANDLE;
};

struct PipelineVulkanBase
{
	VkPipeline handle = VK_NULL_HANDLE;
};

struct GraphicsPipelineVulkan : public RHIGraphicsPipeline, public PipelineVulkanBase
{
	GraphicsPipelineVulkan(const char* name, const GraphicsPipelineInfo& info) : RHIGraphicsPipeline(name, info) {}
	~GraphicsPipelineVulkan() override = default;
};

struct ComputePipelineVulkan : public RHIComputePipeline, public PipelineVulkanBase
{
	ComputePipelineVulkan(const char* name, const ComputePipelineInfo& info) : RHIComputePipeline(name, info) {}
	~ComputePipelineVulkan() override = default;
};


HS_NS_END

#endif // __HS_RENDER_HANDLE_VULKAN_H__