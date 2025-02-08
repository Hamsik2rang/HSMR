//
//  RendererPass.h
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//
#ifndef __RENDERER_PASS_H__
#define __RENDERER_PASS_H__

#include "Precompile.h"

#include <typeinfo>

#include "Engine/ThirdParty/ImGui/imgui.h"
#include "Engine/Renderer/RenderDefinition.h"

HS_NS_BEGIN

#define RHI_RESOURCE_BEGIN struct RendererPass::RHIResource {
#define RHI_RESOURCE_END };


class Renderer;
class CommandBuffer;
class RenderCommandEncoder;

enum class ERenderingOrder
{
    INVALID = 0,
    
    
    OPAQUE = 500,
    SKYBOX = 600,
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

    virtual void Configure(RenderTexture* renderTarget) = 0;

    virtual void Execute(RenderCommandEncoder* renderEncoder) = 0;

    virtual void OnAfterRendering() = 0;
    
    virtual void Clear() {}

    bool IsExecutable() { return _isExecutable; }

    const char* name;

    uint32_t renderingOrder;
    
    static RendererPass* Create(const char* name, Renderer* renderer, ERenderingOrder renderingOrder);

protected:
    struct RHIResource;
    RHIResource* _res;
    
    
    Renderer* _renderer;
    bool _isExecutable = true;
    size_t _submitIndex;
};

HS_NS_END

#endif
