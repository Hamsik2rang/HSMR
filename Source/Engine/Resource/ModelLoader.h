#ifndef __HS_MODEL_LOADER_H__
#define __HS_MODEL_LOADER_H__

#include "Precompile.h"

#include "Core/Math/Common.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace hs { class Mesh; }
namespace hs { class Material; }
namespace hs { class Image; }

HS_NS_BEGIN

struct ModelLoadSettings
{
    bool generateNormals = false;
    bool generateTangents = false;
    bool flipUVs = false;
    bool mergeMeshes = false;
    float scale = 1.0f;
    
    uint32 assimpFlags = aiProcess_Triangulate | 
                         aiProcess_JoinIdenticalVertices |
                         aiProcess_ImproveCacheLocality |
                         aiProcess_LimitBoneWeights |
                         aiProcess_RemoveRedundantMaterials |
                         aiProcess_OptimizeMeshes |
                         aiProcess_GenUVCoords |
                         aiProcess_TransformUVCoords;
};

class HS_API ModelLoader
{
public:
    static bool LoadModel(const std::string& filePath, 
                          std::vector<Scoped<Mesh>>& outMeshes,
                          std::vector<Scoped<Material>>& outMaterials,
                          const ModelLoadSettings& settings = ModelLoadSettings());
    
    static bool LoadModelAsync(const std::string& filePath,
                               std::function<void(const std::vector<Scoped<Mesh>>&, 
                                                 const std::vector<Scoped<Material>>&)> callback,
                               const ModelLoadSettings& settings = ModelLoadSettings());

private:
    static void ProcessNode(aiNode* node, const aiScene* scene,
                           std::vector<Scoped<Mesh>>& outMeshes,
                           std::vector<Scoped<Material>>& outMaterials,
                           const ModelLoadSettings& settings);
    
    static Scoped<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene,
                                   std::vector<Scoped<Material>>& outMaterials,
                                   const ModelLoadSettings& settings);
    
    static Scoped<Material> ProcessMaterial(aiMaterial* material, const aiScene* scene,
                                           const std::string& modelPath,
                                           const ModelLoadSettings& settings);
    
    static Scoped<Image> LoadTexture(const std::string& texturePath, bool isAbsolutePath = false);
    
    static glm::mat4 ConvertMatrix(const aiMatrix4x4& matrix);
    static glm::vec3 ConvertVector3(const aiVector3D& vector);
    static glm::vec2 ConvertVector2(const aiVector2D& vector);
    static glm::vec4 ConvertColor4(const aiColor4D& color);
    
    static std::string GetTexturePath(const std::string& modelPath, const std::string& texturePath);
    static std::string GetFileExtension(const std::string& filePath);
};

HS_NS_END

#endif