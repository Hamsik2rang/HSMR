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

    HS_FORCEINLINE void AddSubMesh(Mesh* subMesh) { _subMeshes.push_back(hs_make_scoped<Mesh>(subMesh)); }

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

    HS_FORCEINLINE void  SetTangent(std::vector<float>&& color) { _color = std::move(color); }
    HS_FORCEINLINE void  SetTangent(const std::vector<float>& color) { _color = color; }
    HS_FORCEINLINE const std::vector<float>& GetTangent() const { return _color; }

    HS_FORCEINLINE void  SetBitangent(std::vector<float>&& color) { _color = std::move(color); }
    HS_FORCEINLINE void  SetBitangent(const std::vector<float>& color) { _color = color; }
    HS_FORCEINLINE const std::vector<float>& GetBitangent() const { return _color; }

    HS_FORCEINLINE void  SetIndices(std::vector<uint32>&& indices) { _indices = std::move(indices); }
    HS_FORCEINLINE void  SetIndices(const std::vector<uint32>& indices) { _indices = indices; }
    HS_FORCEINLINE const std::vector<uint32>& GetIndices() const { return _indices; }

private:
    std::vector<float> _position;
    std::vector<float> _texcoord[8];
    std::vector<float> _normal;
    std::vector<float> _color;
    std::vector<float> _tangent;
    std::vector<float> _bitangent;

    std::vector<Scoped<Mesh>>  _subMeshes;
    std::vector<uint32> _indices;
};

HS_NS_END

#endif
