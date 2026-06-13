//
//  VolumetricCloudDefinition.h
//  HSMR
//
#ifndef __HS_RENDERER_VOLUMETRIC_CLOUD_DEFINITION_H__
#define __HS_RENDERER_VOLUMETRIC_CLOUD_DEFINITION_H__

#include "Precompile.h"
#include "Core/Math/Common.h"

HS_NS_BEGIN

enum class EVolumetricCloudDebugView : uint32
{
    Final = 0,
    BaseNoise,
    DetailNoise,
    Density,
    Lighting,
};

struct HS_RENDERER_API VolumetricCloudSettings
{
    bool enabled = true;

    float coverage = 0.55f;
    float cloudType = 0.75f;
    float precipitation = 0.0f;
    float densityMultiplier = 1.15f;

    float erosion = 0.55f;
    float windSpeed = 18.0f;
    glm::vec2 windDirection = glm::normalize(glm::vec2(1.0f, 0.25f));

    glm::vec3 sunDirection = glm::normalize(glm::vec3(0.35f, 0.75f, 0.25f));
    float sunIntensity = 1.6f;
    glm::vec3 sunColor = glm::vec3(1.0f, 0.92f, 0.78f);
    float ambientIntensity = 0.38f;

    float hgG = 0.62f;
    float powderStrength = 0.72f;
    float cloudBottomMeters = 1500.0f;
    float cloudTopMeters = 4000.0f;

    uint32 primarySampleCount = 96;
    uint32 lightSampleCount = 6;
    EVolumetricCloudDebugView debugView = EVolumetricCloudDebugView::Final;
};

HS_NS_END

#endif // __HS_RENDERER_VOLUMETRIC_CLOUD_DEFINITION_H__
