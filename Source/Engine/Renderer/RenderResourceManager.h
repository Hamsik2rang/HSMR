//
//  RenderResourceManager.h
//  HSMR
//
//  Automated RHI resource creation from shader reflection data.
//
#ifndef __HS_RENDER_RESOURCE_MANAGER_H__
#define __HS_RENDER_RESOURCE_MANAGER_H__

#include "Precompile.h"
#include "Engine/Resource/ResourceDefinition.h"
#include "ShaderSystem/ShaderSystemDefinition.h"

#include <unordered_map>
#include <vector>

namespace hs
{
class RHIContext;
class RHIShader;
class RHIBuffer;
class RHIResourceLayout;
class RHIResourceSet;
class RHIGraphicsPipeline;
class RHIRenderPass;
class Material;
class Mesh;
class Shader;
class Camera;
class Model;
} // namespace hs

HS_NS_BEGIN

// Cached RHI resources per material
struct HS_API MaterialResource
{
    RHIShader* vertexShader = nullptr;
    RHIShader* fragmentShader = nullptr;
    RHIResourceLayout* resourceLayout = nullptr;
    RHIResourceSet* resourceSet = nullptr;
    std::vector<RHIBuffer*> materialBuffers;
    std::unordered_map<size_t, RHIGraphicsPipeline*> pipelineCache;
    bool isValid = false;
};

// Cached RHI resources per mesh
struct HS_API MeshResource
{
    RHIBuffer* vertexBuffer = nullptr;
    RHIBuffer* indexBuffer = nullptr;
    uint32 indexCount = 0;
    bool isValid = false;
};

// Cached RHI resources per camera (PerView UBO)
struct HS_API CameraResource
{
    RHIBuffer* perViewBuffer = nullptr;
    bool isValid = false;
};

// Cached RHI resources per model (PerDraw UBO)
struct HS_API ModelResource
{
    RHIBuffer* perDrawBuffer = nullptr;
    bool isValid = false;
};

class HS_API RenderResourceManager
{
public:
    RenderResourceManager(RHIContext* rhiContext);
    ~RenderResourceManager();

    // Camera resources (PerView UBO, cached per camera)
    CameraResource* GetOrCreateCameraResource(Camera* camera);
    void SetActiveCameraResource(CameraResource* resource);

    // Model resources (PerDraw UBO, cached per model)
    ModelResource* GetOrCreateModelResource(Model* model);
    void SetActiveModelResource(ModelResource* resource);

    // Material resources (cached)
    MaterialResource* GetOrCreateMaterialResources(Material* material);
    RHIGraphicsPipeline* GetOrCreatePipeline(Material* material, RHIRenderPass* renderPass);

    // Mesh resources (cached)
    MeshResource* GetOrCreateMeshResources(Mesh* mesh, const ShaderReflectionDataEx& reflection);

    void ReleaseAll();

private:
    MaterialResource createMaterialResources(Material* material);
    RHIResourceLayout* createResourceLayoutFromReflection(const ShaderReflectionDataEx& reflection);
    std::vector<float> buildInterleavedVertexData(Mesh* mesh, const ShaderVertexInputLayout& vertexLayout);

    RHIContext* _rhiContext;

    CameraResource* _activeCameraResource = nullptr;
    ModelResource* _activeModelResource = nullptr;

    std::unordered_map<Camera*, CameraResource> _cameraResources;
    std::unordered_map<Model*, ModelResource> _modelResources;
    std::unordered_map<Material*, MaterialResource> _materialResources;
    std::unordered_map<Mesh*, MeshResource> _meshResources;
};

HS_NS_END

#endif
