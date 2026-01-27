//
//  SceneObject.cpp
//  HSMR
//
//  Created for lightweight prototyping framework
//
#include "SceneObject.h"

HS_NS_BEGIN

// AABB Transform implementation
AABB AABB::Transform(const glm::mat4& matrix) const
{
    // Transform all 8 corners and find new AABB
    glm::vec3 corners[8] = {
        glm::vec3(min.x, min.y, min.z),
        glm::vec3(max.x, min.y, min.z),
        glm::vec3(min.x, max.y, min.z),
        glm::vec3(max.x, max.y, min.z),
        glm::vec3(min.x, min.y, max.z),
        glm::vec3(max.x, min.y, max.z),
        glm::vec3(min.x, max.y, max.z),
        glm::vec3(max.x, max.y, max.z)
    };

    AABB result;
    result.min = glm::vec3(FLT_MAX);
    result.max = glm::vec3(-FLT_MAX);

    for (int i = 0; i < 8; ++i)
    {
        glm::vec4 transformed = matrix * glm::vec4(corners[i], 1.0f);
        glm::vec3 pt = glm::vec3(transformed);
        result.min = glm::min(result.min, pt);
        result.max = glm::max(result.max, pt);
    }

    return result;
}

// Transform implementation
glm::mat4 Transform::GetMatrix() const
{
    glm::mat4 matrix = glm::mat4(1.0f);
    matrix = glm::translate(matrix, position);
    matrix = glm::rotate(matrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw
    matrix = glm::rotate(matrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch
    matrix = glm::rotate(matrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Roll
    matrix = glm::scale(matrix, scale);
    return matrix;
}

glm::mat4 Transform::GetInverseMatrix() const
{
    return glm::inverse(GetMatrix());
}

void Transform::SetFromMatrix(const glm::mat4& matrix)
{
    // Extract translation
    position = glm::vec3(matrix[3]);

    // Extract scale
    scale.x = glm::length(glm::vec3(matrix[0]));
    scale.y = glm::length(glm::vec3(matrix[1]));
    scale.z = glm::length(glm::vec3(matrix[2]));

    // Extract rotation (remove scale first)
    glm::mat3 rotMat(
        glm::vec3(matrix[0]) / scale.x,
        glm::vec3(matrix[1]) / scale.y,
        glm::vec3(matrix[2]) / scale.z
    );

    // Extract Euler angles from rotation matrix
    // Using ZYX rotation order
    rotation.y = atan2(rotMat[0][2], rotMat[2][2]);
    rotation.x = atan2(-rotMat[1][2], sqrt(rotMat[0][2] * rotMat[0][2] + rotMat[2][2] * rotMat[2][2]));
    rotation.z = atan2(rotMat[1][0], rotMat[1][1]);
}

// SceneObject implementation
SceneObject::SceneObject(const std::string& name)
    : _name(name)
{
}

const glm::mat4& SceneObject::GetWorldMatrix()
{
    if (_worldMatrixDirty)
    {
        _worldMatrix = _transform.GetMatrix();
        _worldMatrixDirty = false;
        _worldBoundsDirty = true; // World bounds need update too
    }
    return _worldMatrix;
}

const AABB& SceneObject::GetWorldBounds()
{
    if (_worldBoundsDirty)
    {
        _worldBounds = _localBounds.Transform(GetWorldMatrix());
        _worldBoundsDirty = false;
    }
    return _worldBounds;
}

HS_NS_END
