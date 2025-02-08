#include "Engine/Utility/ResourceManager.h"

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

#include "Engine/Core/Log.h"

using namespace tinygltf;

HS_NS_BEGIN

bool hs_resource_load_image(std::string& fileName, void* data, int& width, int& height, int& channel)
{
    data = stbi_load(fileName.c_str(), &width, &height, &channel, 0);

    return (data != nullptr);
}

void hs_resource_free_image(void* data)
{
    HS_ASSERT(data, "data what is requested to free is nullptr");
    
    stbi_image_free(data);
}

HS_NS_END
