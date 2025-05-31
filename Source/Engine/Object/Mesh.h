//
//  Mesh.h
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#ifndef __HS_MESH_H__
#define __HS_MESH_H__

#include "Precompile.h"

#include "Engine/Object/Object.h"

#include "glm/glm.hpp"

HS_NS_BEGIN

class Mesh : public Object
{
public:
    Mesh()
        : Object(EType::MESH)
    {}
    ~Mesh() override;

    HS_FORCEINLINE void AddSubMesh(Mesh* subMesh) { _subMeshes.push_back(subMesh); }

    HS_FORCEINLINE void  SetPosition(std::vector<float>&& position) { _position = std::move(position); }
    HS_FORCEINLINE void  SetPosition(const std::vector<float>& position) { _position = position; }
    HS_FORCEINLINE const std::vector<float>& GetPosition() const { return _position; }

    HS_FORCEINLINE void  SetTexCoord(std::vector<float>&& texcoord, int index) { _texcoord[index] = std::move(texcoord); }
    HS_FORCEINLINE void  SetTexCoord(const std::vector<float>& texcoord, int index) { _texcoord[index] = texcoord; }
    HS_FORCEINLINE const std::vector<float>& GetTexCoord(int index) const
    {
        HS_ASSERT(0 >= index && index <= 8, "out of range");
        return _texcoord[index];
    }

    HS_FORCEINLINE void  SetNormal(std::vector<float>&& normal) { _normal = std::move(normal); }
    HS_FORCEINLINE void  SetNormal(const std::vector<float>& normal) { _normal = normal; }
    HS_FORCEINLINE const std::vector<float>& GetNormal() const { return _normal; }

    HS_FORCEINLINE void  SetColor(std::vector<float>&& color) { _color = std::move(color); }
    HS_FORCEINLINE void  SetColor(const std::vector<float>& color) { _color = color; }
    HS_FORCEINLINE const std::vector<float>& GetColor() const { return _color; }

    HS_FORCEINLINE void  SetTangent(std::vector<float>&& tangent) { _tangent = std::move(tangent); }
    HS_FORCEINLINE void  SetTangent(const std::vector<float>& tangent) { _tangent = tangent; }
    HS_FORCEINLINE const std::vector<float>& GetTangent() const { return _tangent; }

    HS_FORCEINLINE void  SetBitangent(std::vector<float>&& bitangent) { _bitangent = std::move(bitangent); }
    HS_FORCEINLINE void  SetBitangent(const std::vector<float>& bitangent) { _bitangent = bitangent; }
    HS_FORCEINLINE const std::vector<float>& GetBitangent() const { return _bitangent; }

    HS_FORCEINLINE void  SetIndices(std::vector<uint32>&& indices) { _indices = std::move(indices); }
    HS_FORCEINLINE void  SetIndices(const std::vector<uint32>& indices) { _indices = indices; }
    HS_FORCEINLINE const std::vector<uint32>& GetIndices() const { return _indices; }

    // Utility methods
    HS_FORCEINLINE uint32 GetVertexCount() const { return static_cast<uint32>(_position.size() / 3); }
    HS_FORCEINLINE uint32 GetTriangleCount() const { return static_cast<uint32>(_indices.size() / 3); }
    HS_FORCEINLINE const std::vector<Mesh*>& GetSubMeshes() const { return _subMeshes; }
    
    // Calculate bounding box
    void CalculateBounds(glm::vec3& outMin, glm::vec3& outMax) const;
    
    // Check if mesh has specific attributes
    HS_FORCEINLINE bool HasNormals() const { return !_normal.empty(); }
    HS_FORCEINLINE bool HasTexCoords(int index = 0) const { return index >= 0 && index < 8 && !_texcoord[index].empty(); }
    HS_FORCEINLINE bool HasColors() const { return !_color.empty(); }
    HS_FORCEINLINE bool HasTangents() const { return !_tangent.empty(); }
    HS_FORCEINLINE bool HasIndices() const { return !_indices.empty(); }
    
    // Material management
    void SetMaterialIndex(int32 index) { _materialIndex = index; }
    HS_FORCEINLINE int32 GetMaterialIndex() const { return _materialIndex; }

private:
    std::vector<float> _position;
    std::vector<float> _texcoord[8];
    std::vector<float> _normal;
    std::vector<float> _color;
    std::vector<float> _tangent;
    std::vector<float> _bitangent;

    std::vector<Mesh*>  _subMeshes;
    std::vector<uint32> _indices;
    
    int32 _materialIndex = -1; // Index to material in the material array
};

HS_NS_END

#endif
