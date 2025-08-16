//
//  Mesh.cpp
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#include "Object/Mesh.h"
#include <limits>
#include <algorithm>

#include "Core/Math/Math.h"

HS_NS_BEGIN

Mesh::~Mesh()
{
}

void Mesh::CalculateBounds()
{
	if (_position.empty())
	{
		_bound.min = { 0.0f, 0.0f, 0.0f, 1.0f };
		_bound.max = { 0.0f, 0.0f, 0.0f, 1.0f };
		return;
	}

    float minX, minY, minZ;
    float maxX, maxY, maxZ;
    minX = minY = minZ = HS_FLT_MIN;
    maxX = maxY = maxZ = HS_FLT_MAX;

    for (int i = 0; i < _position.size(); i += 4)
    {
        minX = std::min(minX, _position[i + 0]);
        minY = std::min(minX, _position[i + 1]);
        minZ = std::min(minX, _position[i + 2]);
        maxX = std::max(minX, _position[i + 0]);
        maxY = std::max(minX, _position[i + 1]);
        maxZ = std::max(minX, _position[i + 2]);
    }

    _bound.min = {minX, minY, minZ, 1.0f};
    _bound.max = {maxX, maxY, maxZ, 1.0f};
}

void Mesh::CalculateNormal()
{
    if (_position.empty() || _indices.empty())
    {
        return;
    }

    size_t vertexCount = _position.size() / 3;
    _normal.clear();
    _normal.resize(vertexCount * 3, 0.0f);

    // 면 법선을 계산하고 정점에 누적
    for (size_t i = 0; i < _indices.size(); i += 3)
    {
        uint32 i0 = _indices[i];
        uint32 i1 = _indices[i + 1];
        uint32 i2 = _indices[i + 2];

        Vec3f v0(_position[i0 * 3], _position[i0 * 3 + 1], _position[i0 * 3 + 2]);
        Vec3f v1(_position[i1 * 3], _position[i1 * 3 + 1], _position[i1 * 3 + 2]);
        Vec3f v2(_position[i2 * 3], _position[i2 * 3 + 1], _position[i2 * 3 + 2]);
        
        Vec3f faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        // 각 정점에 면 법선 누적
        _normal[i0 * 3] += faceNormal.x;
        _normal[i0 * 3 + 1] += faceNormal.y;
        _normal[i0 * 3 + 2] += faceNormal.z;

        _normal[i1 * 3] += faceNormal.x;
        _normal[i1 * 3 + 1] += faceNormal.y;
        _normal[i1 * 3 + 2] += faceNormal.z;

        _normal[i2 * 3] += faceNormal.x;
        _normal[i2 * 3 + 1] += faceNormal.y;
        _normal[i2 * 3 + 2] += faceNormal.z;
    }

    // 정점 법선 정규화
    for (size_t i = 0; i < _normal.size(); i += 3)
    {
        Vec3f normal(_normal[i], _normal[i + 1], _normal[i + 2]);
        normal         = glm::normalize(normal);
        _normal[i]     = normal.x;
        _normal[i + 1] = normal.y;
        _normal[i + 2] = normal.z;
    }
}

void Mesh::CalculateTangent()
{
    if (_position.empty() || _indices.empty() || _texcoord[0].empty())
        return;
    
    size_t vertexCount = _position.size() / 3;
    _tangent.clear();
    _tangent.resize(vertexCount * 3, 0.0f);
    _bitangent.clear();
    _bitangent.resize(vertexCount * 3, 0.0f);
    
    // 삼각형별로 탄젠트/바이탄젠트 계산
    for (size_t i = 0; i < _indices.size(); i += 3)
    {
        uint32 i0 = _indices[i];
        uint32 i1 = _indices[i + 1];
        uint32 i2 = _indices[i + 2];
        
        Vec3f v0(_position[i0 * 3], _position[i0 * 3 + 1], _position[i0 * 3 + 2]);
        Vec3f v1(_position[i1 * 3], _position[i1 * 3 + 1], _position[i1 * 3 + 2]);
        Vec3f v2(_position[i2 * 3], _position[i2 * 3 + 1], _position[i2 * 3 + 2]);
        
        Vec2f uv0(_texcoord[0][i0 * 2], _texcoord[0][i0 * 2 + 1]);
        Vec2f uv1(_texcoord[0][i1 * 2], _texcoord[0][i1 * 2 + 1]);
        Vec2f uv2(_texcoord[0][i2 * 2], _texcoord[0][i2 * 2 + 1]);
        
        Vec3f deltaPos1 = v1 - v0;
        Vec3f deltaPos2 = v2 - v0;
        
        Vec2f deltaUV1 = uv1 - uv0;
        Vec2f deltaUV2 = uv2 - uv0;
        
        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        Vec3f tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
        Vec3f bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
        
        // 각 정점에 누적
        for (int j = 0; j < 3; ++j)
        {
            uint32 idx = _indices[i + j];
            _tangent[idx * 3] += tangent.x;
            _tangent[idx * 3 + 1] += tangent.y;
            _tangent[idx * 3 + 2] += tangent.z;
            
            _bitangent[idx * 3] += bitangent.x;
            _bitangent[idx * 3 + 1] += bitangent.y;
            _bitangent[idx * 3 + 2] += bitangent.z;
        }
    }
    
    // 정규화 및 직교화
    for (size_t i = 0; i < vertexCount; ++i)
    {
        Vec3f n(_normal[i * 3], _normal[i * 3 + 1], _normal[i * 3 + 2]);
        Vec3f t(_tangent[i * 3], _tangent[i * 3 + 1], _tangent[i * 3 + 2]);
        
        // Gram-Schmidt 직교화
        t = glm::normalize(t - n * glm::dot(n, t));
        
        _tangent[i * 3] = t.x;
        _tangent[i * 3 + 1] = t.y;
        _tangent[i * 3 + 2] = t.z;
        
        // 바이탄젠트는 법선과 탄젠트의 외적으로 재계산
        Vec3f b = glm::cross(n, t);
        _bitangent[i * 3] = b.x;
        _bitangent[i * 3 + 1] = b.y;
        _bitangent[i * 3 + 2] = b.z;
    }
}

HS_NS_END
