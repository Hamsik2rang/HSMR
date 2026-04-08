//
//  MeshRendererComponent.h
//  HSMR
//
//  Unified mesh rendering component (Mesh + Materials)
//

#pragma once

#include "Precompile.h"
#include "Core/TypeId.h"
#include "Core/Math/Common.h"

#include <vector>

HS_NS_BEGIN

class Mesh;
class Material;

/**
 * @brief AABB (Axis-Aligned Bounding Box)
 */
struct HS_SCENE_API AABB
{
    glm::vec3 min{ 0.0f };
    glm::vec3 max{ 0.0f };

    AABB() = default;
    AABB(const glm::vec3& minPt, const glm::vec3& maxPt)
        : min(minPt), max(maxPt) {}

    glm::vec3 GetCenter() const { return (min + max) * 0.5f; }
    glm::vec3 GetExtents() const { return (max - min) * 0.5f; }
    glm::vec3 GetSize() const { return max - min; }

    bool IsValid() const { return min.x <= max.x && min.y <= max.y && min.z <= max.z; }

    void Expand(const glm::vec3& point)
    {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    void Expand(const AABB& other)
    {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }

    // Transform AABB by matrix (conservative expansion)
    AABB Transform(const glm::mat4& matrix) const
    {
        glm::vec3 corners[8] = {
            { min.x, min.y, min.z },
            { max.x, min.y, min.z },
            { min.x, max.y, min.z },
            { max.x, max.y, min.z },
            { min.x, min.y, max.z },
            { max.x, min.y, max.z },
            { min.x, max.y, max.z },
            { max.x, max.y, max.z }
        };

        AABB result;
        result.min = glm::vec3(FLT_MAX);
        result.max = glm::vec3(-FLT_MAX);

        for (int i = 0; i < 8; ++i)
        {
            glm::vec4 transformed = matrix * glm::vec4(corners[i], 1.0f);
            glm::vec3 pt = glm::vec3(transformed) / transformed.w;
            result.Expand(pt);
        }

        return result;
    }

    /**
     * @brief Ray-AABB intersection test (slab method)
     * @param rayOrigin Origin of the ray
     * @param rayDir Direction of the ray (normalized)
     * @param outT Distance along ray to intersection point (if hit)
     * @return true if ray intersects AABB
     */
    bool Intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float& outT) const
    {
        if (!IsValid())
            return false;

        float tMin = 0.0f;
        float tMax = FLT_MAX;

        for (int i = 0; i < 3; ++i)
        {
            if (glm::abs(rayDir[i]) < 1e-6f)
            {
                // Ray is parallel to slab
                if (rayOrigin[i] < min[i] || rayOrigin[i] > max[i])
                    return false;
            }
            else
            {
                float invD = 1.0f / rayDir[i];
                float t1 = (min[i] - rayOrigin[i]) * invD;
                float t2 = (max[i] - rayOrigin[i]) * invD;

                if (t1 > t2)
                    std::swap(t1, t2);

                tMin = glm::max(tMin, t1);
                tMax = glm::min(tMax, t2);

                if (tMin > tMax)
                    return false;
            }
        }

        outT = tMin;
        return tMin >= 0.0f;
    }
};

/**
 * @brief Mesh Renderer Component
 *
 * Combines Mesh and Materials for rendering.
 * Each submesh can have its own material.
 */
struct HS_SCENE_API MeshRendererComponent
{
    HS_GENERATE_TYPEID(MeshRendererComponent)

    // Mesh reference
    Mesh* mesh = nullptr;

    // Materials array (one per submesh, or shared)
    std::vector<Material*> materials;

    // Rendering flags
    bool castShadow = true;
    bool receiveShadow = true;
    bool isVisible = true;

    // Bounds for culling and picking
    AABB localBounds;
    AABB worldBounds;  // Updated by SceneGraph or manually
    uint32 boundsWorldVersion = 0;
    bool boundsDirty = true;

    // Render layer mask for selective rendering
    uint32 renderLayerMask = 0xFFFFFFFF;

    // ===== Constructors =====

    MeshRendererComponent() = default;

    MeshRendererComponent(Mesh* m, Material* mat = nullptr)
        : mesh(m)
    {
        if (mat)
        {
            materials.push_back(mat);
        }
    }

    // ===== Material Access =====

    /**
     * @brief Get material for a submesh index
     * @param submeshIndex Index of the submesh
     * @return Material pointer, or nullptr if not set
     */
    Material* GetMaterial(uint32 submeshIndex = 0) const
    {
        if (materials.empty())
            return nullptr;

        // If only one material, use it for all submeshes
        if (materials.size() == 1)
            return materials[0];

        // Otherwise, index into array (with bounds check)
        if (submeshIndex < materials.size())
            return materials[submeshIndex];

        return nullptr;
    }

    /**
     * @brief Set material for a submesh index
     * @param mat Material to set
     * @param submeshIndex Index of the submesh
     */
    void SetMaterial(Material* mat, uint32 submeshIndex = 0)
    {
        // Expand array if needed
        if (submeshIndex >= materials.size())
        {
            materials.resize(submeshIndex + 1, nullptr);
        }
        materials[submeshIndex] = mat;
    }

    /**
     * @brief Set the same material for all submeshes
     */
    void SetSharedMaterial(Material* mat)
    {
        materials.clear();
        if (mat)
        {
            materials.push_back(mat);
        }
    }

    /**
     * @brief Get number of materials
     */
    uint32 GetMaterialCount() const
    {
        return static_cast<uint32>(materials.size());
    }

    // ===== Bounds =====

    /**
     * @brief Update world bounds from local bounds and transform matrix
     */
    void UpdateWorldBounds(const glm::mat4& worldMatrix)
    {
        if (localBounds.IsValid())
        {
            worldBounds = localBounds.Transform(worldMatrix);
        }
        boundsDirty = false;
    }

    /**
     * @brief Check if this renderer is valid for rendering
     */
    bool IsValidForRendering() const
    {
        return mesh != nullptr && isVisible && !materials.empty();
    }
};

HS_NS_END
