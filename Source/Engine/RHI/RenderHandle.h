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
    RenderPass(const char* name, const RenderPassInfo& info);
};

class Framebuffer : public RHIHandle
{
public:
    ~Framebuffer() override;

    const FramebufferInfo info;
    
protected:
    Framebuffer(const char* name, const FramebufferInfo& info);
};

class GraphicsPipeline : public RHIHandle
{
public:
    ~GraphicsPipeline() override;

    const GraphicsPipelineInfo info;
protected:
    GraphicsPipeline(const char* name, const GraphicsPipelineInfo& info);
};

class ComputePipeline : public RHIHandle
{
public:
    ~ComputePipeline() override;

	const ComputePipelineInfo info;
protected:
    ComputePipeline(const char* name, const ComputePipelineInfo& info);
};

template<>
struct Hasher<RenderPass>
{
    static uint32 Get(const RenderPass& key)
    {
        return 0;
    }
};



HS_NS_END

#endif /* __HS_RENDER_PASS_H__ */
