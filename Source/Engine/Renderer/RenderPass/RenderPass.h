//
//  RenderPass.h
//  Renderer
//
//  Created by Yongsik Im on 1/30/25.
//
#ifndef __HS_RENDERER_PASS_H__
#define __HS_RENDERER_PASS_H__

#include "Precompile.h"

#include <typeinfo>

#include "Engine/Renderer/RendererDefinition.h"
#include "RHI/RenderHandle.h"
#include "Renderer/RenderTarget.h"

HS_NS_BEGIN

class RenderPath;
class RHICommandBuffer;
class RHIFramebuffer;

enum class HS_API ERenderingOrder : uint16
{
    INVALID = 0,

    SKYBOX      = 500,
    OPAQUE      = 1000,
    TRANSPARENT = 2000,

    //...
};

constexpr bool operator<(ERenderingOrder lhs, ERenderingOrder rhs)
{
    return static_cast<uint16>(lhs) < static_cast<uint16>(rhs);
}

class HS_API RenderPass
{
public:
    RenderPass(const char* name, RenderPath* renderer, ERenderingOrder renderingOrder);

    virtual ~RenderPass() = default;

    virtual void OnBeforeRendering(uint32_t submitIndex) = 0;

    virtual void Configure(RenderTarget* renderTarget) = 0;

    virtual void Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass) = 0;

    virtual void OnAfterRendering() = 0;

    virtual void Clear() {}

    HS_FORCEINLINE bool IsExecutable() const { return _isExecutable; }

    HS_FORCEINLINE RenderPath* GetRenderer() const { return _renderer; }

    HS_FORCEINLINE const RenderPassInfo& GetFixedSettingForCurrentPass() const { return _renderPassInfo; }

    const char* name;

    ERenderingOrder renderingOrder;

protected:
    RenderPath* _renderer;
    bool      _isExecutable = true;
    size_t    frameIndex;

    RenderPassInfo _renderPassInfo;
};

HS_NS_END

#endif
