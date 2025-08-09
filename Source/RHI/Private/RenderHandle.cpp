#include "Engine/RHI/RenderHandle.h"

HS_NS_BEGIN

RenderPass::RenderPass(const char* name, const RenderPassInfo& info)
    : RHIHandle(EType::RENDER_PASS, name)
    , info(info)
{}

RenderPass::~RenderPass()
{}

Framebuffer::Framebuffer(const char* name, const FramebufferInfo& info)
    : RHIHandle(EType::FRAMEBUFFER, name)
    , info(info)
{}

Framebuffer::~Framebuffer()
{}

GraphicsPipeline::GraphicsPipeline(const char* name, const GraphicsPipelineInfo& info)
    : RHIHandle(EType::GRAPHICS_PIPELINE, name)
    , info(info)
{}

GraphicsPipeline::~GraphicsPipeline()
{}

ComputePipeline::ComputePipeline(const char* name, const ComputePipelineInfo& info)
    : RHIHandle(EType::COMPUTE_PIPELINE, name)
    , info(info)
{}

ComputePipeline::~ComputePipeline()
{}

HS_NS_END
