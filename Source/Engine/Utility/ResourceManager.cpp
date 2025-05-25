#include "Engine/Utility/ResourceManager.h"

#include "Engine/Core/FileSystem.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Engine/Core/Log.h"
#include "Engine/Object/Mesh.h"

HS_NS_BEGIN

bool ResourceManager::s_isInitialize = false;

Image ResourceManager::s_fallbackImage2DWhite;
Image ResourceManager::s_fallbackImage2DBlack;
Image ResourceManager::s_fallbackImage2DRed;
Image ResourceManager::s_fallbackImage2DGreen;
Image ResourceManager::s_fallbackImage2DBlue;

Mesh ResourceManager::s_fallbackMeshPlane;
Mesh ResourceManager::s_fallbackMeshCube;
Mesh ResourceManager::s_fallbackMeshSphere;

bool ResourceManager::Initialize()
{
    // 1x1 White Image 2D
    {
        uint8 whitePixel[4]    = {255, 255, 255, 255}; // RGBA
        s_fallbackImage2DWhite = Image(whitePixel, 1, 1, 4);
        s_fallbackImage2DWhite.SetType(Image::ImageType::BUFFER);
    }

    // 1x1 Black Image 2D
    {
        uint8 blackPixel[4]    = {0, 0, 0, 255}; // RGBA
        s_fallbackImage2DBlack = Image(blackPixel, 1, 1, 4);
        s_fallbackImage2DBlack.SetType(Image::ImageType::BUFFER);
    }

    // 1x1 Red Image 2D
    {
        uint8 redPixel[4]    = {1, 0, 0, 255}; // RGBA
        s_fallbackImage2DRed = Image(redPixel, 1, 1, 4);
        s_fallbackImage2DRed.SetType(Image::ImageType::BUFFER);
    }

    // 1x1 Green Image 2D
    {
        uint8 greenPixel[4]    = {0, 1, 0, 255}; // RGBA
        s_fallbackImage2DGreen = Image(greenPixel, 1, 1, 4);
        s_fallbackImage2DGreen.SetType(Image::ImageType::BUFFER);
    }

    // 1x1 Blue Image 2D
    {
        uint8 bluePixel[4]    = {0, 0, 1, 255}; // RGBA
        s_fallbackImage2DBlue = Image(bluePixel, 1, 1, 4);
        s_fallbackImage2DBlue.SetType(Image::ImageType::BUFFER);
    }

    calculatePlane();
    calculateCube();
    calculatePlane();

    s_isInitialize = true;
    return s_isInitialize;
}

void ResourceManager::Finalize()
{
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
        filePath = hs_file_get_absolute_path(path).c_str();
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
    const aiScene* scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_ConvertToLeftHanded);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        HS_LOG(error, "ResourceManager cannot import mesh (%s)", path.c_str());
    }

    return nullptr;
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

const Image& ResourceManager::GetFallbackImage2DWhite()
{
    HS_ASSERT(s_isInitialize, "ResourceManager is not initialized");

    return s_fallbackImage2DWhite;
}

const Image& ResourceManager::GetFallbackImage2DBlack()
{
    HS_ASSERT(s_isInitialize, "ResourceManager is not initialized");

    return s_fallbackImage2DBlack;
}

const Image& ResourceManager::GetFallbackImage2DRed()
{
    HS_ASSERT(s_isInitialize, "ResourceManager is not initialized");

    return s_fallbackImage2DRed;
}

const Image& ResourceManager::GetFallbackImage2DGreen()
{
    HS_ASSERT(s_isInitialize, "ResourceManager is not initialized");

    return s_fallbackImage2DGreen;
}

const Image& ResourceManager::GetFallbackImage2DBlue()
{
    HS_ASSERT(s_isInitialize, "ResourceManager is not initialized");

    return s_fallbackImage2DBlue;
}

const Mesh& ResourceManager::GetFallbackMeshPlane()
{
    HS_ASSERT(s_isInitialize, "ResourceManager is not initialized");

    return s_fallbackMeshPlane;
}

const Mesh& ResourceManager::GetFallbackMeshCube()
{
    HS_ASSERT(s_isInitialize, "ResourceManager is not initialized");

    return s_fallbackMeshCube;
}

const Mesh& ResourceManager::GetFallbackMeshSphere()
{
    HS_ASSERT(s_isInitialize, "ResourceManager is not initialized");

    return s_fallbackMeshSphere;
}

void ResourceManager::calculatePlane()
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

    s_fallbackMeshPlane.SetPosition(std::move(positions));
    s_fallbackMeshPlane.SetNormal(std::move(normals));
    s_fallbackMeshPlane.SetTexCoord(std::move(texcoords), 0);
    s_fallbackMeshPlane.SetTangent(std::move(tangents));
    s_fallbackMeshPlane.SetBitangent(std::move(bitangents));
    s_fallbackMeshPlane.SetIndices(std::move(indices));
}
void ResourceManager::calculateCube()
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

    s_fallbackMeshCube.SetPosition(std::move(positions));
    s_fallbackMeshCube.SetColor(std::move(colors));
    s_fallbackMeshCube.SetNormal(std::move(normals));
    s_fallbackMeshCube.SetTexCoord(std::move(texcoords), 0);
    s_fallbackMeshCube.SetIndices(std::move(indices));
}
void ResourceManager::calculateSphere()
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
        float phi    = M_PI * float(ring) / float(rings); // 0 ~ PI
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        for (int segment = 0; segment <= segments; ++segment)
        {
            float theta    = 2.0f * M_PI * float(segment) / float(segments); // 0 ~ 2PI
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

    s_fallbackMeshSphere.SetPosition(std::move(positions));
    s_fallbackMeshSphere.SetNormal(std::move(normals));
    s_fallbackMeshSphere.SetTexCoord(std::move(texcoords), 0);
    s_fallbackMeshSphere.SetIndices(std::move(indices));
}

HS_NS_END
