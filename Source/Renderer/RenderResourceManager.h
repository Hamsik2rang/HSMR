//
//  RenderResourceManager.h
//  HSMR
//
//  Automated RHI resource creation from shader reflection data.
//
#ifndef __HS_RENDER_RESOURCE_MANAGER_H__
#define __HS_RENDER_RESOURCE_MANAGER_H__

#include "Precompile.h"
#include "Renderer/RenderDefinition.h"
#include "Resource/ResourceDefinition.h"
#include "ShaderSystem/ShaderSystemDefinition.h"

#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/TransformComponent.h"
#include "Scene/Components/LightComponent.h"

#include <unordered_map>
#include <vector>

namespace hs
{
class RHIContext;
class RHIShader;
class RHIBuffer;
class RHITexture;
class RHISampler;
class RHIResourceLayout;
class RHIResourceSet;
class RHIGraphicsPipeline;
class RHIRenderPass;
class Material;
class Mesh;
class Shader;
class Image;
} // namespace hs

HS_NS_BEGIN

// Cached RHI resources per image (GPU texture + sampler)
struct HS_RENDERER_API ImageResource
{
    RHITexture* texture = nullptr;
    RHISampler* sampler = nullptr;
    uint32 width        = 0;
    uint32 height       = 0;
    EPixelFormat format = EPixelFormat::R8G8B8A8Unorm;
    bool isValid        = false;
};

// Cached RHI resources per material
struct HS_RENDERER_API MaterialResource
{
    RHIShader* vertexShader           = nullptr;
    RHIShader* fragmentShader         = nullptr;
    RHIResourceLayout* resourceLayout = nullptr;
    RHIResourceSet* resourceSet       = nullptr;
    std::vector<RHIBuffer*> materialBuffers;
    std::unordered_map<size_t, RHIGraphicsPipeline*> pipelineCache;
    std::vector<ImageResource*> textureResources; // Referenced textures
    bool isValid = false;
};

// Cached RHI resources per mesh
struct HS_RENDERER_API MeshResource
{
    RHIBuffer* vertexBuffer = nullptr;
    RHIBuffer* indexBuffer  = nullptr;
    uint32 indexCount       = 0;
    bool isValid            = false;
};

// GPU resource for a camera view (PerView UBO)
struct HS_RENDERER_API CameraResource
{
    PerView perViewData{};
    RHIBuffer* perViewBuffer = nullptr;
    bool isValid             = false;
};

struct HS_RENDERER_API LightResource
{
    RHIBuffer* lightBuffer = nullptr;
};

// Forward declarations
struct SceneResource;
class ShaderLibrary;
class Scene;

class HS_RENDERER_API RenderResourceManager
{
public:
    RenderResourceManager(RHIContext* rhiContext);
    ~RenderResourceManager();

    // CPU 데이터로부터 GPU-resolved SceneResource 구축
    SceneResource BuildSceneResource(Scene* scene, ShaderLibrary* shaderLibrary);

    // TODO: 지금은 프레임 구분 없이 (프록시)리소스를 생성하는데, 이거 프레임인덱스 받아서 만들어야 한다. 

    // Camera resources (PerView UBO)
    CameraResource* GetOrCreateCameraResource(CameraComponent* camera);
    void SetActiveCameraResource(CameraResource* resource);
    
    LightResource* GetOrCreateLightResource(LightComponent* light, TransformComponent* trnasform);

    // Material resources (cached)
    MaterialResource* GetOrCreateMaterialResources(Material* material);
    RHIGraphicsPipeline* GetOrCreatePipeline(Material* material, RHIRenderPass* renderPass);

    // Mesh resources (cached)
    MeshResource* GetOrCreateMeshResources(Mesh* mesh, const ShaderReflectionDataEx& reflection);

    // Image resources (cached)
    ImageResource* GetOrCreateImageResource(Image* image);

    void ReleaseAll();

private:
    RHIBuffer* getOrCreatePerDrawBuffer(TransformComponent* transform);

    MaterialResource createMaterialResources(Material* material);
    ImageResource createImageResource(Image* image);
    RHIResourceLayout* createResourceLayoutFromReflection(const ShaderReflectionDataEx& reflection, Material* material);
    std::vector<float> buildInterleavedVertexData(Mesh* mesh, const ShaderVertexInputLayout& vertexLayout);

    RHIContext* _rhiContext;

    CameraResource* _activeCameraResource = nullptr;
    RHIBuffer* _activePerDrawBuffer       = nullptr;

    std::unordered_map<CameraComponent*, CameraResource> _cameraResources;
    std::unordered_map<LightComponent*, LightResource> _lightResources;
    std::unordered_map<TransformComponent*, RHIBuffer*> _perDrawBuffers;
    std::unordered_map<Material*, MaterialResource> _materialResources;
    std::unordered_map<Mesh*, MeshResource> _meshResources;
    std::unordered_map<Image*, ImageResource> _imageResources;
};

HS_NS_END

#endif
