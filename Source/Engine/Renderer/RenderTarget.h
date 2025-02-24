//
//  RenderTarget.h
//  HSMR
//
//  Created by Yongsik Im on 11/2/25.
//
#ifndef __HS_RENDER_TARGET_H__
#define __HS_RENDER_TARGET_H__

#include "Precompile.h"

#include "Engine/RHI/ResourceHandle.h"
#include "Engine/Renderer/RenderDefinition.h"

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

    Texture* GetColorTexture(uint32 index) const { return _colorTextures[index]; }
    Texture* GetDepthStencilTexture() const { return _depthStencilTexture; }

    const std::vector<Texture*>& GetColorTextures() const { return _colorTextures; }
    size_t                       GetColorTextureCount() const { return _colorTextures.size(); }

    const TextureInfo& GetColorTextureInfo(uint32 index) const { return _colorTextures[index]->info; }
    const TextureInfo& GetDepthStencilTextureInfo() const { return _depthStencilTexture->info; }

    const RenderTargetInfo& GetInfo() const { return _info; }

private:
    RenderTargetInfo _info;
    std::vector<Texture*> _colorTextures;
    Texture*              _depthStencilTexture;
};

template <>
struct Hasher<RenderTarget>
{
    static uint32 Get(const RenderTarget& key)
    {
        const RenderTargetInfo& info = key.GetInfo();

        uint32 hash = HashCombine(key.GetWidth(), key.GetHeight(), info.isSwapchainTarget);
        hash        = HashCombine(hash, Hasher<size_t>::Get(key.GetColorTextureCount()), info.useDepthStencilTexture);
        for (size_t i = 0; i < key.GetColorTextureCount() / 2; i += 2)
        {
            hash = HashCombine(hash, Hasher<TextureInfo>::Get(key.GetColorTexture(static_cast<uint32>(i))->info), Hasher<TextureInfo>::Get(key.GetColorTexture(static_cast<uint32>(i) + 1)->info));
        }

        uint32 b = (key.GetColorTextureCount() % 2 != 0) ? Hasher<TextureInfo>::Get(key.GetColorTexture(static_cast<uint32>(key.GetColorTextureCount()) - 1)->info) : 0;
        uint32 c = info.useDepthStencilTexture ? Hasher<TextureInfo>::Get(key.GetDepthStencilTexture()->info) : 0;

        hash = HashCombine(hash, b, c);

        return hash;
    }
};

HS_NS_END

#endif /* __HS_RENDER_TARGET_H__ */
