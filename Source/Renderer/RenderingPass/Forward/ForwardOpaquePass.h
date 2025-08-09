//
//  ForwardOpaquePass.h
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//
#ifndef __HS_FORWARD_OPAQUE_PASS_H__
#define __HS_FORWARD_OPAQUE_PASS_H__

#include "Precompile.h"
#include "ForwardPass.h"

HS_NS_BEGIN

class RenderPass;
class Framebuffer;
class Pipeline;

class ForwardOpaquePass : public ForwardPass
{
public:
    ForwardOpaquePass(const char* name, Renderer* renderer, ERenderingOrder renderingOrder);
    ~ForwardOpaquePass() override;

    void OnBeforeRendering(uint32_t submitIndex) override;

    void Configure(RenderTarget* renderTarget) override;

    void Execute(CommandBuffer* commandBuffer, RenderPass* renderPass) override;

    void OnAfterRendering() override;

private:
    void createResourceHandles();
    void createPipelineHandles(RenderPass* renderPass);

    RenderTarget* _currentRenderTarget = nullptr;
    Buffer* _vertexBuffer[2];
    Buffer* _indexBuffer;
    Shader* _vertexShader        = nullptr;
    Shader* _fragmentShader      = nullptr;
    GraphicsPipeline* _gPipeline = nullptr;

    size_t _indexCount = 0;
};

HS_NS_END

#endif
