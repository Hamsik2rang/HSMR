#include "Object/MeshProxy.h"
#include "Object/Mesh.h"
#include "Core/Log.h"
#include "RHI/RHIContext.h"
#include <cstring>

HS_NS_BEGIN

MeshProxy::MeshProxy(uint64 gameObjectId)
    : ObjectProxy(EType::MESH, gameObjectId)
    , _vertexBuffer(nullptr)
    , _indexBuffer(nullptr)
    , _vertexInputLayout(nullptr)
    , _vertexCount(0)
    , _indexCount(0)
    , _vertexStride(0)
    , _materialIndex(-1)
    , _lastUpdateHash(0)
{
}

MeshProxy::~MeshProxy()
{
    ReleaseRenderResources();
}

void MeshProxy::UpdateFromGameObject(const Object* gameObject)
{
    const Mesh* mesh = static_cast<const Mesh*>(gameObject);
    if (!mesh || !mesh->IsValid())
    {
        return;
    }

    uint64 currentHash = static_cast<uint64>(mesh->GetVertexCount()) + 
                        static_cast<uint64>(mesh->GetTriangleCount()) +
                        static_cast<uint64>(mesh->GetMaterialIndex());

    if (currentHash != _lastUpdateHash || _isDirty)
    {
        CreateRHIResources(mesh);
        _lastUpdateHash = currentHash;
        MarkClean();
    }
}

void MeshProxy::ReleaseRenderResources()
{
    if (_vertexBuffer)
    {
        // TODO: Release RHI vertex buffer when RHI is implemented
        _vertexBuffer = nullptr;
    }

    if (_indexBuffer)
    {
        // TODO: Release RHI index buffer when RHI is implemented
        _indexBuffer = nullptr;
    }

    if (_vertexInputLayout)
    {
        // TODO: Release RHI vertex input layout when RHI is implemented
        _vertexInputLayout = nullptr;
    }
}

void MeshProxy::CreateRHIResources(const Mesh* mesh)
{
    if (!mesh || mesh->GetPosition().empty())
    {
        HS_LOG(warning, "MeshProxy: Cannot create RHI resources from invalid mesh");
        return;
    }

    ReleaseRenderResources();

    _vertexCount = mesh->GetVertexCount();
    _indexCount = static_cast<uint32>(mesh->GetIndices().size());
    _materialIndex = mesh->GetMaterialIndex();
    _vertexStride = CalculateVertexStride(mesh);

    // Build vertex attributes list
    _vertexAttributes.clear();
    uint32 offset = 0;

    // Position (always present)
    _vertexAttributes.push_back({
        ERHIVertexElementType::Float3,
        offset,
        sizeof(float) * 3,
        std::string("POSITION"),
        0
    });
    offset += sizeof(float) * 3;

    // Normal
    if (mesh->HasNormals())
    {
        _vertexAttributes.push_back({
            ERHIVertexElementType::Float3,
            offset,
            sizeof(float) * 3,
            std::string("NORMAL"),
            0
        });
        offset += sizeof(float) * 3;
    }

    // Texture coordinates
    for (int i = 0; i < 8; ++i)
    {
        if (mesh->HasTexCoords(i))
        {
            _vertexAttributes.push_back({
                ERHIVertexElementType::Float2,
                offset,
                sizeof(float) * 2,
                std::string("TEXCOORD"),
                static_cast<uint32>(i)
            });
            offset += sizeof(float) * 2;
        }
    }

    // Color
    if (mesh->HasColors())
    {
        _vertexAttributes.push_back({
            ERHIVertexElementType::Float4,
            offset,
            sizeof(float) * 4,
            "COLOR",
            0
        });
        offset += sizeof(float) * 4;
    }

    // Tangent
    if (mesh->HasTangents())
    {
        _vertexAttributes.push_back({
            ERHIVertexElementType::Float3,
            offset,
            sizeof(float) * 3,
            std::string("TANGENT"),
            0
        });
        offset += sizeof(float) * 3;

        // Bitangent
        _vertexAttributes.push_back({
            ERHIVertexElementType::Float3,
            offset,
            sizeof(float) * 3,
            std::string("BITANGENT"),
            0
        });
        offset += sizeof(float) * 3;
    }

    // Pack vertex data
    std::vector<float> packedVertexData = PackVertexData(mesh);

    // TODO: Create RHI vertex buffer when RHI system is available
    // RHIBufferDesc vertexBufferDesc;
    // vertexBufferDesc.usage = ERHIBufferUsage::VertexBuffer;
    // vertexBufferDesc.size = packedVertexData.size() * sizeof(float);
    // vertexBufferDesc.initialData = packedVertexData.data();
    // 
    // _vertexBuffer = RHIContext::CreateBuffer(vertexBufferDesc);

    // Create index buffer if indices exist
    if (mesh->HasIndices())
    {
        // TODO: Create RHI index buffer when RHI system is available
        // RHIBufferDesc indexBufferDesc;
        // indexBufferDesc.usage = ERHIBufferUsage::IndexBuffer;
        // indexBufferDesc.size = mesh->GetIndices().size() * sizeof(uint32);
        // indexBufferDesc.initialData = mesh->GetIndices().data();
        // 
        // _indexBuffer = RHIContext::CreateBuffer(indexBufferDesc);
    }

    CreateVertexInputLayout();

    HS_LOG(info, "MeshProxy: Created RHI resources for mesh with {} vertices, {} indices", 
           _vertexCount, _indexCount);
}

void MeshProxy::UpdateVertexData(const Mesh* mesh)
{
    if (!_vertexBuffer || !mesh)
    {
        return;
    }

    std::vector<float> packedVertexData = PackVertexData(mesh);

    // TODO: Update vertex buffer data when RHI system is available
    // RHIContext::UpdateBufferData(_vertexBuffer, packedVertexData.data(), packedVertexData.size() * sizeof(float));
}

void MeshProxy::CreateVertexInputLayout()
{
    if (_vertexAttributes.empty())
    {
        return;
    }

    // TODO: Create vertex input layout when RHI system is available
    // RHIVertexInputLayoutDesc layoutDesc;
    // layoutDesc.elements = _vertexAttributes.data();
    // layoutDesc.elementCount = static_cast<uint32>(_vertexAttributes.size());
    // layoutDesc.stride = _vertexStride;
    // 
    // _vertexInputLayout = RHIContext::CreateVertexInputLayout(layoutDesc);
}

uint32 MeshProxy::CalculateVertexStride(const Mesh* mesh)
{
    uint32 stride = sizeof(float) * 3; // Position

    if (mesh->HasNormals())
        stride += sizeof(float) * 3;

    for (int i = 0; i < 8; ++i)
    {
        if (mesh->HasTexCoords(i))
            stride += sizeof(float) * 2;
    }

    if (mesh->HasColors())
        stride += sizeof(float) * 4;

    if (mesh->HasTangents())
        stride += sizeof(float) * 6; // Tangent + Bitangent

    return stride;
}

std::vector<float> MeshProxy::PackVertexData(const Mesh* mesh)
{
    const uint32 vertexCount = mesh->GetVertexCount();
    const uint32 elementsPerVertex = _vertexStride / sizeof(float);
    
    std::vector<float> packedData(vertexCount * elementsPerVertex);

    const auto& positions = mesh->GetPosition();
    const auto& normals = mesh->GetNormal();
    const auto& colors = mesh->GetColor();
    const auto& tangents = mesh->GetTangent();
    const auto& bitangents = mesh->GetBitangent();

    for (uint32 v = 0; v < vertexCount; ++v)
    {
        uint32 index = 0;

        // Position
        packedData[v * elementsPerVertex + index++] = positions[v * 3 + 0];
        packedData[v * elementsPerVertex + index++] = positions[v * 3 + 1];
        packedData[v * elementsPerVertex + index++] = positions[v * 3 + 2];

        // Normal
        if (mesh->HasNormals())
        {
            packedData[v * elementsPerVertex + index++] = normals[v * 3 + 0];
            packedData[v * elementsPerVertex + index++] = normals[v * 3 + 1];
            packedData[v * elementsPerVertex + index++] = normals[v * 3 + 2];
        }

        // Texture coordinates
        for (int i = 0; i < 8; ++i)
        {
            if (mesh->HasTexCoords(i))
            {
                const auto& texCoords = mesh->GetTexCoord(i);
                packedData[v * elementsPerVertex + index++] = texCoords[v * 2 + 0];
                packedData[v * elementsPerVertex + index++] = texCoords[v * 2 + 1];
            }
        }

        // Color
        if (mesh->HasColors())
        {
            if (colors.size() >= (v + 1) * 4)
            {
                packedData[v * elementsPerVertex + index++] = colors[v * 4 + 0];
                packedData[v * elementsPerVertex + index++] = colors[v * 4 + 1];
                packedData[v * elementsPerVertex + index++] = colors[v * 4 + 2];
                packedData[v * elementsPerVertex + index++] = colors[v * 4 + 3];
            }
            else
            {
                // Default white color
                packedData[v * elementsPerVertex + index++] = 1.0f;
                packedData[v * elementsPerVertex + index++] = 1.0f;
                packedData[v * elementsPerVertex + index++] = 1.0f;
                packedData[v * elementsPerVertex + index++] = 1.0f;
            }
        }

        // Tangent and Bitangent
        if (mesh->HasTangents())
        {
            if (tangents.size() >= (v + 1) * 3)
            {
                packedData[v * elementsPerVertex + index++] = tangents[v * 3 + 0];
                packedData[v * elementsPerVertex + index++] = tangents[v * 3 + 1];
                packedData[v * elementsPerVertex + index++] = tangents[v * 3 + 2];
            }
            else
            {
                packedData[v * elementsPerVertex + index++] = 1.0f;
                packedData[v * elementsPerVertex + index++] = 0.0f;
                packedData[v * elementsPerVertex + index++] = 0.0f;
            }

            if (bitangents.size() >= (v + 1) * 3)
            {
                packedData[v * elementsPerVertex + index++] = bitangents[v * 3 + 0];
                packedData[v * elementsPerVertex + index++] = bitangents[v * 3 + 1];
                packedData[v * elementsPerVertex + index++] = bitangents[v * 3 + 2];
            }
            else
            {
                packedData[v * elementsPerVertex + index++] = 0.0f;
                packedData[v * elementsPerVertex + index++] = 1.0f;
                packedData[v * elementsPerVertex + index++] = 0.0f;
            }
        }
    }

    return packedData;
}

HS_NS_END