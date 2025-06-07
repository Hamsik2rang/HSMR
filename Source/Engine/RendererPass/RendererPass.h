//
//  RendererPass.h
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//
#ifndef __HS_RENDERER_PASS_H__
#define __HS_RENDERER_PASS_H__

#include "Precompile.h"

#include <typeinfo>

#include "Engine/Renderer/RenderDefinition.h"
#include "Engine/RHI/RenderHandle.h"
#include "Engine/Renderer/RenderTarget.h"

HS_NS_BEGIN

class Renderer;
class CommandBuffer;
class RenderCommandEncoder;
class Framebuffer;

enum class ERenderingOrder
{
    INVALID = 0,

    OPAQUE      = 500,
    SKYBOX      = 600,
    TRANSPARENT = 800,

    //...
};

constexpr bool operator<(ERenderingOrder lhs, ERenderingOrder rhs)
{
    return static_cast<uint16>(lhs) < static_cast<uint16>(rhs);
}

class RendererPass
{
public:
    RendererPass(const char* name, Renderer* renderer, ERenderingOrder renderingOrder);

    virtual ~RendererPass() = default;

    virtual void OnBeforeRendering(uint32_t submitIndex) = 0;

    virtual void Configure(RenderTarget* renderTarget) = 0;

    virtual void Execute(CommandBuffer* commandBuffer, RenderPass* renderPass) = 0;

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
