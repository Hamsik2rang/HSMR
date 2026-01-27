//
//  SceneObject.h
//  HSMR
//
//  Created for lightweight prototyping framework
//
#ifndef __HS_APPLICATION_SCENE_OBJECT_H__
#define __HS_APPLICATION_SCENE_OBJECT_H__

#include "Precompile.h"
#include "Core/Math/Common.h"
#include <string>

HS_NS_BEGIN

// Axis-Aligned Bounding Box
struct HS_APPLICATION_API AABB
{
    glm::vec3 min;
    glm::vec3 max;

    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec3& minPt, const glm::vec3& maxPt) : min(minPt), max(maxPt) {}

    glm::vec3 GetCenter() const { return (min + max) * 0.5f; }
    glm::vec3 GetExtents() const { return (max - min) * 0.5f; }
    glm::vec3 GetSize() const { return max - min; }

    bool Contains(const glm::vec3& point) const
    {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

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

    // Transform AABB by matrix
    AABB Transform(const glm::mat4& matrix) const;
};

// Transform component for scene objects
struct HS_APPLICATION_API Transform
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f); // Euler angles in radians
    glm::vec3 scale = glm::vec3(1.0f);

    glm::mat4 GetMatrix() const;
    glm::mat4 GetInverseMatrix() const;

    void SetFromMatrix(const glm::mat4& matrix);
};

// Scene object representing a renderable entity
class HS_APPLICATION_API SceneObject
{
public:
    SceneObject() = default;
    SceneObject(const std::string& name);
    ~SceneObject() = default;

    // Name
    const std::string& GetName() const { return _name; }
    void SetName(const std::string& name) { _name = name; }

    // Transform
    Transform& GetTransform() { return _transform; }
    const Transform& GetTransform() const { return _transform; }

    void SetPosition(const glm::vec3& pos)
    {
        _transform.position = pos;
        _worldMatrixDirty = true;
    }
    void SetRotation(const glm::vec3& rot)
    {
        _transform.rotation = rot;
        _worldMatrixDirty = true;
    }
    void SetScale(const glm::vec3& scale)
    {
        _transform.scale = scale;
        _worldMatrixDirty = true;
    }

    const glm::vec3& GetPosition() const { return _transform.position; }
    const glm::vec3& GetRotation() const { return _transform.rotation; }
    const glm::vec3& GetScale() const { return _transform.scale; }

    // World matrix
    const glm::mat4& GetWorldMatrix();

    // Local bounds (model space)
    const AABB& GetLocalBounds() const { return _localBounds; }
    void SetLocalBounds(const AABB& bounds)
    {
        _localBounds = bounds;
        _worldBoundsDirty = true;
    }

    // World bounds (transformed)
    const AABB& GetWorldBounds();

    // Model reference (index into Scene's model array)
    int32 GetModelIndex() const { return _modelIndex; }
    void SetModelIndex(int32 index) { _modelIndex = index; }

    // Shader reference (index into Scene's shader array)
    int32 GetShaderIndex() const { return _shaderIndex; }
    void SetShaderIndex(int32 index) { _shaderIndex = index; }

    // Material properties (can be expanded)
    const glm::vec4& GetBaseColor() const { return _baseColor; }
    void SetBaseColor(const glm::vec4& color) { _baseColor = color; }

    float GetMetallic() const { return _metallic; }
    void SetMetallic(float metallic) { _metallic = metallic; }

    float GetRoughness() const { return _roughness; }
    void SetRoughness(float roughness) { _roughness = roughness; }

    // Visibility
    bool IsVisible() const { return _visible; }
    void SetVisible(bool visible) { _visible = visible; }

private:
    std::string _name;
    Transform _transform;

    glm::mat4 _worldMatrix = glm::mat4(1.0f);
    bool _worldMatrixDirty = true;

    AABB _localBounds;
    AABB _worldBounds;
    bool _worldBoundsDirty = true;

    int32 _modelIndex = -1;
    int32 _shaderIndex = -1;

    // Basic material properties
    glm::vec4 _baseColor = glm::vec4(1.0f);
    float _metallic = 0.0f;
    float _roughness = 0.5f;

    bool _visible = true;
};

HS_NS_END

#endif // __HS_APPLICATION_SCENE_OBJECT_H__
