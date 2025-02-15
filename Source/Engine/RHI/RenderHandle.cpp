#include "Engine/RHI/RenderHandle.h"

HS_NS_BEGIN

RenderPass::RenderPass(const RenderPassInfo& info)
    : RHIHandle(EType::RENDER_PASS)
    , info(info)
{}

RenderPass::~RenderPass()
{}

Framebuffer::Framebuffer(const FramebufferInfo& info)
    : RHIHandle(EType::FRAMEBUFFER)
    , info(info)
{}

Framebuffer::~Framebuffer()
{}

GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineInfo& info)
    : RHIHandle(EType::GRAPHICS_PIPELINE)
    , info(info)
{}

GraphicsPipeline::~GraphicsPipeline()
{}

HS_NS_END
