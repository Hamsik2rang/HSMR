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

HS_NS_END

#endif /* __HS_RENDER_PASS_H__ */
