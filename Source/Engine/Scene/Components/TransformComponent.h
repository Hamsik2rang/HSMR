//
//  TransformComponent.h
//  HSMR
//

#pragma once

#include "Precompile.h"
#include "Core/Math/Common.h"
#include <entt/entt.hpp>
#include <vector>

HS_NS_BEGIN

/**
 * @brief Transform 컴포넌트
 *
 * 위치, 회전, 스케일 정보와 계층 구조를 관리합니다.
 * SceneGraph가 worldMatrix를 계산하여 캐싱합니다.
 */
struct HS_API TransformComponent
{
    // Local transform (부모 기준)
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };  // identity quaternion
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

    // Cached world transform (SceneGraph에서 계산)
    glm::mat4 worldMatrix{ 1.0f };

    // Hierarchy
    entt::entity parent{ entt::null };
    std::vector<entt::entity> children;

    // Dirty flag (변경 감지용)
    bool isDirty = true;

    // ===== Local Transform 조작 =====

    void SetPosition(const glm::vec3& pos)
    {
        position = pos;
        isDirty = true;
    }

    void SetRotation(const glm::quat& rot)
    {
        rotation = rot;
        isDirty = true;
    }

    void SetScale(const glm::vec3& scl)
    {
        scale = scl;
        isDirty = true;
    }

    void SetEulerAngles(const glm::vec3& eulerDegrees)
    {
        rotation = glm::quat(glm::radians(eulerDegrees));
        isDirty = true;
    }

    void Translate(const glm::vec3& delta)
    {
        position += delta;
        isDirty = true;
    }

    void Rotate(const glm::quat& deltaRot)
    {
        rotation = deltaRot * rotation;
        isDirty = true;
    }

    void RotateEuler(const glm::vec3& eulerDegrees)
    {
        Rotate(glm::quat(glm::radians(eulerDegrees)));
    }

    // ===== Local Matrix 계산 =====

    glm::mat4 GetLocalMatrix() const
    {
        glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotationMat = glm::mat4_cast(rotation);
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
        return translationMat * rotationMat * scaleMat;
    }

    // ===== Direction Vectors =====

    glm::vec3 GetForward() const
    {
        return glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 GetRight() const
    {
        return glm::normalize(rotation * glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 GetUp() const
    {
        return glm::normalize(rotation * glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // ===== World Position (from cached worldMatrix) =====

    glm::vec3 GetWorldPosition() const
    {
        return glm::vec3(worldMatrix[3]);
    }

    glm::vec3 GetWorldScale() const
    {
        return glm::vec3(
            glm::length(glm::vec3(worldMatrix[0])),
            glm::length(glm::vec3(worldMatrix[1])),
            glm::length(glm::vec3(worldMatrix[2]))
        );
    }

    // ===== Hierarchy Helpers =====

    bool HasParent() const
    {
        return parent != entt::null;
    }

    bool HasChildren() const
    {
        return !children.empty();
    }

    size_t GetChildCount() const
    {
        return children.size();
    }
};

HS_NS_END
