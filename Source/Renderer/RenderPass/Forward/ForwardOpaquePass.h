//
//  ForwardOpaquePass.h
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//
#ifndef __HS_FORWARD_OPAQUE_PASS_H__
#define __HS_FORWARD_OPAQUE_PASS_H__

#include "Precompile.h"
#include "Renderer/RenderPass/Forward/ForwardRenderPass.h"

HS_NS_BEGIN

class RHIRenderPass;
class RHIFramebuffer;
class Pipeline;

class ForwardOpaquePass : public ForwardRenderPass
{
public:
    ForwardOpaquePass(const char* name, RenderPath* renderer, ERenderingOrder renderingOrder);
    ~ForwardOpaquePass() override;

    void OnBeforeRendering(uint32_t submitIndex) override;

    void Configure(RenderTarget* renderTarget) override;

    void Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass) override;

    void OnAfterRendering() override;

private:
    void createResourceHandles();
    void createPipelineHandles(RHIRenderPass* renderPass);

    RenderTarget* _currentRenderTarget = nullptr;
    RHIBuffer* _vertexBuffer[2];
    RHIBuffer* _indexBuffer;
    RHIShader* _vertexShader        = nullptr;
    RHIShader* _fragmentShader      = nullptr;
    RHIGraphicsPipeline* _gPipeline = nullptr;

    size_t _indexCount = 0;
};

HS_NS_END

#endif
