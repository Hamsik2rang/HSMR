#include "RHI/Metal/MetalRenderHandle.h"

HS_NS_BEGIN

MetalGraphicsPipeline::MetalGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info)
    : RHIGraphicsPipeline(name, info)
{
}

MetalGraphicsPipeline::~MetalGraphicsPipeline()
{
}

MetalComputePipeline::MetalComputePipeline(const char* name, const ComputePipelineInfo& info)
    : RHIComputePipeline(name, info)
{
}

MetalComputePipeline::~MetalComputePipeline()
{
}

HS_NS_END
