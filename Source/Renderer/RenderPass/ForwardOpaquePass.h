//
//  ForwardOpaquePass.h
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//
#ifndef __HS_FORWARD_OPAQUE_PASS_H__
#define __HS_FORWARD_OPAQUE_PASS_H__

#include "Precompile.h"
#include "Renderer/RenderPass/ForwardRenderPass.h"

namespace hs
{
/*#include "RHI/RenderHandle.h"*/ class RHIRenderPass;
/*#include "RHI/RenderHandle.h"*/ class RHIFramebuffer;
} // namespace hs

HS_NS_BEGIN

class HS_RENDERER_API ForwardOpaquePass : public ForwardRenderPass
{
public:
    ForwardOpaquePass(const char* name, RenderPath* renderer, ERenderingOrder renderingOrder);
    ~ForwardOpaquePass() override;

    void OnBeforeRendering(uint32_t submitIndex) override;

    void Configure(RenderTarget* renderTarget) override;

    void Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass) override;
    void Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass, const RenderParameter& param) override;

    void OnAfterRendering() override;

private:
    RenderTarget* _currentRenderTarget = nullptr;
};

HS_NS_END

#endif
