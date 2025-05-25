#include "Engine/Utility/ResourceManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Engine/Core/Log.h"
#include "Engine/Object/Mesh.h"

HS_NS_BEGIN

bool hs_process_mesh(aiMesh* pAiMesh, const aiScene* pAiScene)
{
    Mesh* pOutMesh = new Mesh();
    if (pAiScene->mName.length > 0)
    {
        //..
    }
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
    Assimp::Importer importer;
    const aiScene*   scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_ConvertToLeftHanded);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        HS_LOG(error, "ResourceManager cannot import mesh (%s)", path.c_str());
    }

    return nullptr;
}

bool hs_resource_load_image(std::string& fileName, void* data, int& width, int& height, int& channel)
{
    data = stbi_load(fileName.c_str(), &width, &height, &channel, 0);

    return (data != nullptr);
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

HS_NS_END
