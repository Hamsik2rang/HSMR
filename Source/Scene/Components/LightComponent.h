//
//  LightComponent.h
//  HSMR
//

#pragma once

#include "Precompile.h"
#include "Core/Math/Common.h"

HS_NS_BEGIN

/**
 * @brief 라이트 타입
 */
enum class ELightType : uint8
{
    Directional,    // 방향광 (태양)
    Point,          // 점광원
    Spot            // 스포트라이트
};

/**
 * @brief 라이트 컴포넌트
 *
 * 씬 조명 정보를 저장합니다.
 */
struct HS_SCENE_API LightComponent
{
    ELightType type = ELightType::Directional;

    glm::vec3 color{ 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;

    // Point/Spot light parameters
    float range = 10.0f;
    float attenuation = 1.0f;

    // Spot light parameters
    float innerConeAngle = 30.0f;   // degrees
    float outerConeAngle = 45.0f;   // degrees

    // Shadow parameters
    bool castShadow = true;
    float shadowBias = 0.005f;
    uint32 shadowMapResolution = 1024;

    // State
    bool isEnabled = true;

    LightComponent() = default;
    LightComponent(ELightType lightType)
        : type(lightType)
    {}

    // ===== Helpers =====

    float GetCosInnerConeAngle() const
    {
        return glm::cos(glm::radians(innerConeAngle));
    }

    float GetCosOuterConeAngle() const
    {
        return glm::cos(glm::radians(outerConeAngle));
    }
};

HS_NS_END
