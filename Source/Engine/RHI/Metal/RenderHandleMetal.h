//
//  RenderHandleMetal.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_RENDER_HANDLE_METAL_H__
#define __HS_RENDER_HANDLE_METAL_H__


#include "Precompile.h"

#include "Engine/RHI/RenderHandle.h"
#include "Engine/RHI/Metal/RHIUtilityMetal.h"


HS_NS_BEGIN


struct RenderPassMetal : public RenderPass
{
    RenderPassMetal(const RenderPassInfo& info);
    ~RenderPassMetal() override;
};

struct FramebufferMetal : public Framebuffer
{
    FramebufferMetal(const FramebufferInfo& info);
    ~FramebufferMetal() override;
};

struct GraphicsPipelineMetal : public GraphicsPipeline
{
    GraphicsPipelineMetal(const GraphicsPipelineInfo& info);
    ~GraphicsPipelineMetal() override;
    
    
};






HS_NS_END


#endif


