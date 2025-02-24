#include "Engine/RHI/Metal/RenderHandleMetal.h"

HS_NS_BEGIN

RenderPassMetal::RenderPassMetal(const RenderPassInfo& info)
    : RenderPass(info)
{
}

RenderPassMetal::~RenderPassMetal()
{
}

FramebufferMetal::FramebufferMetal(const FramebufferInfo& info)
    : Framebuffer(info)
{
    
}

FramebufferMetal::~FramebufferMetal()
{
    
}

GraphicsPipelineMetal::GraphicsPipelineMetal(const GraphicsPipelineInfo& info)
    : GraphicsPipeline(info)
{
}

GraphicsPipelineMetal::~GraphicsPipelineMetal()
{
}
HS_NS_END
