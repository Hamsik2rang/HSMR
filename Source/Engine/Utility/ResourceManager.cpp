#include "Engine/Utility/ResourceManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Engine/Core/Log.h"
#include "Engine/Object/Mesh.h"
#include "Engine/Object/Material.h"

#include <unordered_map>
#include <filesystem>

HS_NS_BEGIN

std::string ResourceManager::_resourcePath = "";

// Forward declarations
Scoped<Mesh> ProcessNode(aiNode* node, const aiScene* scene, std::vector<Scoped<Material>>& materials);
Scoped<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Scoped<Material>>& materials);
std::vector<Scoped<Material>> ProcessMaterials(const aiScene* scene, const std::string& modelDirectory);

// Helper function to convert aiVector3D to float vector
std::vector<float> ConvertToFloatVector(const aiVector3D* data, uint32 count, uint32 components = 3)
{
    std::vector<float> result;
    result.reserve(count * components);
    
    for (uint32 i = 0; i < count; ++i)
    {
        result.push_back(data[i].x);
        result.push_back(data[i].y);
        result.push_back(data[i].z);
    }
    
    return result;
}

// Helper function to convert texture coordinates
std::vector<float> ConvertTexCoords(const aiVector3D* data, uint32 count)
{
    std::vector<float> result;
    result.reserve(count * 2); // UV only (2 components)
    
    for (uint32 i = 0; i < count; ++i)
    {
        result.push_back(data[i].x);
        result.push_back(data[i].y);
    }
    
    return result;
}

// Helper function to convert colors
std::vector<float> ConvertColors(const aiColor4D* data, uint32 count)
{
    std::vector<float> result;
    result.reserve(count * 4);
    
    for (uint32 i = 0; i < count; ++i)
    {
        result.push_back(data[i].r);
        result.push_back(data[i].g);
        result.push_back(data[i].b);
        result.push_back(data[i].a);
    }
    
    return result;
}

// Helper function to convert aiTextureType to EMaterialTextureType
EMaterialTextureType ConvertTextureType(aiTextureType type)
{
    switch (type)
    {
    case aiTextureType_DIFFUSE:
        return EMaterialTextureType::DIFFUSE;
    case aiTextureType_SPECULAR:
        return EMaterialTextureType::SPECULAR;
    case aiTextureType_NORMALS:
        return EMaterialTextureType::NORMAL;
    case aiTextureType_EMISSIVE:
        return EMaterialTextureType::EMISSION;
    case aiTextureType_AMBIENT:
        return EMaterialTextureType::AMBIENT;
    case aiTextureType_METALNESS:
        return EMaterialTextureType::METALLIC;
    case aiTextureType_DIFFUSE_ROUGHNESS:
        return EMaterialTextureType::ROUGHNESS;
    case aiTextureType_AMBIENT_OCCLUSION:
        return EMaterialTextureType::AMBIENT_OCCLUSION;
    default:
        return EMaterialTextureType::DIFFUSE; // Default to diffuse
    }
}

// Process materials from the scene
std::vector<Scoped<Material>> ProcessMaterials(const aiScene* scene, const std::string& modelDirectory)
{
    std::vector<Scoped<Material>> materials;
    materials.reserve(scene->mNumMaterials);
    
    for (uint32 i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* aiMat = scene->mMaterials[i];
        Scoped<Material> material = hs_make_scoped<Material>();
        
        // Get material name
        aiString name;
        if (aiMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
        {
            material->name = name.C_Str();
        }
        
        // Get diffuse color
        aiColor4D diffuse;
        if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
        {
            material->SetDiffuseColor(glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a));
        }
        
        // Get specular color
        aiColor4D specular;
        if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS)
        {
            material->SetSpecularColor(glm::vec4(specular.r, specular.g, specular.b, specular.a));
        }
        
        // Get emission color
        aiColor4D emission;
        if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emission) == AI_SUCCESS)
        {
            material->SetEmissionColor(glm::vec4(emission.r, emission.g, emission.b, emission.a));
        }
        
        // Get ambient color
        aiColor4D ambient;
        if (aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS)
        {
            material->SetAmbientColor(glm::vec4(ambient.r, ambient.g, ambient.b, ambient.a));
        }
        
        // Get shininess
        float shininess;
        if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
        {
            material->SetShininess(shininess);
        }
        
        // Get opacity
        float opacity;
        if (aiMat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
        {
            material->SetOpacity(opacity);
        }
        
        // Check two-sided
        int twoSided;
        if (aiMat->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS)
        {
            material->SetTwoSided(twoSided != 0);
        }
        
        // Load textures
        for (aiTextureType type = aiTextureType_DIFFUSE; type <= aiTextureType_AMBIENT_OCCLUSION; type = (aiTextureType)(type + 1))
        {
            uint32 textureCount = aiMat->GetTextureCount(type);
            if (textureCount > 0)
            {
                aiString path;
                if (aiMat->GetTexture(type, 0, &path) == AI_SUCCESS) // Load first texture of each type
                {
                    std::string texturePath = path.C_Str();
                    
                    // Handle relative paths
                    if (!std::filesystem::path(texturePath).is_absolute())
                    {
                        texturePath = modelDirectory + "/" + texturePath;
                    }
                    
                    HS_LOG(info, "Loading texture: %s for material %s", texturePath.c_str(), material->name);
                    
                    // Load the texture
                    Scoped<Image> texture = ResourceManager::LoadImageFromFile(texturePath, true);
                    if (texture)
                    {
                        EMaterialTextureType hsTextureType = ConvertTextureType(type);
                        material->SetTexture(hsTextureType, texture.release());
                        HS_LOG(info, "Successfully loaded texture for material %s", material->name);
                    }
                    else
                    {
                        HS_LOG(warning, "Failed to load texture: %s", texturePath.c_str());
                    }
                }
            }
        }
        
        materials.push_back(std::move(material));
    }
    
    return materials;
}

// Process a single mesh
Scoped<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Scoped<Material>>& materials)
{
    Scoped<Mesh> hsMesh = hs_make_scoped<Mesh>();
    
    // Set mesh name
    if (mesh->mName.length > 0)
    {
        hsMesh->name = mesh->mName.C_Str();
    }
    
    // Process vertices
    if (mesh->HasPositions())
    {
        hsMesh->SetPosition(ConvertToFloatVector(mesh->mVertices, mesh->mNumVertices));
    }
    
    // Process normals
    if (mesh->HasNormals())
    {
        hsMesh->SetNormal(ConvertToFloatVector(mesh->mNormals, mesh->mNumVertices));
    }
    
    // Process texture coordinates
    for (uint32 i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i)
    {
        if (mesh->HasTextureCoords(i))
        {
            hsMesh->SetTexCoord(ConvertTexCoords(mesh->mTextureCoords[i], mesh->mNumVertices), i);
        }
    }
    
    // Process vertex colors
    if (mesh->HasVertexColors(0))
    {
        hsMesh->SetColor(ConvertColors(mesh->mColors[0], mesh->mNumVertices));
    }
    
    // Process tangents and bitangents
    if (mesh->HasTangentsAndBitangents())
    {
        hsMesh->SetTangent(ConvertToFloatVector(mesh->mTangents, mesh->mNumVertices));
        hsMesh->SetBitangent(ConvertToFloatVector(mesh->mBitangents, mesh->mNumVertices));
    }
    
    // Process indices
    std::vector<uint32> indices;
    indices.reserve(mesh->mNumFaces * 3); // Assuming triangles
    
    for (uint32 i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace& face = mesh->mFaces[i];
        // We triangulated the mesh, so each face should have 3 indices
        for (uint32 j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }
    
    hsMesh->SetIndices(std::move(indices));
    
    // Associate material with mesh
    if (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < materials.size())
    {
        hsMesh->SetMaterialIndex(mesh->mMaterialIndex);
        HS_LOG(info, "Mesh %s uses material: %s (index: %d)", 
               mesh->mName.C_Str(), 
               materials[mesh->mMaterialIndex]->name,
               mesh->mMaterialIndex);
    }
    
    return hsMesh;
}

// Process a node and all its children
Scoped<Mesh> ProcessNode(aiNode* node, const aiScene* scene, std::vector<Scoped<Material>>& materials)
{
    Scoped<Mesh> rootMesh = hs_make_scoped<Mesh>();
    
    // Process all meshes in this node
    for (uint32 i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Scoped<Mesh> processedMesh = ProcessMesh(mesh, scene, materials);
        
        if (processedMesh)
        {
            // If this is the first mesh and rootMesh is empty, use it as root
            if (i == 0 && rootMesh->GetPosition().empty())
            {
                rootMesh = std::move(processedMesh);
            }
            else
            {
                // Otherwise add as submesh
                rootMesh->AddSubMesh(processedMesh.release());
            }
        }
    }
    
    // Process all children
    for (uint32 i = 0; i < node->mNumChildren; ++i)
    {
        Scoped<Mesh> childMesh = ProcessNode(node->mChildren[i], scene, materials);
        if (childMesh && !childMesh->GetPosition().empty())
        {
            rootMesh->AddSubMesh(childMesh.release());
        }
    }
    
    return rootMesh;
}

bool ResourceManager::Initialize()
{
    // Get resource path from engine context
    EngineContext* context = hs_engine_get_context();
    if (context)
    {
        _resourcePath = context->resourceDirectory;
        if (!_resourcePath.empty() && _resourcePath.back() != '/' && _resourcePath.back() != '\\')
        {
            _resourcePath += "/";
        }
    }
    
    HS_LOG(info, "ResourceManager initialized with path: %s", _resourcePath.c_str());
    return true;
}

void ResourceManager::Finalize()
{
    // Cleanup if needed
}

Scoped<Image> ResourceManager::LoadImageFromFile(const std::string& path, bool isAbsolutePath)
{
    int width   = 0;
    int height  = 0;
    int channel = 0;

    const char* filePath = nullptr;
    if (isAbsolutePath)
    {
        filePath = path.c_str();
    }
    else
    {
        filePath = (_resourcePath + path).c_str();
    }

    uint8* rawData = nullptr;
    rawData        = stbi_load(filePath, &width, &height, &channel, 0);

    if (rawData == nullptr)
    {
        HS_LOG(error, "Fail to load Image!");
        return nullptr;
    }

    Scoped<Image> pImage = hs_make_scoped<Image>(rawData, width, height, channel);

    return pImage;
}

Scoped<Mesh> ResourceManager::LoadMeshFromFile(const std::string& path, bool isAbsolutePath)
{
    const char* filePath = nullptr;
    if (isAbsolutePath)
    {
        filePath = path.c_str();
    }
    else
    {
        filePath = (_resourcePath + path).c_str();
    }
    
    Assimp::Importer importer;
    
    // Configure import flags
    uint32 importFlags = aiProcess_Triangulate |              // Convert all faces to triangles
                        aiProcess_CalcTangentSpace |          // Calculate tangents and bitangents
                        aiProcess_GenNormals |                // Generate normals if not present
                        aiProcess_GenSmoothNormals |          // Generate smooth normals
                        aiProcess_JoinIdenticalVertices |     // Join identical vertices
                        aiProcess_OptimizeMeshes |            // Optimize mesh data
                        aiProcess_ValidateDataStructure |     // Validate the data structure
                        aiProcess_ImproveCacheLocality |      // Improve vertex cache locality
                        aiProcess_RemoveRedundantMaterials |  // Remove redundant materials
                        aiProcess_FixInfacingNormals |        // Fix normals pointing inward
                        aiProcess_SortByPType |               // Sort by primitive type
                        aiProcess_FindInvalidData |           // Find and remove invalid data
                        aiProcess_GenUVCoords |               // Generate UV coordinates if not present
                        aiProcess_TransformUVCoords |         // Transform UV coordinates
                        aiProcess_FlipUVs;                    // Flip UV coordinates for OpenGL
    
    // Note: Removed aiProcess_ConvertToLeftHanded as it might cause issues with some models
    // Add it back if needed for specific coordinate system requirements
    
    const aiScene* scene = importer.ReadFile(filePath, importFlags);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        HS_LOG(error, "ResourceManager cannot import mesh (%s): %s", filePath, importer.GetErrorString());
        return nullptr;
    }
    
    HS_LOG(info, "Loading mesh: %s", filePath);
    HS_LOG(info, "  - Meshes: %d", scene->mNumMeshes);
    HS_LOG(info, "  - Materials: %d", scene->mNumMaterials);
    HS_LOG(info, "  - Animations: %d", scene->mNumAnimations);
    
    // Extract directory from file path for texture loading
    std::string modelDirectory;
    size_t lastSlash = std::string(filePath).find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        modelDirectory = std::string(filePath).substr(0, lastSlash);
    }
    
    // First process all materials
    std::vector<Scoped<Material>> materials = ProcessMaterials(scene, modelDirectory);
    
    // Process the scene starting from root node
    Scoped<Mesh> rootMesh = ProcessNode(scene->mRootNode, scene, materials);
    
    if (!rootMesh || rootMesh->GetPosition().empty())
    {
        HS_LOG(error, "Failed to process any meshes from file: %s", filePath);
        return nullptr;
    }
    
    HS_LOG(info, "Successfully loaded mesh: %s", filePath);
    
    return rootMesh;
}

void ResourceManager::FreeImage(Image* image)
{
    uint8* data = image->GetRawData();
    if (nullptr == data)
    {
        return;
    }

    stbi_image_free(static_cast<void*>(data));
}

void ResourceManager::FreeMesh(Mesh* mesh)
{
    // The mesh destructor should handle cleanup
    // This is here for any additional cleanup if needed
}

HS_NS_END
