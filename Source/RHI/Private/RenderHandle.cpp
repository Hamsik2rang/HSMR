#include "RHI/RenderHandle.h"

HS_NS_BEGIN

RHIRenderPass::RHIRenderPass(const char* name, const RenderPassInfo& info)
    : RHIHandle(EType::RenderPass, name)
    , info(info)
{}

RHIRenderPass::~RHIRenderPass()
{}

RHIFramebuffer::RHIFramebuffer(const char* name, const FramebufferInfo& info)
    : RHIHandle(EType::Framebuffer, name)
    , info(info)
{}

RHIFramebuffer::~RHIFramebuffer()
{}

RHIGraphicsPipeline::RHIGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info)
    : RHIHandle(EType::GraphicsPipeline, name)
    , info(info)
{}

RHIGraphicsPipeline::~RHIGraphicsPipeline()
{}

RHIComputePipeline::RHIComputePipeline(const char* name, const ComputePipelineInfo& info)
    : RHIHandle(EType::ComputePipeline, name)
    , info(info)
{}

RHIComputePipeline::~RHIComputePipeline()
{}

HS_NS_END
