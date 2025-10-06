//
//  RenderPass.h
//  Engine
//
//  Created by Yongsik Im on 2/8/25.
//

#ifndef __HS_RENDER_PASS_H__
#define __HS_RENDER_PASS_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"

HS_NS_BEGIN

class HS_RHI_API RHIRenderPass : public RHIHandle
{
public:

    ~RHIRenderPass() override;

    const RenderPassInfo info;

protected:
    RHIRenderPass(const char* name, const RenderPassInfo& info);
};

class HS_RHI_API RHIFramebuffer : public RHIHandle
{
public:
    ~RHIFramebuffer() override;

    const FramebufferInfo info;
    
protected:
    RHIFramebuffer(const char* name, const FramebufferInfo& info);
};

class HS_RHI_API RHIGraphicsPipeline : public RHIHandle
{
public:
    ~RHIGraphicsPipeline() override;

    const GraphicsPipelineInfo info;
protected:
    RHIGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info);
};

class HS_RHI_API RHIComputePipeline : public RHIHandle
{
public:
    ~RHIComputePipeline() override;

	const ComputePipelineInfo info;
protected:
    RHIComputePipeline(const char* name, const ComputePipelineInfo& info);
};

template<>
struct Hasher<RHIRenderPass>
{
    static uint32 Get(const RHIRenderPass& key)
    {
        return 0;
    }
};



HS_NS_END

#endif /* __HS_RENDER_PASS_H__ */
