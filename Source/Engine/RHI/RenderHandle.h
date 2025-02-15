//
//  RenderPass.h
//  Engine
//
//  Created by Yongsik Im on 2/8/25.
//

#ifndef __HS_RENDER_PASS_H__
#define __HS_RENDER_PASS_H__

#include "Precompile.h"

#include "Engine/RHI/RHIDefinition.h"

HS_NS_BEGIN

struct RenderPass : public RHIHandle
{
    RenderPass(const RenderPassInfo& info);
    ~RenderPass() override;

    const RenderPassInfo info;
};

struct Framebuffer : public RHIHandle
{
    Framebuffer(const FramebufferInfo& info);
    ~Framebuffer() override;
    
    const FramebufferInfo info;
};

struct GraphicsPipeline : public RHIHandle
{
    GraphicsPipeline(const GraphicsPipelineInfo& info);
    ~GraphicsPipeline() override;
    
    const GraphicsPipelineInfo info;
};

HS_NS_END

#endif /* __HS_RENDER_PASS_H__ */
