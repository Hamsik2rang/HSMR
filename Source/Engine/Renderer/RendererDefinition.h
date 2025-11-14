//
//  RendererDefinition.h
//  RenderPath
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_RENDERER_DEFINITION_H__
#define __HS_RENDERER_DEFINITION_H__

#include "RHI/RHIDefinition.h"

HS_NS_BEGIN

struct HS_API  RenderTargetInfo
{
    uint32 width;
    uint32 height;
    
    uint8                    colorTextureCount;
    std::vector<TextureInfo> colorTextureInfos;

    bool        useDepthStencilTexture = false;
    TextureInfo depthStencilInfo;

    //    bool useResolveTexture; // TOOD
    //    TexutureInfo resolveStencilInfo; //TODO

    bool isSwapchainTarget = false;
    Swapchain* swapchain;
};

enum class ERenderGroup : uint16
{
    SKYBOX = 500,
    OPAQUE = 800,
    ALPHA_TEST = 1000,
    TRANSPARENT = 1100,
    UI = 2000
};

struct HS_API  RenderParameter
{
};


HS_NS_END

template <>
struct hs::Hasher<hs::RenderTargetInfo>
{
    static uint32 Get(hs::RenderTargetInfo& key)
    {
        uint32 hash = HashCombine(key.colorTextureCount, key.useDepthStencilTexture, key.isSwapchainTarget);

        for (size_t i = 0; i < key.colorTextureCount / 2; i += 2)
        {
            hash = HashCombine(hash,
                hs::Hasher<hs::TextureInfo>::Get(key.colorTextureInfos[i]),
                hs::Hasher<hs::TextureInfo>::Get(key.colorTextureInfos[i + 1]));
        }
        if (key.colorTextureCount % 2 != 0)
        {
            hash = HashCombine(hash, hs::Hasher<hs::TextureInfo>::Get(key.colorTextureInfos.back()));
        }

        hash = HashCombine(hash, key.width, key.height);

        return hash;
    }
};

#endif /* __HS_RENDER_DEFINITION_H__ */
