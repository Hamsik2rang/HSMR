//
//  RenderDefinition.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_RENDER_DEFINITION_H__
#define __HS_RENDER_DEFINITION_H__

#include "Engine/RHI/RHIDefinition.h"

HS_NS_BEGIN

struct RenderTargetInfo
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
};

struct RenderParameter
{
};

template <>
struct Hasher<RenderTargetInfo>
{
    static uint32 Get(RenderTargetInfo& key)
    {
        uint32 hash = HashCombine(key.colorTextureCount, key.useDepthStencilTexture, key.isSwapchainTarget);
        
        for (size_t i = 0; i < key.colorTextureCount / 2; i += 2)
        {
            hash = HashCombine(hash,
                    Hasher<TextureInfo>::Get(key.colorTextureInfos[i]),
                    Hasher<TextureInfo>::Get(key.colorTextureInfos[i+1]));
        }
        if (key.colorTextureCount % 2 != 0)
        {
            hash = HashCombine(hash, Hasher<TextureInfo>::Get(key.colorTextureInfos.back()));
        }
        
        hash = HashCombine(hash, key.width, key.height);
        
        return hash;
    }
};

HS_NS_END

#endif /* __HS_RENDER_DEFINITION_H__ */
