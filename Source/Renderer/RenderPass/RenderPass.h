//
//  RenderPass.h
//  Renderer
//
//  Created by Yongsik Im on 1/30/25.
//
#ifndef __HS_RENDERER_PASS_H__
#define __HS_RENDERER_PASS_H__

#include "Precompile.h"

#include "RHI/RenderHandle.h"
#include "Renderer/RendererDefinition.h"
#include "Renderer/RenderTarget.h"

HS_NS_BEGIN

class Renderer;
class RHICommandBuffer;
class RHIFramebuffer;

enum class HS_RENDERER_API ERenderingOrder : uint16
{
    Invalid = 0,

    Opaque      = 2000,
    Skybox      = 2500,
    Transparent = 3000,

    PostProcess = 8000
    //...
};

constexpr bool operator<(ERenderingOrder lhs, ERenderingOrder rhs)
{
    return static_cast<uint16>(lhs) < static_cast<uint16>(rhs);
}

class HS_RENDERER_API RenderPass
{
public:
    RenderPass(const char* name, Renderer* renderer, ERenderingOrder renderingOrder);

    virtual ~RenderPass() = default;

    virtual void OnBeforeRendering(uint32_t submitIndex) = 0;

    virtual void Configure(RenderTarget* renderTarget) = 0;

    virtual void Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass, const SceneResource& sceneResource) = 0;

    virtual void OnAfterRendering() = 0;

    virtual void Clear() {}

    HS_FORCEINLINE bool IsExecutable() const { return _isExecutable; }

    HS_FORCEINLINE Renderer* GetRenderer() const { return _renderer; }

    HS_FORCEINLINE const RenderPassInfo& GetFixedSettingForCurrentPass() const { return _renderPassInfo; }

    const char* name;

    ERenderingOrder renderingOrder;

protected:
    Renderer* _renderer;
    bool      _isExecutable = true;
    size_t    frameIndex;

    RenderPassInfo _renderPassInfo;
};

HS_NS_END

#endif
