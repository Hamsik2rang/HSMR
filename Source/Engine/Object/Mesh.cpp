//
//  Mesh.cpp
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#include "Engine/Object/Mesh.h"
#include <limits>
#include <algorithm>

HS_NS_BEGIN

Mesh::~Mesh()
{
    // Clean up submeshes
    for (Mesh* subMesh : _subMeshes)
    {
        delete subMesh;
    }
    _subMeshes.clear();
}

void Mesh::CalculateBounds(glm::vec3& outMin, glm::vec3& outMax) const
{
    if (_position.empty())
    {
        outMin = glm::vec3(0.0f);
        outMax = glm::vec3(0.0f);
        return;
    }
    
    // Initialize with extreme values
    outMin = glm::vec3(std::numeric_limits<float>::max());
    outMax = glm::vec3(std::numeric_limits<float>::lowest());
    
    // Process vertices (assuming xyz format)
    for (size_t i = 0; i < _position.size(); i += 3)
    {
        float x = _position[i];
        float y = _position[i + 1];
        float z = _position[i + 2];
        
        outMin.x = std::min(outMin.x, x);
        outMin.y = std::min(outMin.y, y);
        outMin.z = std::min(outMin.z, z);
        
        outMax.x = std::max(outMax.x, x);
        outMax.y = std::max(outMax.y, y);
        outMax.z = std::max(outMax.z, z);
    }
    
    // Also consider submeshes
    for (const Mesh* subMesh : _subMeshes)
    {
        glm::vec3 subMin, subMax;
        subMesh->CalculateBounds(subMin, subMax);
        
        outMin = glm::min(outMin, subMin);
        outMax = glm::max(outMax, subMax);
    }
}

HS_NS_END
