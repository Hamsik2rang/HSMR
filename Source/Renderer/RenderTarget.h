//
//  RenderTarget.h
//  HSMR
//
//  Created by Yongsik Im on 11/2/25.
//
#ifndef __HS_RENDER_TARGET_H__
#define __HS_RENDER_TARGET_H__

#include "Precompile.h"

#include "RHI/ResourceHandle.h"
#include "Renderer/RendererDefinition.h"

HS_NS_BEGIN

class RenderTarget
{
public:
    RenderTarget() = default;
    RenderTarget(const RenderTargetInfo& info);
    ~RenderTarget();

    void Create(const RenderTargetInfo& info);
    void Update(const RenderTargetInfo& info);
    void Update(uint32 width, uint32 height);
    void Clear();

    uint32 GetWidth() const { return _info.width; }
    uint32 GetHeight() const { return _info.height; }

    RHITexture* GetColorTexture(uint32 index) const { return _colorTextures[index]; }
    RHITexture* GetDepthStencilTexture() const { return _depthStencilTexture; }

    const std::vector<RHITexture*>& GetColorTextures() const { return _colorTextures; }
    size_t                       GetColorTextureCount() const { return _colorTextures.size(); }

    const TextureInfo& GetColorTextureInfo(uint32 index) const { return _colorTextures[index]->info; }
    const TextureInfo& GetDepthStencilTextureInfo() const { return _depthStencilTexture->info; }

    const RenderTargetInfo& GetInfo() const { return _info; }

private:
    RenderTargetInfo      _info;
    std::vector<RHITexture*> _colorTextures;
    RHITexture*              _depthStencilTexture;
};

template <>
struct Hasher<RenderTarget>
{
    static uint32 Get(const RenderTarget& key)
    {
        const RenderTargetInfo& info = key.GetInfo();

        uint32 hash = HashCombine(key.GetWidth(), key.GetHeight(), info.isSwapchainTarget);
        hash        = HashCombine(hash, Hasher<size_t>::Get(key.GetColorTextureCount()), info.useDepthStencilTexture);
        for (size_t i = 0; i < key.GetColorTextureCount(); i++)
        {
            RHITexture* colorTexture = key.GetColorTexture(static_cast<uint32>(i));
            hash                  = HashCombine(hash, Hasher<TextureInfo>::Get(colorTexture->info), PointerHash(colorTexture));
        }

        if (info.useDepthStencilTexture)
        {
            RHITexture* depthTexture = key.GetDepthStencilTexture();
            uint32   b            = Hasher<TextureInfo>::Get(depthTexture->info);
            uint32   c            = PointerHash(depthTexture);

            hash = HashCombine(hash, b, c);
        }

        return hash;
    }
};

HS_NS_END

#endif /* __HS_RENDER_TARGET_H__ */
