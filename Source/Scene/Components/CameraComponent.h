//
//  CameraComponent.h
//  HSMR
//

#pragma once

#include "Precompile.h"
#include "Core/Math/Common.h"

HS_NS_BEGIN

/**
 * @brief 카메라 컴포넌트
 *
 * 뷰/프로젝션 매트릭스 계산에 필요한 카메라 파라미터를 저장합니다.
 */
struct HS_SCENE_API CameraComponent
{
    enum class EProjectionType : uint8
    {
        Perspective,
        Orthographic
    };

    EProjectionType projectionType = EProjectionType::Perspective;

    // Perspective parameters
    float fov = 60.0f;          // degrees
    float aspectRatio = 16.0f / 9.0f;

    // Orthographic parameters
    float orthoSize = 10.0f;    // half-height

    // Common parameters
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    // State
    bool isActive = false;
    bool isPrimary = false;     // 메인 카메라 여부

    // Viewport (0~1 normalized)
    glm::vec4 viewport{ 0.0f, 0.0f, 1.0f, 1.0f };  // x, y, width, height

    // ===== Matrix Calculations =====

    glm::mat4 GetProjectionMatrix() const
    {
        if (projectionType == EProjectionType::Perspective)
        {
            return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        }
        else
        {
            float halfWidth = orthoSize * aspectRatio;
            float halfHeight = orthoSize;
            return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
        }
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
