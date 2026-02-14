#include "Resource/ModelLoader.h"
#include "Resource/ObjectManager.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"
#include "Resource/Image.h"
#include "Core/SystemContext.h"
#include "Core/HAL/FileSystem.h"
#include <functional>

HS_NS_BEGIN

bool ModelLoader::LoadModel(const std::string& filePath, 
                           std::vector<Scoped<Mesh>>& outMeshes,
                           std::vector<Scoped<Material>>& outMaterials,
                           const ModelLoadSettings& settings)
{
    Assimp::Importer importer;
    
    uint32 flags = settings.assimpFlags;
    if (settings.generateNormals) flags |= aiProcess_CalcTangentSpace;
    if (settings.flipUVs) flags |= aiProcess_FlipUVs;
    
    const aiScene* scene = importer.ReadFile(filePath, flags);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        return false;
    }
    
    outMeshes.clear();
    outMaterials.clear();
    
    ProcessNode(scene->mRootNode, scene, outMeshes, outMaterials, settings);
    
    return true;
}

void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene,
                             std::vector<Scoped<Mesh>>& outMeshes,
                             std::vector<Scoped<Material>>& outMaterials,
                             const ModelLoadSettings& settings)
{
    for (uint32 i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Scoped<Mesh> processedMesh = ProcessMesh(mesh, scene, outMaterials, settings);
        if (processedMesh)
        {
            outMeshes.push_back(std::move(processedMesh));
        }
    }
    
    for (uint32 i = 0; i < node->mNumChildren; ++i)
    {
        ProcessNode(node->mChildren[i], scene, outMeshes, outMaterials, settings);
    }
}

Scoped<Mesh> ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene,
                                      std::vector<Scoped<Material>>& outMaterials,
                                      const ModelLoadSettings& settings)
{
    Scoped<Mesh> resultMesh = MakeScoped<Mesh>();
    
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texCoords[8];
    std::vector<float> tangents;
    std::vector<float> bitangents;
    std::vector<uint32> indices;
    
    positions.reserve(mesh->mNumVertices * 3);
    if (mesh->HasNormals()) normals.reserve(mesh->mNumVertices * 3);
    if (mesh->HasTangentsAndBitangents()) 
    {
        tangents.reserve(mesh->mNumVertices * 3);
        bitangents.reserve(mesh->mNumVertices * 3);
    }
    
    for (uint32 i = 0; i < 8; ++i)
    {
        if (mesh->HasTextureCoords(i))
        {
            texCoords[i].reserve(mesh->mNumVertices * 2);
        }
    }
    
    for (uint32 i = 0; i < mesh->mNumVertices; ++i)
    {
        aiVector3D pos = mesh->mVertices[i];
        positions.push_back(pos.x * settings.scale);
        positions.push_back(pos.y * settings.scale);
        positions.push_back(pos.z * settings.scale);
        
        if (mesh->HasNormals())
        {
            aiVector3D normal = mesh->mNormals[i];
            normals.push_back(normal.x);
            normals.push_back(normal.y);
            normals.push_back(normal.z);
        }
        
        if (mesh->HasTangentsAndBitangents())
        {
            aiVector3D tangent = mesh->mTangents[i];
            tangents.push_back(tangent.x);
            tangents.push_back(tangent.y);
            tangents.push_back(tangent.z);
            
            aiVector3D bitangent = mesh->mBitangents[i];
            bitangents.push_back(bitangent.x);
            bitangents.push_back(bitangent.y);
            bitangents.push_back(bitangent.z);
        }
        
        for (uint32 j = 0; j < 8; ++j)
        {
            if (mesh->HasTextureCoords(j))
            {
                aiVector3D uv = mesh->mTextureCoords[j][i];
                float u = uv.x;
                float v = settings.flipUVs ? 1.0f - uv.y : uv.y;
                texCoords[j].push_back(u);
                texCoords[j].push_back(v);
            }
        }
    }
    
    if (mesh->HasFaces())
    {
        indices.reserve(mesh->mNumFaces * 3);
        for (uint32 i = 0; i < mesh->mNumFaces; ++i)
        {
            aiFace& face = mesh->mFaces[i];
            for (uint32 j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            }
        }
    }
    
    resultMesh->SetPosition(std::move(positions));
    if (!normals.empty()) resultMesh->SetNormal(std::move(normals));
    if (!tangents.empty()) resultMesh->SetTangent(std::move(tangents));
    if (!bitangents.empty()) resultMesh->SetBitangent(std::move(bitangents));
    if (!indices.empty()) resultMesh->SetIndices(std::move(indices));
    
    for (uint32 i = 0; i < 8; ++i)
    {
        if (!texCoords[i].empty())
        {
            resultMesh->SetTexCoord(std::move(texCoords[i]), i);
        }
    }
    
    if (mesh->mMaterialIndex < scene->mNumMaterials)
    {
        resultMesh->SetMaterialIndex(static_cast<int32>(mesh->mMaterialIndex));
    }
    
    if (settings.generateNormals && !resultMesh->HasNormals())
    {
        resultMesh->CalculateNormal();
    }
    
    if (settings.generateTangents && !resultMesh->HasTangents())
    {
        resultMesh->CalculateTangent();
    }
    
    resultMesh->CalculateBounds();
    
    return resultMesh;
}

Scoped<Material> ModelLoader::ProcessMaterial(aiMaterial* material, const aiScene* scene,
                                             const std::string& modelPath,
                                             const ModelLoadSettings& settings)
{
    Scoped<Material> resultMaterial = MakeScoped<Material>();
    
    aiColor4D diffuseColor;
    if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor))
    {
        resultMaterial->SetDiffuseColor(ConvertColor4(diffuseColor));
    }
    
    aiColor4D specularColor;
    if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specularColor))
    {
        resultMaterial->SetSpecularColor(ConvertColor4(specularColor));
    }
    
    aiColor4D ambientColor;
    if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambientColor))
    {
        resultMaterial->SetAmbientColor(ConvertColor4(ambientColor));
    }
    
    aiColor4D emissionColor;
    if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emissionColor))
    {
        resultMaterial->SetEmissionColor(ConvertColor4(emissionColor));
    }
    
    float shininess = 0.0f;
    if (AI_SUCCESS == aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess))
    {
        resultMaterial->SetShininess(shininess);
    }
    
    float opacity = 1.0f;
    if (AI_SUCCESS == aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity))
    {
        resultMaterial->SetOpacity(opacity);
    }
    
    aiString diffuseTexturePath;
    if (AI_SUCCESS == aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &diffuseTexturePath))
    {
        std::string fullPath = GetTexturePath(modelPath, diffuseTexturePath.C_Str());
        Scoped<Image> diffuseTexture = LoadTexture(fullPath);
        if (diffuseTexture)
        {
            resultMaterial->SetTexture(EMaterialTextureType::Diffuse, diffuseTexture.get());
        }
    }
    
    aiString normalTexturePath;
    if (AI_SUCCESS == aiGetMaterialTexture(material, aiTextureType_NORMALS, 0, &normalTexturePath))
    {
        std::string fullPath = GetTexturePath(modelPath, normalTexturePath.C_Str());
        Scoped<Image> normalTexture = LoadTexture(fullPath);
        if (normalTexture)
        {
            resultMaterial->SetTexture(EMaterialTextureType::Normal, normalTexture.get());
        }
    }
    
    aiString specularTexturePath;
    if (AI_SUCCESS == aiGetMaterialTexture(material, aiTextureType_SPECULAR, 0, &specularTexturePath))
    {
        std::string fullPath = GetTexturePath(modelPath, specularTexturePath.C_Str());
        Scoped<Image> specularTexture = LoadTexture(fullPath);
        if (specularTexture)
        {
            resultMaterial->SetTexture(EMaterialTextureType::Specular, specularTexture.get());
        }
    }
    
    return resultMaterial;
}

Scoped<Image> ModelLoader::LoadTexture(const std::string& texturePath, bool isAbsolutePath)
{
    return ObjectManager::LoadImageFromFile(texturePath, isAbsolutePath);
}

glm::mat4 ModelLoader::ConvertMatrix(const aiMatrix4x4& matrix)
{
    return glm::mat4(
        matrix.a1, matrix.b1, matrix.c1, matrix.d1,
        matrix.a2, matrix.b2, matrix.c2, matrix.d2,
        matrix.a3, matrix.b3, matrix.c3, matrix.d3,
        matrix.a4, matrix.b4, matrix.c4, matrix.d4
    );
}

glm::vec3 ModelLoader::ConvertVector3(const aiVector3D& vector)
{
    return glm::vec3(vector.x, vector.y, vector.z);
}

glm::vec2 ModelLoader::ConvertVector2(const aiVector2D& vector)
{
    return glm::vec2(vector.x, vector.y);
}

glm::vec4 ModelLoader::ConvertColor4(const aiColor4D& color)
{
    return glm::vec4(color.r, color.g, color.b, color.a);
}

std::string ModelLoader::GetTexturePath(const std::string& modelPath, const std::string& texturePath)
{
    if (texturePath.empty()) return "";
    
    if (FileSystem::IsAbsolutePath(texturePath))
    {
        return texturePath;
    }
    
    size_t lastSlash = modelPath.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        std::string modelDir = modelPath.substr(0, lastSlash);
        return modelDir + "/" + texturePath;
    }
    
    return texturePath;
}

std::string ModelLoader::GetFileExtension(const std::string& filePath)
{
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos != std::string::npos && dotPos < filePath.length() - 1)
    {
        return filePath.substr(dotPos + 1);
    }
    return "";
}

HS_NS_END