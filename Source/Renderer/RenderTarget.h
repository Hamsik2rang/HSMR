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

class HS_RENDERER_API RenderTarget
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

HS_NS_END

namespace std
{
    template <>
    struct hash<hs::RenderTarget>
    {
        size_t operator()(const hs::RenderTarget& key) const
        {
            const hs::RenderTargetInfo& info = key.GetInfo();

            size_t h = hs::HashCombine(key.GetWidth(), key.GetHeight(), static_cast<uint32>(info.isSwapchainTarget));
            h = hs::HashCombine64(h, key.GetColorTextureCount(), static_cast<size_t>(info.useDepthStencilTexture));

            std::hash<hs::TextureInfo> textureHash;
            for (size_t i = 0; i < key.GetColorTextureCount(); i++)
            {
                hs::RHITexture* colorTexture = key.GetColorTexture(static_cast<uint32>(i));
                h = hs::HashCombine64(h, textureHash(colorTexture->info), hs::PointerHash(colorTexture));
            }

            if (info.useDepthStencilTexture)
            {
                hs::RHITexture* depthTexture = key.GetDepthStencilTexture();
                size_t b = textureHash(depthTexture->info);
                size_t c = hs::PointerHash(depthTexture);
                h = hs::HashCombine64(h, b, c);
            }

            return h;
        }
    };
}

#endif /* __HS_RENDER_TARGET_H__ */
