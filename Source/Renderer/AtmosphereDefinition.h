//
//  AtmosphereDefinition.h
//  HSMR
//
#ifndef __HS_RENDERER_ATMOSPHERE_DEFINITION_H__
#define __HS_RENDERER_ATMOSPHERE_DEFINITION_H__

#include "Precompile.h"
#include "Core/Math/Common.h"

HS_NS_BEGIN

enum class EAtmosphereDebugView : uint32
{
    Final = 0,
    Transmittance,
    Irradiance,
    Scattering,
};

struct HS_RENDERER_API AtmosphereSettings
{
    bool enabled = true;

    glm::vec3 sunDirection = glm::normalize(glm::vec3(0.35f, 0.75f, 0.25f));
    float sunIntensity = 18.0f;

    float exposure = 1.0f;
    float rayleighMultiplier = 1.0f;
    float mieMultiplier = 1.0f;
    float ozoneMultiplier = 1.0f;
    float mieG = 0.8f;

    float planetRadiusMeters = 6360000.0f;
    float atmosphereRadiusMeters = 6460000.0f;
    float rayleighScaleHeightMeters = 8000.0f;
    float mieScaleHeightMeters = 1200.0f;

    uint32 multipleScatteringOrder = 4;
    EAtmosphereDebugView debugView = EAtmosphereDebugView::Final;
};

HS_NS_END

#endif // __HS_RENDERER_ATMOSPHERE_DEFINITION_H__
