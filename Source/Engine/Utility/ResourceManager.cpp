#include "Engine/Utility/ResourceManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#define TINYGLTF_IMPLEMENTATION
//#define TINYGLTF_NO_STB_IMAGE_WRITE // 필요한 경우
//#define TINYGLTF_NO_STB_IMAGE       // 중요! - tinygltf가 내부 stb_image를 사용하지 않도록 설정
//#include "tiny_gltf.h"
#include ""

#include "Engine/Core/Log.h"

HS_NS_BEGIN

Image* ResourceManager::LoadImageFromFile(const std::string& path, bool isAbsolutePath)
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

    return nullptr;
}

Mesh* ResourceManager::LoadMeshFromFile(const std::string& path, bool isAbsolutePath)
{
    
    
    
    return nullptr;
}

bool hs_resource_load_image(std::string& fileName, void* data, int& width, int& height, int& channel)
{
    data = stbi_load(fileName.c_str(), &width, &height, &channel, 0);

    return (data != nullptr);
}

void ResourceManager::FreeImage(Image* image)
{
    uint8* data = image->GetData();
    if (nullptr == data)
    {
        return;
    }

    stbi_image_free(data);
}

HS_NS_END
