//
//  RendererDefinition.h
//  RenderPath
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_RENDERER_DEFINITION_H__
#define __HS_RENDERER_DEFINITION_H__

#include "RHI/RHIDefinition.h"
#include "Engine/Camera.h"
#include "Engine/Resource/Image.h"
#include "Engine/Resource/Shader.h"
#include "Engine/Resource/Material.h"
#include "Engine/Resource/Mesh.h"
#include "Engine/Resource/Model.h"


HS_NS_BEGIN

class RenderResourceManager;
class ShaderLibrary;
class Scene;

struct HS_API RenderTargetInfo
{
    uint32 width;
    uint32 height;

    uint8 colorTextureCount;
    std::vector<TextureInfo> colorTextureInfos;

    bool useDepthStencilTexture = false;
    TextureInfo depthStencilInfo;

    //    bool useResolveTexture; // TOOD
    //    TexutureInfo resolveStencilInfo; //TODO

    bool isSwapchainTarget = false;
    Swapchain* swapchain;
};

enum class ERenderGroup : uint16
{
    SKYBOX      = 500,
    OPAQUE      = 800,
    ALPHA_TEST  = 1000,
    TRANSPARENT = 1100,
    UI          = 2000
};

struct HS_API RenderParameter
{
    // Legacy: Direct model list (for backward compatibility)
    std::vector<Model*> models;
    std::vector<Camera*> cameras;

    // ECS Scene for MeshRendererComponent-based rendering
    Scene* scene = nullptr;

    RenderResourceManager* resourceManager = nullptr;
    ShaderLibrary* shaderLibrary = nullptr;
};

HS_NS_END

namespace std
{
template <>
struct hash<hs::RenderTargetInfo>
{
    size_t operator()(const hs::RenderTargetInfo& key) const
    {
        size_t h = hs::HashCombine(
            static_cast<uint32>(key.colorTextureCount),
            static_cast<uint32>(key.useDepthStencilTexture),
            static_cast<uint32>(key.isSwapchainTarget)
        );

        std::hash<hs::TextureInfo> textureHash;
        for (size_t i = 0; i + 1 < key.colorTextureCount; i += 2)
        {
            h = hs::HashCombine64(h, textureHash(key.colorTextureInfos[i]), textureHash(key.colorTextureInfos[i + 1]));
        }
        if (key.colorTextureCount % 2 != 0)
        {
            h = hs::HashCombine64(h, textureHash(key.colorTextureInfos.back()));
        }

        h = hs::HashCombine64(h, key.width, key.height);

        return h;
    }
};
} // namespace std

#endif /* __HS_RENDER_DEFINITION_H__ */
