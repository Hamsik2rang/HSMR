#include "Resource/ObjectManager.h"

#include "Resource/ResourceDefinition.h"

#include "Core/HAL/FileSystem.h"

#include "Core/Log.h"
#include "Core/Math/Common.h"

#include "Resource/Image.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"
#include "Resource/Shader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <unordered_map>
#include <thread>

HS_NS_BEGIN

bool ObjectManager::s_isInitialize        = false;
std::string ObjectManager::s_resourcePath = "";

Scoped<Image> ObjectManager::s_fallbackImage2DWhite;
Scoped<Image> ObjectManager::s_fallbackImage2DBlack;
Scoped<Image> ObjectManager::s_fallbackImage2DRed;
Scoped<Image> ObjectManager::s_fallbackImage2DGreen;
Scoped<Image> ObjectManager::s_fallbackImage2DBlue;

Scoped<Mesh> ObjectManager::s_fallbackMeshPlane;
Scoped<Mesh> ObjectManager::s_fallbackMeshCube;
Scoped<Mesh> ObjectManager::s_fallbackMeshSphere;

std::unordered_map<std::string, Scoped<Model>> ObjectManager::s_modelCache;

bool ObjectManager::Initialize()
{
    // Get resource path from engine context
    SystemContext* sysContext = SystemContext::Get();
    if (sysContext)
    {
        s_resourcePath = sysContext->assetDirectory;
        if (!s_resourcePath.empty() && s_resourcePath.back() != HS_DIR_SEPERATOR)
        {
            s_resourcePath += HS_DIR_SEPERATOR;
        }
    }

    HS_LOG(info, "ObjectManager initialized with path: %s", s_resourcePath.c_str());

    // 1x1 White Image 2D
    {
        uint8 whitePixel[4]    = {255, 255, 255, 255}; // RGBA
        s_fallbackImage2DWhite = MakeScoped<Image>(whitePixel, 1, 1, 4);
        s_fallbackImage2DWhite->SetType(Image::ImageType::Buffer);
    }

    // 1x1 Black Image 2D
    {
        uint8 blackPixel[4]    = {0, 0, 0, 255}; // RGBA
        s_fallbackImage2DBlack = MakeScoped<Image>(blackPixel, 1, 1, 4);
        s_fallbackImage2DBlack->SetType(Image::ImageType::Buffer);
    }

    // 1x1 Red Image 2D
    {
        uint8 redPixel[4]    = {255, 0, 0, 255}; // RGBA
        s_fallbackImage2DRed = MakeScoped<Image>(redPixel, 1, 1, 4);
        s_fallbackImage2DRed->SetType(Image::ImageType::Buffer);
    }

    // 1x1 Green Image 2D
    {
        uint8 greenPixel[4]    = {0, 255, 0, 255}; // RGBA
        s_fallbackImage2DGreen = MakeScoped<Image>(greenPixel, 1, 1, 4);
        s_fallbackImage2DGreen->SetType(Image::ImageType::Buffer);
    }

    // 1x1 Blue Image 2D
    {
        uint8 bluePixel[4]    = {0, 0, 255, 255}; // RGBA
        s_fallbackImage2DBlue = MakeScoped<Image>(bluePixel, 1, 1, 4);
        s_fallbackImage2DBlue->SetType(Image::ImageType::Buffer);
    }

    // Create fallback meshes
    s_fallbackMeshPlane  = MakeScoped<Mesh>();
    s_fallbackMeshCube   = MakeScoped<Mesh>();
    s_fallbackMeshSphere = MakeScoped<Mesh>();

    calculatePlane();
    calculateCube();
    calculateSphere();

    s_isInitialize = true;

    return s_isInitialize;
}

void ObjectManager::Finalize()
{
    if (!s_isInitialize)
    {
        return;
    }
    if (s_fallbackImage2DBlack)
    {
        s_fallbackImage2DBlack = nullptr;
    }
    if (s_fallbackImage2DWhite)
    {
        s_fallbackImage2DWhite = nullptr;
    }
    if (s_fallbackImage2DRed)
    {
        s_fallbackImage2DRed = nullptr;
    }
    if (s_fallbackImage2DGreen)
    {
        s_fallbackImage2DGreen = nullptr;
    }
    if (s_fallbackImage2DBlue)
    {
        s_fallbackImage2DBlue = nullptr;
    }

    // Clean up fallback meshes
    if (s_fallbackMeshPlane)
    {
        s_fallbackMeshPlane = nullptr;
    }
    if (s_fallbackMeshCube)
    {
        s_fallbackMeshCube = nullptr;
    }
    if (s_fallbackMeshSphere)
    {
        s_fallbackMeshSphere = nullptr;
    }

    s_modelCache.clear();

    s_isInitialize = false;
}

// Forward declarations
static Scoped<Mesh> ProcessNode(aiNode* node, const aiScene* scene, std::vector<Scoped<Material>>& materials);
static Scoped<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Scoped<Material>>& materials);
static std::vector<Scoped<Material>> ProcessMaterial(const aiScene* scene, const std::string& modelDirectory);

// Helper function to convert aiVector3D to float vector
static std::vector<float> ConvertToFloatVector(const aiVector3D* data, uint32 count, uint32 components = 3)
{
    std::vector<float> result;
    result.reserve(count * components);

    for (uint32 i = 0; i < count; ++i)
    {
        // Assimp이 이미 LH 변환을 수행한 데이터를 float 배열로 변환
        result.push_back(data[i].x);
        result.push_back(data[i].y);
        result.push_back(data[i].z);
    }

    return result;
}

// Helper function to convert texture coordinates
static std::vector<float> ConvertToTexCoords(const aiVector3D* data, uint32 count)
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
static std::vector<float> ConvertColors(const aiColor4D* data, uint32 count)
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
static EMaterialTextureType ConvertTextureType(aiTextureType type)
{
    switch (type)
    {
    case aiTextureType_DIFFUSE:
        return EMaterialTextureType::Diffuse;
    case aiTextureType_SPECULAR:
        return EMaterialTextureType::Specular;
    case aiTextureType_NORMALS:
        return EMaterialTextureType::Normal;
    case aiTextureType_EMISSIVE:
        return EMaterialTextureType::Emission;
    case aiTextureType_AMBIENT:
        return EMaterialTextureType::Ambient;
    case aiTextureType_METALNESS:
        return EMaterialTextureType::Metallic;
    case aiTextureType_DIFFUSE_ROUGHNESS:
        return EMaterialTextureType::Roughness;
    case aiTextureType_AMBIENT_OCCLUSION:
        return EMaterialTextureType::AmbientOcclusion;
    default:
        return EMaterialTextureType::Diffuse; // Default to diffuse
    }
}

// Process materials from the scene
static std::vector<Scoped<Material>> ProcessMaterial(const aiScene* scene, const std::string& modelDirectory)
{
    std::vector<Scoped<Material>> materials;
    materials.reserve(scene->mNumMaterials);

    for (uint32 i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* aiMat         = scene->mMaterials[i];
        Scoped<Material> material = MakeScoped<Material>();

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
                    if (!FileSystem::IsAbsolutePath(texturePath))
                    {
                        texturePath = modelDirectory + "/" + texturePath;
                    }

                    HS_LOG(info, "Loading texture: %s for material %s", texturePath.c_str(), material->name);

                    // Load the texture
                    Scoped<Image> texture = ObjectManager::LoadImageFromFile(texturePath, true);
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
static Scoped<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Scoped<Material>>& materials)
{
    Scoped<Mesh> hsMesh = MakeScoped<Mesh>();

    // Set mesh name
    if (mesh->mName.length > 0)
    {
        hsMesh->name = mesh->mName.C_Str();
    }

    // Process vertices
    if (mesh->HasPositions())
    {
        ConvertToFloatVector(mesh->mVertices, mesh->mNumVertices);
        hsMesh->SetPosition(std::move(ConvertToFloatVector(mesh->mVertices, mesh->mNumVertices)));
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
            hsMesh->SetTexCoord(ConvertToTexCoords(mesh->mTextureCoords[i], mesh->mNumVertices), i);
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
        HS_LOG(info, "Mesh %s uses material: %s (index: %d)", mesh->mName.C_Str(), materials[mesh->mMaterialIndex]->name, mesh->mMaterialIndex);
    }

    return hsMesh;
}

// Process a node and all its children
static Scoped<Mesh> ProcessNode(aiNode* node, const aiScene* scene, std::vector<Scoped<Material>>& materials)
{
    Scoped<Mesh> rootMesh = MakeScoped<Mesh>();

    // Process all meshes in this node
    for (uint32 i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh               = scene->mMeshes[node->mMeshes[i]];
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

Scoped<Image> ObjectManager::LoadImageFromFile(const std::string& path, bool isAbsolutePath)
{
    int width   = 0;
    int height  = 0;
    int channel = 0;

    std::string filePath;
    if (isAbsolutePath)
    {
        filePath = path;
    }
    else
    {
        filePath = FileSystem::GetAbsolutePath(path);
    }

    uint8* rawData = nullptr;
    rawData        = stbi_load(filePath.c_str(), &width, &height, &channel, 0);

    if (rawData == nullptr)
    {
        HS_LOG(error, "Fail to load Image!");
        return nullptr;
    }

    Scoped<Image> pImage = MakeScoped<Image>(rawData, width, height, channel);

    return pImage;
}

Scoped<Mesh> ObjectManager::LoadMeshFromFile(const std::string& path, bool isAbsolutePath)
{
    std::string filePath = "";
    if (isAbsolutePath)
    {
        filePath = path.c_str();
    }
    else
    {
        filePath = (s_resourcePath + path).c_str();
    }

    Assimp::Importer importer;

    // Configure import flags
    uint32 importFlags = aiProcess_Triangulate |              // Convert all faces to triangles
                         aiProcess_CalcTangentSpace |         // Calculate tangents and bitangents
                         aiProcess_GenNormals |               // Generate normals if not present
                         aiProcess_GenSmoothNormals |         // Generate smooth normals
                         aiProcess_JoinIdenticalVertices |    // Join identical vertices
                         aiProcess_OptimizeMeshes |           // Optimize mesh data
                         aiProcess_ValidateDataStructure |    // Validate the data structure
                         aiProcess_ImproveCacheLocality |     // Improve vertex cache locality
                         aiProcess_RemoveRedundantMaterials | // Remove redundant materials
                         aiProcess_FixInfacingNormals |       // Fix normals pointing inward
                         aiProcess_SortByPType |              // Sort by primitive type
                         aiProcess_FindInvalidData |          // Find and remove invalid data
                         aiProcess_GenUVCoords |              // Generate UV coordinates if not present
                         aiProcess_TransformUVCoords |        // Transform UV coordinates
                         aiProcess_MakeLeftHanded |           // RH→LH 좌표 변환 (Z축 반전)
                         aiProcess_FlipUVs;                   // UV V좌표 반전

    const aiScene* scene = importer.ReadFile(filePath, importFlags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        HS_LOG(error, "ObjectManager cannot import mesh (%s): %s", filePath.c_str(), importer.GetErrorString());
        return nullptr;
    }

    HS_LOG(info, "Loading mesh: %s", filePath.c_str());
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
    std::vector<Scoped<Material>> materials = ProcessMaterial(scene, modelDirectory);

    // Process the scene starting from root node
    Scoped<Mesh> rootMesh = ProcessNode(scene->mRootNode, scene, materials);

    if (!rootMesh || rootMesh->GetPosition().empty())
    {
        HS_LOG(error, "Failed to process any meshes from file: %s", filePath.c_str());
        return nullptr;
    }

    HS_LOG(info, "Successfully loaded mesh: %s", filePath.c_str());

    return rootMesh;
}

void ObjectManager::FreeImage(Image* image)
{
    uint8* data = image->GetRawData();
    if (nullptr == data)
    {
        return;
    }

    stbi_image_free(static_cast<void*>(data));
}

void ObjectManager::FreeMesh(Mesh* mesh)
{
    // The mesh destructor should handle cleanup
    // This is here for any additional cleanup if needed
}

Scoped<Shader> ObjectManager::LoadShaderFromFile(const std::string& path, EShaderStage stage, const char* entryName, bool isAbsolutePath)
{
    std::string shaderPath = path;
    if (isAbsolutePath == false)
    {
        shaderPath = s_resourcePath + path;
    }

    if (FileSystem::Exist(shaderPath) == false)
    {
        HS_LOG(error, "Shader file does not exist: %s", shaderPath.c_str());
        return nullptr;
    }
    FileHandle fHandle;
    FileSystem::Open(shaderPath, EFileAccess::ReadOnly, fHandle);

    size_t sourceLen = FileSystem::GetSize(fHandle);
    if (sourceLen == 0)
    {
        HS_LOG(error, "Shader file is empty: %s", shaderPath.c_str());
        FileSystem::Close(fHandle);
        return nullptr;
    }

    std::string sourceCode;
    sourceCode.resize(sourceLen + 1);
    size_t bytesRead = FileSystem::Read(fHandle, sourceCode.data(), sourceLen);
    if (bytesRead != sourceLen)
    {
        HS_LOG(error, "Failed to read entire shader file: %s", shaderPath.c_str());
        FileSystem::Close(fHandle);
        return nullptr;
    }

    FileSystem::Close(fHandle);
    sourceCode[sourceLen] = '\0'; // Null-terminate

    Scoped<Shader> shader = MakeScoped<Shader>(sourceCode, stage, entryName);

    return shader;
}

Scoped<Shader> ObjectManager::LoadShaderFromSource(const std::string& shaderName, const std::string& sourceCode,
                                                    EShaderStage requestedStages,
                                                    const std::vector<std::string>& includePaths)
{
    if (sourceCode.empty())
    {
        HS_LOG(error, "Shader source code is empty for '%s'", shaderName.c_str());
        return nullptr;
    }

    Scoped<Shader> shader = MakeScoped<Shader>(shaderName, sourceCode, requestedStages, includePaths);
    return shader;
}

void ObjectManager::FreeShader(Shader* shader)
{
    // The shader destructor should handle cleanup
    // This is here for any additional cleanup if needed
}

void ObjectManager::calculatePlane()
{
    // Plane Mesh (LH)

    // 1x1 평면, 중앙이 원점, Y축이 위
    std::vector<float> positions = {
        -0.5f, 0.0f, 0.5f, // 0: 왼쪽 위
        0.5f, 0.0f, 0.5f,  // 1: 오른쪽 위
        0.5f, 0.0f, -0.5f, // 2: 오른쪽 아래
        -0.5f, 0.0f, -0.5f // 3: 왼쪽 아래
    };

    std::vector<float> normals = {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    std::vector<float> texcoords = {
        0.0f, 0.0f, // 0: 왼쪽 위
        1.0f, 0.0f, // 1: 오른쪽 위
        1.0f, 1.0f, // 2: 오른쪽 아래
        0.0f, 1.0f  // 3: 왼쪽 아래
    };

    std::vector<float> tangents = {
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f
    };

    std::vector<float> bitangents = {
        0.0f, 0.0f, 1.0f, // 왼손 좌표계에서 Z+ 방향
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f
    };

    std::vector<uint32> indices = {
        0, 1, 2, // 첫 번째 삼각형
        0, 2, 3  // 두 번째 삼각형
    };

    s_fallbackMeshPlane->SetPosition(std::move(positions));
    s_fallbackMeshPlane->SetNormal(std::move(normals));
    s_fallbackMeshPlane->SetTexCoord(std::move(texcoords), 0);
    s_fallbackMeshPlane->SetTangent(std::move(tangents));
    s_fallbackMeshPlane->SetBitangent(std::move(bitangents));
    s_fallbackMeshPlane->SetIndices(std::move(indices));
}
void ObjectManager::calculateCube()
{

    // Cube Mesh (LH)

    // 1x1x1 큐브, 중앙이 원점
    // 왼손 좌표계: X(오른쪽), Y(위), Z(앞쪽)
    std::vector<float> positions = {
        // 앞면 (Z+)
        -0.5f, -0.5f, 0.5f, // 0
        0.5f, -0.5f, 0.5f,  // 1
        0.5f, 0.5f, 0.5f,   // 2
        -0.5f, 0.5f, 0.5f,  // 3

        // 뒷면 (Z-)
        0.5f, -0.5f, -0.5f,  // 4
        -0.5f, -0.5f, -0.5f, // 5
        -0.5f, 0.5f, -0.5f,  // 6
        0.5f, 0.5f, -0.5f,   // 7

        // 윗면 (Y+)
        -0.5f, 0.5f, 0.5f,  // 8
        0.5f, 0.5f, 0.5f,   // 9
        0.5f, 0.5f, -0.5f,  // 10
        -0.5f, 0.5f, -0.5f, // 11

        // 아랫면 (Y-)
        -0.5f, -0.5f, -0.5f, // 12
        0.5f, -0.5f, -0.5f,  // 13
        0.5f, -0.5f, 0.5f,   // 14
        -0.5f, -0.5f, 0.5f,  // 15

        // 오른쪽면 (X+)
        0.5f, -0.5f, 0.5f,  // 16
        0.5f, -0.5f, -0.5f, // 17
        0.5f, 0.5f, -0.5f,  // 18
        0.5f, 0.5f, 0.5f,   // 19

        // 왼쪽면 (X-)
        -0.5f, -0.5f, -0.5f, // 20
        -0.5f, -0.5f, 0.5f,  // 21
        -0.5f, 0.5f, 0.5f,   // 22
        -0.5f, 0.5f, -0.5f   // 23
    };

    std::vector<float> colors(positions.size(), 1.0f);

    std::vector<float> normals = {
        // 앞면
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        // 뒷면
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,

        // 윗면
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        // 아랫면
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,

        // 오른쪽면
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        // 왼쪽면
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f
    };

    std::vector<float> texcoords = {
        // 각 면에 대한 UV 좌표
        // 앞면
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        // 뒷면
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        // 윗면
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // 아랫면
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        // 오른쪽면
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        // 왼쪽면
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };

    std::vector<uint32> indices = {
        // 앞면
        0, 1, 2,
        0, 2, 3,

        // 뒷면
        4, 5, 6,
        4, 6, 7,

        // 윗면
        8, 9, 10,
        8, 10, 11,

        // 아랫면
        12, 13, 14,
        12, 14, 15,

        // 오른쪽면
        16, 17, 18,
        16, 18, 19,

        // 왼쪽면
        20, 21, 22,
        20, 22, 23
    };

    s_fallbackMeshCube->SetPosition(std::move(positions));
    s_fallbackMeshCube->SetColor(std::move(colors));
    s_fallbackMeshCube->SetNormal(std::move(normals));
    s_fallbackMeshCube->SetTexCoord(std::move(texcoords), 0);
    s_fallbackMeshCube->SetIndices(std::move(indices));
}
void ObjectManager::calculateSphere()
{
    // Sphere Mesh (LH)
    const int segments = 32; // 경도 분할 수
    const int rings    = 16; // 위도 분할 수
    const float radius = 0.5f;

    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<uint32> indices;

    // 정점 생성
    for (int ring = 0; ring <= rings; ++ring)
    {
        float phi    = HS_PI * float(ring) / float(rings); // 0 ~ PI
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        for (int segment = 0; segment <= segments; ++segment)
        {
            float theta    = 2.0f * HS_PI * float(segment) / float(segments); // 0 ~ 2PI
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            // 위치 (왼손 좌표계: Z는 앞쪽)
            float x = cosTheta * sinPhi;
            float y = cosPhi;
            float z = sinTheta * sinPhi;

            positions.push_back(x * radius);
            positions.push_back(y * radius);
            positions.push_back(z * radius);
            positions.push_back(1.0f);

            // 법선 (구의 경우 정규화된 위치가 법선)
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);

            // UV 좌표
            float u = float(segment) / float(segments);
            float v = float(ring) / float(rings);
            texcoords.push_back(u);
            texcoords.push_back(v);
        }
    }

    // 인덱스 생성
    for (int ring = 0; ring < rings; ++ring)
    {
        for (int segment = 0; segment < segments; ++segment)
        {
            int current = ring * (segments + 1) + segment;
            int next    = current + segments + 1;

            // 첫 번째 삼각형
            indices.push_back(current);
            indices.push_back(current + 1);
            indices.push_back(next);

            // 두 번째 삼각형
            indices.push_back(current + 1);
            indices.push_back(next + 1);
            indices.push_back(next);
        }
    }

    s_fallbackMeshSphere->SetPosition(std::move(positions));
    s_fallbackMeshSphere->SetNormal(std::move(normals));
    s_fallbackMeshSphere->SetTexCoord(std::move(texcoords), 0);
    s_fallbackMeshSphere->SetIndices(std::move(indices));
}

const Image* ObjectManager::GetFallbackImage2DWhite()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback image.");
    }

    // Return the fallback white image
    return s_fallbackImage2DWhite.get();
}
const Image* ObjectManager::GetFallbackImage2DBlack()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback image.");
    }
    // Return the fallback black image
    return s_fallbackImage2DBlack.get();
}
const Image* ObjectManager::GetFallbackImage2DRed()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback image.");
    }

    return s_fallbackImage2DRed.get(); // Return empty image or handle error appropriately
}
const Image* ObjectManager::GetFallbackImage2DGreen()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback image.");
    }
    return s_fallbackImage2DGreen.get(); // Return empty image or handle error appropriately
}
const Image* ObjectManager::GetFallbackImage2DBlue()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback image.");
    }
    return s_fallbackImage2DBlue.get(); // Return empty image or handle error appropriately
}

const Mesh* ObjectManager::GetFallbackMeshPlane()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback mesh.");
    }
    return s_fallbackMeshPlane.get(); // Return empty mesh or handle error appropriately
}

const Mesh* ObjectManager::GetFallbackMeshCube()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback mesh.");
    }
    return s_fallbackMeshCube.get(); // Return empty mesh or handle error appropriately
}

const Mesh* ObjectManager::GetFallbackMeshSphere()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback mesh.");
    }
    return s_fallbackMeshSphere.get(); // Return empty mesh or handle error appropriately
}

bool ObjectManager::LoadModel(const std::string& path, std::vector<Scoped<Mesh>>& outMeshes, std::vector<Scoped<Material>>& outMaterials, bool isAbsolutePath)
{
    if (path.ends_with(".gltf"))
    {
        return loadGLTF(path, outMeshes, outMaterials, isAbsolutePath);
    }
    std::string filePath = path;
    if (!isAbsolutePath)
    {
        filePath = s_resourcePath + path;
    }

    Assimp::Importer importer;

    uint32 importFlags = aiProcess_Triangulate |
                         aiProcess_CalcTangentSpace |
                         aiProcess_GenNormals |
                         aiProcess_JoinIdenticalVertices |
                         aiProcess_ImproveCacheLocality |
                         aiProcess_LimitBoneWeights |
                         aiProcess_RemoveRedundantMaterials |
                         aiProcess_OptimizeMeshes |
                         aiProcess_GenUVCoords |
                         aiProcess_TransformUVCoords;

    const aiScene* scene = importer.ReadFile(filePath, importFlags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        HS_LOG(error, "Failed to load model: %s", importer.GetErrorString());
        return false;
    }

    HS_LOG(info, "Loading model: %s", filePath.c_str());
    HS_LOG(info, "  - Meshes: %d", scene->mNumMeshes);
    HS_LOG(info, "  - Materials: %d", scene->mNumMaterials);

    std::string modelDirectory;
    size_t lastSlash = filePath.find_last_of(HS_DIR_SEPERATOR);
    if (lastSlash != std::string::npos)
    {
        modelDirectory = filePath.substr(0, lastSlash);
    }

    std::vector<Scoped<Material>> materials;
    materials.reserve(scene->mNumMaterials);

    for (uint32 i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* aiMat         = scene->mMaterials[i];
        Scoped<Material> material = MakeScoped<Material>();

        aiColor4D diffuse;
        if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
        {
            material->SetDiffuseColor(glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a));
        }

        aiColor4D specular;
        if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS)
        {
            material->SetSpecularColor(glm::vec4(specular.r, specular.g, specular.b, specular.a));
        }

        float shininess;
        if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
        {
            material->SetShininess(shininess);
        }

        for (aiTextureType type = aiTextureType_DIFFUSE; type <= aiTextureType_AMBIENT_OCCLUSION; type = (aiTextureType)(type + 1))
        {
            uint32 textureCount = aiMat->GetTextureCount(type);
            if (textureCount > 0)
            {
                aiString path;
                if (aiMat->GetTexture(type, 0, &path) == AI_SUCCESS)
                {
                    std::string texturePath = path.C_Str();
                    if (!FileSystem::IsAbsolutePath(texturePath))
                    {
                        texturePath = modelDirectory + "/" + texturePath;
                    }

                    Scoped<Image> texture = LoadImageFromFile(texturePath, true);
                    if (texture)
                    {
                        EMaterialTextureType hsTextureType = EMaterialTextureType::Diffuse;
                        if (type == aiTextureType_NORMALS) hsTextureType = EMaterialTextureType::Normal;
                        else if (type == aiTextureType_SPECULAR) hsTextureType = EMaterialTextureType::Specular;

                        material->SetTexture(hsTextureType, texture.release());
                    }
                }
            }
        }

        materials.push_back(std::move(material));
    }

    outMeshes.clear();
    outMaterials.clear();

    for (uint32 i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh* mesh        = scene->mMeshes[i];
        Scoped<Mesh> hsMesh = MakeScoped<Mesh>();

        std::vector<float> positions;
        positions.reserve(mesh->mNumVertices * 3);
        for (uint32 j = 0; j < mesh->mNumVertices; ++j)
        {
            positions.push_back(mesh->mVertices[j].x);
            positions.push_back(mesh->mVertices[j].y);
            positions.push_back(mesh->mVertices[j].z);
        }
        hsMesh->SetPosition(std::move(positions));

        if (mesh->HasNormals())
        {
            std::vector<float> normals;
            normals.reserve(mesh->mNumVertices * 3);
            for (uint32 j = 0; j < mesh->mNumVertices; ++j)
            {
                normals.push_back(mesh->mNormals[j].x);
                normals.push_back(mesh->mNormals[j].y);
                normals.push_back(mesh->mNormals[j].z);
            }
            hsMesh->SetNormal(std::move(normals));
        }

        if (mesh->HasTextureCoords(0))
        {
            std::vector<float> texCoords;
            texCoords.reserve(mesh->mNumVertices * 2);
            for (uint32 j = 0; j < mesh->mNumVertices; ++j)
            {
                texCoords.push_back(mesh->mTextureCoords[0][j].x);
                texCoords.push_back(mesh->mTextureCoords[0][j].y);
            }
            hsMesh->SetTexCoord(std::move(texCoords), 0);
        }

        std::vector<uint32> indices;
        indices.reserve(mesh->mNumFaces * 3);
        for (uint32 j = 0; j < mesh->mNumFaces; ++j)
        {
            aiFace& face = mesh->mFaces[j];
            for (uint32 k = 0; k < face.mNumIndices; ++k)
            {
                indices.push_back(face.mIndices[k]);
            }
        }
        hsMesh->SetIndices(std::move(indices));

        if (mesh->mMaterialIndex < materials.size())
        {
            hsMesh->SetMaterialIndex(mesh->mMaterialIndex);
        }

        hsMesh->CalculateBounds();
        outMeshes.push_back(std::move(hsMesh));
    }

    outMaterials = std::move(materials);

    return true;
}

bool ObjectManager::LoadModel(const std::string& path, Scoped<Model>& outModel, bool isAbsolutePath)
{
    std::vector<Scoped<Mesh>> meshes;
    std::vector<Scoped<Material>> materials;
    if (!LoadModel(path, meshes, materials, isAbsolutePath))
    {
        return false;
    }
    outModel = MakeScoped<Model>();
    // TODO: 현재는 첫 번째 메쉬와 머티리얼만 설정
    outModel->SetMesh(std::move(meshes.front()));
    outModel->SetMaterial(std::move(materials.front()));

    return true;
}

ModelLoadResult ObjectManager::LoadModel(const std::string& path, bool isAbsolutePath)
{
    // Cache hit
    auto it = s_modelCache.find(path);
    if (it != s_modelCache.end())
    {
        Model* model = it->second.get();
        return { model->GetMesh(), model->GetMaterial() };
    }

    // Cache miss: load via existing overload
    Scoped<Model> model;
    if (!LoadModel(path, model, isAbsolutePath) || !model)
    {
        return {};
    }

    ModelLoadResult result{ model->GetMesh(), model->GetMaterial() };
    s_modelCache[path] = std::move(model);
    return result;
}

bool ObjectManager::loadGLTF(const std::string& path, std::vector<Scoped<Mesh>>& outMeshes, std::vector<Scoped<Material>>& outMaterials, bool isAbsolutePath)
{
    std::string filePath = path;
    if (!isAbsolutePath)
    {
        filePath = s_resourcePath + path;
    }

    Assimp::Importer importer;

    uint32 importFlags =
        aiProcess_Triangulate |
        aiProcess_CalcTangentSpace |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_OptimizeMeshes |
        aiProcess_GenUVCoords |
        aiProcess_TransformUVCoords |
        aiProcess_SortByPType |
        aiProcess_FindInvalidData |
        aiProcess_PreTransformVertices | // 노드 트랜스폼 베이킹
        aiProcess_MakeLeftHanded |       // RH→LH 좌표 변환 (Z축 반전)
        aiProcess_FlipUVs;               // UV V좌표 반전

    const aiScene* scene = importer.ReadFile(filePath, importFlags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        HS_LOG(error, "Failed to load GLTF: %s - %s", filePath.c_str(), importer.GetErrorString());
        return false;
    }

    HS_LOG(info, "Loading GLTF: %s", filePath.c_str());
    HS_LOG(info, "  - Meshes: %d", scene->mNumMeshes);
    HS_LOG(info, "  - Materials: %d", scene->mNumMaterials);
    HS_LOG(info, "  - Textures (embedded): %d", scene->mNumTextures);
    HS_LOG(info, "  - Animations: %d", scene->mNumAnimations);

    std::string modelDirectory;
    size_t lastSlash = filePath.find_last_of(HS_DIR_SEPERATOR);
    if (lastSlash != std::string::npos)
    {
        modelDirectory = filePath.substr(0, lastSlash);
    }

    // --- Materials (PBR) ---
    std::vector<Scoped<Material>> materials;
    materials.reserve(scene->mNumMaterials);

    for (uint32 i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* aiMat         = scene->mMaterials[i];
        Scoped<Material> material = MakeScoped<Material>();

        // Material name
        aiString matName;
        if (aiMat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS)
        {
            material->name = matName.C_Str();
        }

        // Base color (GLTF PBR)
        aiColor4D baseColor;
        if (aiMat->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS)
        {
            material->SetDiffuseColor(glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a));
        }
        else
        {
            aiColor4D diffuse;
            if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
            {
                material->SetDiffuseColor(glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a));
            }
        }

        // Metallic factor
        float metallic = 0.0f;
        if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
        {
            material->SetMetallic(metallic);
        }

        // Roughness factor
        float roughness = 0.5f;
        if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
        {
            material->SetRoughness(roughness);
        }

        // Emission color
        aiColor4D emission;
        if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emission) == AI_SUCCESS)
        {
            material->SetEmissionColor(glm::vec4(emission.r, emission.g, emission.b, emission.a));
        }

        // Opacity
        float opacity = 1.0f;
        if (aiMat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
        {
            material->SetOpacity(opacity);
        }

        // Two-sided
        int twoSided = 0;
        if (aiMat->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS)
        {
            material->SetTwoSided(twoSided != 0);
        }

        // GLTF texture type mapping
        struct TextureMapping
        {
            aiTextureType aiType;
            EMaterialTextureType hsType;
        };

        TextureMapping textureMappings[] = {
            {aiTextureType_BASE_COLOR, EMaterialTextureType::Diffuse},
            {aiTextureType_DIFFUSE, EMaterialTextureType::Diffuse},
            {aiTextureType_NORMALS, EMaterialTextureType::Normal},
            {aiTextureType_METALNESS, EMaterialTextureType::Metallic},
            {aiTextureType_DIFFUSE_ROUGHNESS, EMaterialTextureType::Roughness},
            {aiTextureType_EMISSIVE, EMaterialTextureType::Emission},
            {aiTextureType_AMBIENT_OCCLUSION, EMaterialTextureType::AmbientOcclusion},
        };

        std::unordered_map<EMaterialTextureType, const char*> textureTypeStringMap =
            {
                {EMaterialTextureType::Diffuse, "DIFFUSE"},
                {EMaterialTextureType::Normal, "NORMAL"},
                {EMaterialTextureType::Metallic, "METALLIC"},
                {EMaterialTextureType::Roughness, "ROUGHNESS"},
                {EMaterialTextureType::Emission, "EMISSION"},
                {EMaterialTextureType::AmbientOcclusion, "AMBIENT_OCCLUSION"},
            };

        for (const auto& mapping : textureMappings)
        {
            if (material->HasTexture(mapping.hsType))
            {
                continue;
            }

            uint32 textureCount = aiMat->GetTextureCount(mapping.aiType);
            if (textureCount == 0)
            {
                continue;
            }

            aiString texPath;
            if (aiMat->GetTexture(mapping.aiType, 0, &texPath) != AI_SUCCESS)
            {
                continue;
            }

            std::string texturePathStr = texPath.C_Str();
            Scoped<Image> texture      = nullptr;

            // GLB 임베디드 텍스처 처리
            if (texturePathStr[0] == '*')
            {
                int texIndex = std::atoi(texturePathStr.c_str() + 1);
                if (texIndex >= 0 && static_cast<uint32>(texIndex) < scene->mNumTextures)
                {
                    const aiTexture* embeddedTex = scene->mTextures[texIndex];

                    if (embeddedTex->mHeight == 0)
                    {
                        // 압축된 텍스처 (PNG, JPEG 등)
                        int width = 0, height = 0, channel = 0;
                        uint8* rawData = stbi_load_from_memory(
                            reinterpret_cast<const uint8*>(embeddedTex->pcData),
                            embeddedTex->mWidth,
                            &width, &height, &channel, 0
                        );

                        if (rawData)
                        {
                            texture = MakeScoped<Image>(rawData, width, height, channel);
                        }
                    }
                    else
                    {
                        // 비압축 ARGB8888 텍스처
                        uint32 pixelCount = embeddedTex->mWidth * embeddedTex->mHeight;
                        std::vector<uint8> rgbaData(pixelCount * 4);

                        for (uint32 p = 0; p < pixelCount; ++p)
                        {
                            rgbaData[p * 4 + 0] = embeddedTex->pcData[p].r;
                            rgbaData[p * 4 + 1] = embeddedTex->pcData[p].g;
                            rgbaData[p * 4 + 2] = embeddedTex->pcData[p].b;
                            rgbaData[p * 4 + 3] = embeddedTex->pcData[p].a;
                        }

                        texture = MakeScoped<Image>(rgbaData.data(), embeddedTex->mWidth, embeddedTex->mHeight, 4);
                    }
                }
            }
            else
            {
                // 외부 텍스처 파일
                if (!FileSystem::IsAbsolutePath(texturePathStr))
                {
                    texturePathStr = modelDirectory + HS_DIR_SEPERATOR + texturePathStr;
                }
                texture = LoadImageFromFile(texturePathStr, true);
            }

            if (texture)
            {
                material->SetTexture(mapping.hsType, texture.release());
                HS_LOG(info, "  Loaded texture [%s]: %s", textureTypeStringMap[mapping.hsType], texturePathStr.c_str());
            }
            else
            {
                HS_LOG(warning, "  Failed to load texture: %s", texturePathStr.c_str());
            }
        }

        materials.push_back(std::move(material));
    }

    // --- Meshes ---
    outMeshes.clear();
    outMaterials.clear();
    outMeshes.reserve(scene->mNumMeshes);

    for (uint32 i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh* mesh        = scene->mMeshes[i];
        Scoped<Mesh> hsMesh = MakeScoped<Mesh>();

        if (mesh->mName.length > 0)
        {
            hsMesh->name = mesh->mName.C_Str();
        }

        // Positions
        if (mesh->HasPositions())
        {
            hsMesh->SetPosition(ConvertToFloatVector(mesh->mVertices, mesh->mNumVertices));
        }

        // Normals
        if (mesh->HasNormals())
        {
            hsMesh->SetNormal(ConvertToFloatVector(mesh->mNormals, mesh->mNumVertices));
        }

        // Tangents & Bitangents
        if (mesh->HasTangentsAndBitangents())
        {
            hsMesh->SetTangent(ConvertToFloatVector(mesh->mTangents, mesh->mNumVertices));
            hsMesh->SetBitangent(ConvertToFloatVector(mesh->mBitangents, mesh->mNumVertices));
        }

        // Texture coordinates (모든 UV 채널)
        for (uint32 ch = 0; ch < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++ch)
        {
            if (mesh->HasTextureCoords(ch))
            {
                hsMesh->SetTexCoord(ConvertToTexCoords(mesh->mTextureCoords[ch], mesh->mNumVertices), ch);
            }
        }

        // Vertex colors
        if (mesh->HasVertexColors(0))
        {
            hsMesh->SetColor(ConvertColors(mesh->mColors[0], mesh->mNumVertices));
        }

        // Indices
        std::vector<uint32> indices;
        indices.reserve(mesh->mNumFaces * 3);
        for (uint32 j = 0; j < mesh->mNumFaces; ++j)
        {
            aiFace& face = mesh->mFaces[j];
            for (uint32 k = 0; k < face.mNumIndices; ++k)
            {
                indices.push_back(face.mIndices[k]);
            }
        }
        hsMesh->SetIndices(std::move(indices));

        // Material index
        if (mesh->mMaterialIndex < materials.size())
        {
            hsMesh->SetMaterialIndex(mesh->mMaterialIndex);
        }

        hsMesh->CalculateBounds();
        outMeshes.push_back(std::move(hsMesh));
    }

    outMaterials = std::move(materials);

    HS_LOG(info, "Successfully loaded GLTF: %s (%d meshes, %d materials)", filePath.c_str(), static_cast<int>(outMeshes.size()), static_cast<int>(outMaterials.size()));

    return true;
}

bool ObjectManager::loadGLTF(const std::string& path, Scoped<Model>& outModel, bool isAbsolutePath)
{
    std::vector<Scoped<Mesh>> meshes;
    std::vector<Scoped<Material>> materials;
    if (!loadGLTF(path, meshes, materials, isAbsolutePath))
    {
        return false;
    }
    outModel = MakeScoped<Model>();
    // TODO: 현재는 첫 번째 메쉬와 머티리얼만 설정
    outModel->SetMesh(std::move(meshes.front()));
    outModel->SetMaterial(std::move(materials.front()));

    return true;
}

Scoped<Material> ObjectManager::LoadMaterialFromFile(const std::string& path, bool isAbsolutePath)
{
    // Material loading from file is not yet implemented
    HS_LOG(warning, "LoadMaterialFromFile not implemented yet: %s", path.c_str());
    return nullptr;
}

void ObjectManager::FreeMaterial(Material* material)
{
    // The material destructor should handle cleanup
}

HS_NS_END
