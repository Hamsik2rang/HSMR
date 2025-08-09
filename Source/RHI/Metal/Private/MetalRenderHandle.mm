#include "Engine/RHI/Metal/RenderHandleMetal.h"

HS_NS_BEGIN

RenderPassMetal::RenderPassMetal(const char* name, const RenderPassInfo& info)
    : RenderPass(name, info)
{
}

RenderPassMetal::~RenderPassMetal()
{
}

FramebufferMetal::FramebufferMetal(const char* name, const FramebufferInfo& info)
    : Framebuffer(name, info)
{
    
}

FramebufferMetal::~FramebufferMetal()
{
    
}

GraphicsPipelineMetal::GraphicsPipelineMetal(const char* name, const GraphicsPipelineInfo& info)
    : GraphicsPipeline(name, info)
{
}

GraphicsPipelineMetal::~GraphicsPipelineMetal()
{
}
HS_NS_END
