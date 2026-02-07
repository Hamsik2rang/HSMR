//
//  ForwardOpaquePass.h
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//
#ifndef __HS_FORWARD_OPAQUE_PASS_H__
#define __HS_FORWARD_OPAQUE_PASS_H__

#include "Precompile.h"
#include "Engine/Renderer/RenderPass/ForwardRenderPass.h"

#include <unordered_map>

namespace hs
{
/*#include "RHI/RenderHandle.h"*/ class RHIRenderPass;
/*#include "RHI/RenderHandle.h"*/ class RHIFramebuffer;
/*#include "RHI/RenderHandle.h"*/ class RHIGraphicsPipeline;
/*#include "RHI/RenderHandle.h"*/ class RHIResourceLayout;
/*#include "RHI/RenderHandle.h"*/ class RHIResourceSet;
/*#include "RHI/Resource/Mesh.h*/ class Mesh;
} // namespace hs

HS_NS_BEGIN

class HS_API ForwardOpaquePass : public ForwardRenderPass
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
    void createResourceHandles();
    void createPipelineHandles(RHIRenderPass* renderPass);

    RenderTarget* _currentRenderTarget = nullptr;
    RHIShader* _vertexShader           = nullptr;
    RHIShader* _fragmentShader         = nullptr;
    RHIGraphicsPipeline* _gPipeline    = nullptr;

    RHIBuffer* _perViewBuffer          = nullptr;
    RHIBuffer* _perDrawBuffer          = nullptr;
    RHIResourceLayout* _resourceLayout = nullptr;
    RHIResourceSet* _resourceSet       = nullptr;

    std::unordered_map<Mesh*, RHIBuffer*> _meshVertexBuffers;
    std::unordered_map<Mesh*, RHIBuffer*> _meshIndexBuffers;
    std::unordered_map<Mesh*, uint32> _meshIndexCounts;
};

HS_NS_END

#endif
