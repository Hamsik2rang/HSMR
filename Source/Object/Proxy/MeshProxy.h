#ifndef __HS_MESH_PROXY_H__
#define __HS_MESH_PROXY_H__

#include "Precompile.h"
#include "Object/ObjectProxy.h"
#include "RHI/RHIDefinition.h"
#include <vector>

HS_NS_BEGIN

class Mesh;
class RHIBuffer;
class RHIVertexInputLayout;


class HS_OBJECT_API MeshProxy : public ObjectProxy
{
public:
    explicit MeshProxy(uint64 gameObjectId);
    ~MeshProxy() override;

    void UpdateFromGameObject(const Object* gameObject) override;
    void ReleaseRenderResources() override;

    RHIBuffer* GetVertexBuffer() const { return _vertexBuffer; }
    RHIBuffer* GetIndexBuffer() const { return _indexBuffer; }
    RHIVertexInputLayout* GetVertexInputLayout() const { return _vertexInputLayout; }

    uint32 GetVertexCount() const { return _vertexCount; }
    uint32 GetIndexCount() const { return _indexCount; }
    uint32 GetTriangleCount() const { return _indexCount / 3; }
    uint32 GetVertexStride() const { return _vertexStride; }

    const std::vector<VertexAttribute>& GetVertexAttributes() const { return _vertexAttributes; }
    
    bool HasValidBuffers() const { return _vertexBuffer != nullptr; }
    bool HasIndexBuffer() const { return _indexBuffer != nullptr; }

    int32 GetMaterialIndex() const { return _materialIndex; }

private:
    void CreateRHIResources(const Mesh* mesh);
    void UpdateVertexData(const Mesh* mesh);
    void CreateVertexInputLayout();
    uint32 CalculateVertexStride(const Mesh* mesh);
    std::vector<float> PackVertexData(const Mesh* mesh);

    RHIBuffer* _vertexBuffer;
    RHIBuffer* _indexBuffer;
    RHIVertexInputLayout* _vertexInputLayout;
    
    std::vector<VertexAttribute> _vertexAttributes;
    
    uint32 _vertexCount;
    uint32 _indexCount;
    uint32 _vertexStride;
    int32 _materialIndex;
    
    uint64 _lastUpdateHash;
};

HS_NS_END

#endif