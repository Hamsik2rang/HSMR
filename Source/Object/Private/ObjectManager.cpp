#include "Object/ObjectManager.h"

#include "HAL/FileSystem.h"
#include "Core/Math/Math.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Core/Log.h"
#include "Object/Mesh.h"
#include "Object/Material.h"

#include <unordered_map>

HS_NS_BEGIN

bool ObjectManager::s_isInitialize        = false;
std::string ObjectManager::s_resourcePath = "";

Image* ObjectManager::s_fallbackImage2DWhite;
Image* ObjectManager::s_fallbackImage2DBlack;
Image* ObjectManager::s_fallbackImage2DRed;
Image* ObjectManager::s_fallbackImage2DGreen;
Image* ObjectManager::s_fallbackImage2DBlue;

Mesh* ObjectManager::s_fallbackMeshPlane;
Mesh* ObjectManager::s_fallbackMeshCube;
Mesh* ObjectManager::s_fallbackMeshSphere;

bool ObjectManager::Initialize()
{
    // Get resource path from engine context
    SystemContext* sysContext = SystemContext::Get();
    if (sysContext)
    {
        s_resourcePath = sysContext->resourceDirectory;
        if (!s_resourcePath.empty() && s_resourcePath.back() != HS_DIR_SEPERATOR)
        {
            s_resourcePath += HS_DIR_SEPERATOR;
        }
    }

    HS_LOG(info, "ObjectManager initialized with path: %s", s_resourcePath.c_str());

    // 1x1 White Image 2D
    {
        uint8 whitePixel[4]    = {255, 255, 255, 255}; // RGBA
        s_fallbackImage2DWhite = new Image(whitePixel, 1, 1, 4);
        s_fallbackImage2DWhite->SetType(Image::ImageType::BUFFER);
    }

    // 1x1 Black Image 2D
    {
        uint8 blackPixel[4]    = {0, 0, 0, 255}; // RGBA
        s_fallbackImage2DBlack = new Image(blackPixel, 1, 1, 4);
        s_fallbackImage2DBlack->SetType(Image::ImageType::BUFFER);
    }

    // 1x1 Red Image 2D
    {
        uint8 redPixel[4]    = {1, 0, 0, 255}; // RGBA
        s_fallbackImage2DRed = new Image(redPixel, 1, 1, 4);
        s_fallbackImage2DRed->SetType(Image::ImageType::BUFFER);
    }

    // 1x1 Green Image 2D
    {
        uint8 greenPixel[4]    = {0, 1, 0, 255}; // RGBA
        s_fallbackImage2DGreen = new Image(greenPixel, 1, 1, 4);
        s_fallbackImage2DGreen->SetType(Image::ImageType::BUFFER);
    }

    // 1x1 Blue Image 2D
    {
        uint8 bluePixel[4]    = {0, 0, 1, 255}; // RGBA
        s_fallbackImage2DBlue = new Image(bluePixel, 1, 1, 4);
        s_fallbackImage2DBlue->SetType(Image::ImageType::BUFFER);
    }

    calculatePlane();
    calculateCube();
    calculatePlane();

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
        delete s_fallbackImage2DBlack;
        s_fallbackImage2DBlack = nullptr;
    }
    if (s_fallbackImage2DWhite)
    {
        delete s_fallbackImage2DWhite;
        s_fallbackImage2DWhite = nullptr;
    }
    if (s_fallbackImage2DRed)
    {
        delete s_fallbackImage2DRed;
        s_fallbackImage2DRed = nullptr;
    }
    if (s_fallbackImage2DGreen)
    {
        delete s_fallbackImage2DGreen;
        s_fallbackImage2DGreen = nullptr;
    }
    if (s_fallbackImage2DBlue)
    {
        delete s_fallbackImage2DBlue;
        s_fallbackImage2DBlue = nullptr;
    }
    s_isInitialize = false;
}

// Forward declarations
Scoped<Mesh> ProcessNode(aiNode* node, const aiScene* scene, std::vector<Scoped<Material>>& materials);
Scoped<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Scoped<Material>>& materials);
std::vector<Scoped<Material>> ProcessMaterial(const aiScene* scene, const std::string& modelDirectory);

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
std::vector<float> ConvertToCoords(const aiVector3D* data, uint32 count)
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
std::vector<Scoped<Material>> ProcessMaterial(const aiScene* scene, const std::string& modelDirectory)
{
    std::vector<Scoped<Material>> materials;
    materials.reserve(scene->mNumMaterials);

    for (uint32 i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* aiMat         = scene->mMaterials[i];
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
            hsMesh->SetTexCoord(ConvertToCoords(mesh->mTextureCoords[i], mesh->mNumVertices), i);
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
Scoped<Mesh> ProcessNode(aiNode* node, const aiScene* scene, std::vector<Scoped<Material>>& materials)
{
    Scoped<Mesh> rootMesh = hs_make_scoped<Mesh>();

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

    Scoped<Image> pImage = hs_make_scoped<Image>(rawData, width, height, channel);

    return pImage;
}

Scoped<Mesh> ObjectManager::LoadMeshFromFile(const std::string& path, bool isAbsolutePath)
{
    const char* filePath = nullptr;
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
                         aiProcess_FlipUVs;                   // Flip UV coordinates for OpenGL

    // Note: Removed aiProcess_ConvertToLeftHanded as it might cause issues with some models
    // Add it back if needed for specific coordinate system requirements

    const aiScene* scene = importer.ReadFile(filePath, importFlags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        HS_LOG(error, "ObjectManager cannot import mesh (%s): %s", filePath, importer.GetErrorString());
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
    std::vector<Scoped<Material>> materials = ProcessMaterial(scene, modelDirectory);

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

void ObjectManager::calculatePlane()
{
    // Plane Mesh (LH)

    // 1x1 평면, 중앙이 원점, Y축이 위
    std::vector<float> positions = {
        -0.5f, 0.0f, 0.5f, 1.0f, // 0: 왼쪽 위
        0.5f, 0.0f, 0.5f, 1.0f,  // 1: 오른쪽 위
        0.5f, 0.0f, -0.5f, 1.0f, // 2: 오른쪽 아래
        -0.5f, 0.0f, -0.5f, 1.0f // 3: 왼쪽 아래
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
        0, 2, 1, // 첫 번째 삼각형 (시계방향)
        0, 3, 2  // 두 번째 삼각형 (시계방향)
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
        -0.5f, -0.5f, 0.5f, 1.0f, // 0
        0.5f, -0.5f, 0.5f, 1.0f,  // 1
        0.5f, 0.5f, 0.5f, 1.0f,   // 2
        -0.5f, 0.5f, 0.5f, 1.0f,  // 3

        // 뒷면 (Z-)
        0.5f, -0.5f, -0.5f, 1.0f,  // 4
        -0.5f, -0.5f, -0.5f, 1.0f, // 5
        -0.5f, 0.5f, -0.5f, 1.0f,  // 6
        0.5f, 0.5f, -0.5f, 1.0f,   // 7

        // 윗면 (Y+)
        -0.5f, 0.5f, 0.5f, 1.0f,  // 8
        0.5f, 0.5f, 0.5f, 1.0f,   // 9
        0.5f, 0.5f, -0.5f, 1.0f,  // 10
        -0.5f, 0.5f, -0.5f, 1.0f, // 11

        // 아랫면 (Y-)
        -0.5f, -0.5f, -0.5f, 1.0f, // 12
        0.5f, -0.5f, -0.5f, 1.0f,  // 13
        0.5f, -0.5f, 0.5f, 1.0f,   // 14
        -0.5f, -0.5f, 0.5f, 1.0f,  // 15

        // 오른쪽면 (X+)
        0.5f, -0.5f, 0.5f, 1.0f,  // 16
        0.5f, -0.5f, -0.5f, 1.0f, // 17
        0.5f, 0.5f, -0.5f, 1.0f,  // 18
        0.5f, 0.5f, 0.5f, 1.0f,   // 19

        // 왼쪽면 (X-)
        -0.5f, -0.5f, -0.5f, 1.0f, // 20
        -0.5f, -0.5f, 0.5f, 1.0f,  // 21
        -0.5f, 0.5f, 0.5f, 1.0f,   // 22
        -0.5f, 0.5f, -0.5f, 1.0f   // 23
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
        // 앞면 (시계방향)
        0, 2, 1,
        0, 3, 2,

        // 뒷면 (시계방향)
        4, 6, 5,
        4, 7, 6,

        // 윗면 (시계방향)
        8, 10, 9,
        8, 11, 10,

        // 아랫면 (시계방향)
        12, 14, 13,
        12, 15, 14,

        // 오른쪽면 (시계방향)
        16, 18, 17,
        16, 19, 18,

        // 왼쪽면 (시계방향)
        20, 22, 21,
        20, 23, 22
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
            indices.push_back(next);
            indices.push_back(current + 1);

            // 두 번째 삼각형
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
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
    return s_fallbackImage2DWhite;
}
const Image* ObjectManager::GetFallbackImage2DBlack()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback image.");
    }
    // Return the fallback black image
    return s_fallbackImage2DBlack;
}
const Image* ObjectManager::GetFallbackImage2DRed()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback image.");
    }

    return s_fallbackImage2DRed; // Return empty image or handle error appropriately
}
const Image* ObjectManager::GetFallbackImage2DGreen()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback image.");
    }
    return s_fallbackImage2DGreen; // Return empty image or handle error appropriately
}
const Image* ObjectManager::GetFallbackImage2DBlue()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback image.");
    }
    return s_fallbackImage2DBlue; // Return empty image or handle error appropriately
}

const Mesh* ObjectManager::GetFallbackMeshPlane()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback mesh.");
    }
    return s_fallbackMeshPlane; // Return empty mesh or handle error appropriately
}

const Mesh* ObjectManager::GetFallbackMeshCube()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback mesh.");
    }
    return s_fallbackMeshCube; // Return empty mesh or handle error appropriately
}

const Mesh* ObjectManager::GetFallbackMeshSphere()
{
    if (!s_isInitialize)
    {
        HS_LOG(crash, "ObjectManager is not initialized. Cannot get fallback mesh.");
    }
    return s_fallbackMeshSphere; // Return empty mesh or handle error appropriately
}

HS_NS_END
