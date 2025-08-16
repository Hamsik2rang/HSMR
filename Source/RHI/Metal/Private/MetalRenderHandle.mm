#include "RHI/Metal/MetalRenderHandle.h"

HS_NS_BEGIN

MetalRenderPass::MetalRenderPass(const char* name, const RenderPassInfo& info)
    : RHIRenderPass(name, info)
{
}

MetalRenderPass::~MetalRenderPass()
{
}

MetalFramebuffer::MetalFramebuffer(const char* name, const FramebufferInfo& info)
    : RHIFramebuffer(name, info)
{
    
}

MetalFramebuffer::~MetalFramebuffer()
{
    
}

MetalGraphicsPipeline::MetalGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info)
    : RHIGraphicsPipeline(name, info)
{
}

MetalGraphicsPipeline::~MetalGraphicsPipeline()
{
}
HS_NS_END
