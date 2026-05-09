//
//  RendererDefinition.h
//  Renderer
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_RENDERER_DEFINITION_H__
#define __HS_RENDERER_DEFINITION_H__

#include "Renderer/RenderDefinition.h"
#include "RHI/RHIDefinition.h"
#include "Resource/Image.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Mesh.h"

HS_NS_BEGIN

class Scene;

// Forward declarations for GPU resource types
struct CameraResource;
struct LightResource;
struct MeshResource;
struct MaterialResource;
struct DrawResource;
class RHIBuffer;

struct HS_RENDERER_API RenderTargetInfo
{
    uint32 width;
    uint32 height;

    uint8 colorTextureCount;
    std::vector<TextureInfo> colorTextureInfo;

    bool useDepthStencilTexture = false;
    TextureInfo depthStencilInfo;

    //    bool useResolveTexture; // TOOD
    //    TexutureInfo resolveStencilInfo; //TODO

    bool isSwapchainTarget = false;
    Swapchain* swapchain;
};

enum class ERenderGroup : uint16
{
    Skybox      = 500,
    Opaque      = 800,
    AlphaTest   = 1000,
    Transparent = 1100,
    Ui          = 2000
};

// 렌더링 단위: GPU 리소스 묶음
struct HS_RENDERER_API RenderModel
{
    glm::mat4 worldMatrix{1.0f};
    glm::mat4 inverseWorldMatrix{1.0f};
    Material* material                 = nullptr; // 파이프라인 룩업용
    RHIBuffer* perDrawBuffer           = nullptr; // PerDraw UBO
    MeshResource* meshResource         = nullptr; // VB/IB
    MaterialResource* materialResource = nullptr; // Shader/Textures/ResourceSet
    DrawResource* drawResource         = nullptr; // Per-view/per-draw merged ResourceSet
};

struct HS_RENDERER_API RenderViewSnapshot
{
    uint64 viewId = 0;
    PerView perView{};
};

struct HS_RENDERER_API RenderLightSnapshot
{
    uint64 lightId = 0;
    LightUBO light{};
};

enum class EDebugCameraProjectionType : uint8
{
    Perspective,
    Orthographic
};

struct HS_RENDERER_API DebugCameraSnapshot
{
    uint64 entityId = 0;
    glm::mat4 worldMatrix{1.0f};
    EDebugCameraProjectionType projectionType = EDebugCameraProjectionType::Perspective;
    float fov = 60.0f;
    float aspectRatio = 16.0f / 9.0f;
    float orthoSize = 10.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
};

enum class EDebugLightType : uint8
{
    Directional,
    Point,
    Spot
};

struct HS_RENDERER_API DebugLightSnapshot
{
    uint64 entityId = 0;
    glm::mat4 worldMatrix{1.0f};
    EDebugLightType type = EDebugLightType::Directional;
    glm::vec3 color{1.0f};
    float intensity = 1.0f;
    float range = 10.0f;
    float innerConeAngle = 30.0f;
    float outerConeAngle = 45.0f;
    bool isEnabled = true;
};

struct HS_RENDERER_API RenderPrimitiveSnapshot
{
    uint64 primitiveId = 0;
    uint32 worldVersion = 0;
    glm::mat4 worldMatrix{1.0f};
    Mesh* mesh = nullptr;
    Material* material = nullptr;
};

struct HS_RENDERER_API RenderSceneSnapshot
{
    std::vector<RenderViewSnapshot> views;
    std::vector<RenderLightSnapshot> lights;
    std::vector<RenderPrimitiveSnapshot> primitives;
    std::vector<DebugCameraSnapshot> debugCameras;
    std::vector<DebugLightSnapshot> debugLights;
};

struct HS_RENDERER_API RenderOptions
{
    bool enableGrid = false;
    bool enableDebug = false;
};

// 씬 전체의 렌더링용 GPU 리소스 집합
struct HS_RENDERER_API SceneResource
{
    std::vector<CameraResource*> cameraResources;
    std::vector<LightResource*> lightResources;
    std::vector<RenderModel> renderModels;
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
            h = hs::HashCombine64(h, textureHash(key.colorTextureInfo[i]), textureHash(key.colorTextureInfo[i + 1]));
        }
        if (key.colorTextureCount % 2 != 0)
        {
            h = hs::HashCombine64(h, textureHash(key.colorTextureInfo.back()));
        }

        h = hs::HashCombine64(h, key.width, key.height);

        return h;
    }
};
} // namespace std

#endif /* __HS_RENDER_DEFINITION_H__ */
