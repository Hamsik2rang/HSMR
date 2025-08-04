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
	RenderPassVulkan(const char* name, const RenderPassInfo& info) : RenderPass(name, info) {}
	~RenderPassVulkan() override = default;

	VkRenderPass handle = VK_NULL_HANDLE;
};

struct FramebufferVulkan : public Framebuffer
{
	FramebufferVulkan(const char* name, const FramebufferInfo& info) : Framebuffer(name, info) {}
	~FramebufferVulkan() override = default;

	VkFramebuffer handle = VK_NULL_HANDLE;
};

struct PipelineVulkanBase
{
	VkPipeline handle = VK_NULL_HANDLE;
};

struct GraphicsPipelineVulkan : public GraphicsPipeline, public PipelineVulkanBase
{
	GraphicsPipelineVulkan(const char* name, const GraphicsPipelineInfo& info) : GraphicsPipeline(name, info) {}
	~GraphicsPipelineVulkan() override = default;
};

struct ComputePipelineVulkan : public ComputePipeline, public PipelineVulkanBase
{
	ComputePipelineVulkan(const char* name, const ComputePipelineInfo& info) : ComputePipeline(name, info) {}
	~ComputePipelineVulkan() override = default;
};


HS_NS_END

#endif // __HS_RENDER_HANDLE_VULKAN_H__