//
//  RenderHandleVulkan.h
//  Engine
//
//  Created by Yongsik Im on 4/27/25.
//
#ifndef __HS_RENDER_HANDLE_VULKAN_H__
#define __HS_RENDER_HANDLE_VULKAN_H__

#include "Precompile.h"

#include "Engine/RHI/RenderHandle.h"
#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"

HS_NS_BEGIN

struct RenderPassVulkan : public RenderPass
{
	RenderPassVulkan(const RenderPassInfo& info) : RenderPass(info) {}
	~RenderPassVulkan() override = default;

	VkRenderPass handle = VK_NULL_HANDLE;
};

struct FramebufferVulkan : public Framebuffer
{
	FramebufferVulkan(const FramebufferInfo& info) : Framebuffer(info) {}
	~FramebufferVulkan() override = default;

	VkFramebuffer handle = VK_NULL_HANDLE;
};

struct PipelineVulkanBase
{
	VkPipeline handle = VK_NULL_HANDLE;
};

struct GraphicsPipelineVulkan : public GraphicsPipeline, public PipelineVulkanBase
{
	GraphicsPipelineVulkan(const GraphicsPipelineInfo& info) : GraphicsPipeline(info) {}
	~GraphicsPipelineVulkan() override = default;
};

struct ComputePipelineVulkan : public ComputePipeline, public PipelineVulkanBase
{
	ComputePipelineVulkan(const ComputePipelineInfo& info) : ComputePipeline(info) {}
	~ComputePipelineVulkan() override = default;
};


HS_NS_END

#endif // __HS_RENDER_HANDLE_VULKAN_H__