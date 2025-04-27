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

HS_NS_BEGIN

struct RenderPassVulkan : public RenderPass
{
	RenderPassVulkan(const RenderPassInfo& info) : RenderPass(info) {}
	~RenderPassVulkan() override;
};

struct FramebufferVulkan : public Framebuffer
{
	FramebufferVulkan(const FramebufferInfo& info) : Framebuffer(info) {}
	~FramebufferVulkan() override;

	VkFramebuffer handle;
};

struct GraphicsPipelineVulkan : public GraphicsPipeline
{
	GraphicsPipelineVulkan(const GraphicsPipelineInfo& info) : GraphicsPipeline(info) {}
	~GraphicsPipelineVulkan() override;
};

struct ComputePipelineVulkan : public ComputePipeline
{
	ComputePipelineVulkan(const ComputePipelineInfo& info) : ComputePipeline(info) {}
	~ComputePipelineVulkan() override;
};


HS_NS_END

#endif // __HS_RENDER_HANDLE_VULKAN_H__