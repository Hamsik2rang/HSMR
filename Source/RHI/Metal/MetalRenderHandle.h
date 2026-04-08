//
//  MetalRenderHandle.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_RENDER_HANDLE_METAL_H__
#define __HS_RENDER_HANDLE_METAL_H__


#include "Precompile.h"

#include "RHI/RenderHandle.h"
#include "RHI/Metal/MetalUtility.h"


HS_NS_BEGIN


struct MetalGraphicsPipeline : public RHIGraphicsPipeline
{
    MetalGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info);
    ~MetalGraphicsPipeline() override;

    id<MTLRenderPipelineState> pipelineState;
    id<MTLDepthStencilState> depthStencilState;
};

struct MetalComputePipeline : public RHIComputePipeline
{
    MetalComputePipeline(const char* name, const ComputePipelineInfo& info);
    ~MetalComputePipeline() override;

    id<MTLComputePipelineState> pipelineState;
};

HS_NS_END


#endif


