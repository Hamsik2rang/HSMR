#include "RHI/RenderHandle.h"

HS_NS_BEGIN

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
