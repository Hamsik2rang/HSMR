//
//  CameraComponent.h
//  HSMR
//

#pragma once

#include "Precompile.h"
#include "Core/TypeId.h"
#include "Core/Math/Common.h"

HS_NS_BEGIN

struct HS_SCENE_API CameraComponent
{
    HS_GENERATE_TYPEID(CameraComponent)

    enum class EProjectionType : uint8
    {
        Perspective,
        Orthographic
    };

    EProjectionType projectionType = EProjectionType::Perspective;

    float fov = 60.0f;
    float aspectRatio = 16.0f / 9.0f;

    float orthoSize = 10.0f;

    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    bool isActive = false;
    bool isPrimary = false;
    int priority = 0;

    glm::vec4 viewport{0.0f, 0.0f, 1.0f, 1.0f};

    glm::mat4 GetProjectionMatrix() const
    {
        if (projectionType == EProjectionType::Perspective)
        {
            return glm::perspectiveLH(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        }

        float halfWidth = orthoSize * aspectRatio;
        float halfHeight = orthoSize;
        return glm::orthoLH(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
    }

    void SetAspectRatio(float width, float height)
    {
        if (height > 0.0f)
        {
            aspectRatio = width / height;
        }
    }
};

HS_NS_END
