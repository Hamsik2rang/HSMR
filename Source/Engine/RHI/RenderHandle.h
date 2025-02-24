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

class RenderPass : public RHIHandle
{
public:
    ~RenderPass() override;

    const RenderPassInfo info;

protected:
    RenderPass(const RenderPassInfo& info);
};

class Framebuffer : public RHIHandle
{
public:
    ~Framebuffer() override;

    const FramebufferInfo info;
    
protected:
    Framebuffer(const FramebufferInfo& info);
};

struct GraphicsPipeline : public RHIHandle
{
public:
    ~GraphicsPipeline() override;

    const GraphicsPipelineInfo info;
protected:
    GraphicsPipeline(const GraphicsPipelineInfo& info);
};

template<>
struct Hasher<RenderPass>
{
    static uint32 Get(const RenderPass& key)
    {
        
    }
};



HS_NS_END

#endif /* __HS_RENDER_PASS_H__ */
