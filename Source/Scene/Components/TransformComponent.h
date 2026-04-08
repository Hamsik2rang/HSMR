//
//  TransformComponent.h
//  HSMR
//

#pragma once

#include "Precompile.h"
#include "Core/TypeId.h"
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
struct HS_SCENE_API TransformComponent
{
    HS_GENERATE_TYPEID(TransformComponent)

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
    bool isDirty    = true;
    bool worldDirty = true;
    uint32 localVersion = 1;
    uint32 worldVersion = 0;

    void MarkDirty()
    {
        isDirty = true;
        worldDirty = true;
        localVersion++;
    }

    // ===== Local Transform 조작 =====

    void SetPosition(const glm::vec3& pos)
    {
        position = pos;
        MarkDirty();
    }

    void SetRotation(const glm::quat& rot)
    {
        rotation = rot;
        MarkDirty();
    }

    void SetScale(const glm::vec3& scl)
    {
        scale = scl;
        MarkDirty();
    }

    glm::vec3 GetEulerAngles() const
    {
        return glm::degrees(glm::eulerAngles(rotation));
    }

    void SetEulerAngles(const glm::vec3& eulerDegrees)
    {
        rotation = glm::quat(glm::radians(eulerDegrees));
        MarkDirty();
    }

    void Translate(const glm::vec3& delta)
    {
        position += delta;
        MarkDirty();
    }

    void Rotate(const glm::quat& deltaRot)
    {
        rotation = deltaRot * rotation;
        MarkDirty();
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

    glm::quat GetWorldRotation() const
    {
        glm::vec3 ws = GetWorldScale();
        glm::mat3 rotMat(
            glm::vec3(worldMatrix[0]) / ws.x,
            glm::vec3(worldMatrix[1]) / ws.y,
            glm::vec3(worldMatrix[2]) / ws.z
        );
        return glm::normalize(glm::quat_cast(rotMat));
    }

    glm::vec3 GetWorldEulerAngles() const
    {
        return glm::degrees(glm::eulerAngles(GetWorldRotation()));
    }

    // ===== World Transform Setters (역산하여 로컬에 반영) =====

    void SetWorldPosition(const glm::vec3& worldPos)
    {
        if (HasParent())
        {
            // TODO: 부모의 worldMatrix가 최신이어야 정확하다.
            //       TransformSystem 업데이트 후 호출을 권장.
            glm::mat4 parentInv = glm::inverse(worldMatrix * glm::inverse(GetLocalMatrix()));
            position = glm::vec3(parentInv * glm::vec4(worldPos, 1.0f));
        }
        else
        {
            position = worldPos;
        }
        MarkDirty();
    }

    void SetWorldRotation(const glm::quat& worldRot)
    {
        if (HasParent())
        {
            glm::quat parentWorldRot = getParentWorldRotation();
            rotation = glm::inverse(parentWorldRot) * worldRot;
        }
        else
        {
            rotation = worldRot;
        }
        MarkDirty();
    }

    void SetWorldEulerAngles(const glm::vec3& worldEulerDegrees)
    {
        SetWorldRotation(glm::quat(glm::radians(worldEulerDegrees)));
    }

private:
    glm::quat getParentWorldRotation() const
    {
        glm::mat4 parentWorld = worldMatrix * glm::inverse(GetLocalMatrix());
        glm::vec3 parentScale(
            glm::length(glm::vec3(parentWorld[0])),
            glm::length(glm::vec3(parentWorld[1])),
            glm::length(glm::vec3(parentWorld[2]))
        );
        glm::mat3 parentRotMat(
            glm::vec3(parentWorld[0]) / parentScale.x,
            glm::vec3(parentWorld[1]) / parentScale.y,
            glm::vec3(parentWorld[2]) / parentScale.z
        );
        return glm::normalize(glm::quat_cast(parentRotMat));
    }

public:

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
