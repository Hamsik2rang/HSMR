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


struct MetalRenderPass : public RHIRenderPass
{
    MetalRenderPass(const char* name, const RenderPassInfo& info);
    ~MetalRenderPass() override;
    
    MTLRenderPassDescriptor* handle;
};

struct MetalFramebuffer : public RHIFramebuffer
{
    MetalFramebuffer(const char* name, const FramebufferInfo& info);
    ~MetalFramebuffer() override;
};

struct MetalGraphicsPipeline : public RHIGraphicsPipeline
{
    MetalGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info);
    ~MetalGraphicsPipeline() override;
    
    id<MTLRenderPipelineState> pipelineState;
    id<MTLDepthStencilState> depthStencilState;
};



HS_NS_END


#endif


