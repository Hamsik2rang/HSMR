//
//  RenderDefinition.h
//  HSMR
//
//  Rendering UBO definitions (moved from ResourceDefinition.h)
//
#ifndef __HS_RENDER_DEFINITION_H__
#define __HS_RENDER_DEFINITION_H__

#include "Precompile.h"
#include "Core/Math/Common.h"
#include "RHI/RHIDefinition.h"

HS_NS_BEGIN

#pragma region Shader Uniform Layouts

#define HS_SHADER_ALIGNED alignas(16)

// TODO: 리플렉션으로 자동 구성하는게 이상적임
struct HS_SHADER_ALIGNED PerDraw
{
    HS_SHADER_ALIGNED glm::mat4x4 modelMatrix;
    HS_SHADER_ALIGNED glm::mat4x4 inverseModelMatrix;
};

// All trailing scalars/vectors are packed into vec4 slots so the C++ layout
// matches Vulkan std140, Metal MSL, and HLSL cbuffer rules byte-for-byte.
struct HS_SHADER_ALIGNED PerView
{
    HS_SHADER_ALIGNED glm::mat4x4 viewMatrix;
    HS_SHADER_ALIGNED glm::mat4x4 projectionMatrix;
    HS_SHADER_ALIGNED glm::mat4x4 viewProjectionMatrix;
    HS_SHADER_ALIGNED glm::mat4x4 inverseViewMatrix;
    HS_SHADER_ALIGNED glm::mat4x4 inverseProjectionMatrix;
    HS_SHADER_ALIGNED glm::mat4x4 inverseViewProjectionMatrix;
    HS_SHADER_ALIGNED glm::vec4 cameraPositionTime; // xyz = cameraPos, w = time
    HS_SHADER_ALIGNED glm::vec4 resolution;         // xy  = resolution, zw = padding
};
static_assert(sizeof(PerView) == 384 + 16 + 16, "PerView must be 416 bytes (6x mat4 + 2x vec4)");

// All scalars are packed into vec4 slots so the C++ layout matches Vulkan
// std140, Metal MSL, and HLSL cbuffer rules byte-for-byte without relying on
// any backend's vec3+scalar slot-fitting heuristic.
struct HS_SHADER_ALIGNED LightUBO
{
    HS_SHADER_ALIGNED glm::vec4 position;       // offset 0  : xyz = pos, w = padding
    HS_SHADER_ALIGNED glm::vec4 colorIntensity; // offset 16 : rgb = color, w = intensity
    HS_SHADER_ALIGNED glm::vec4 directionType;  // offset 32 : xyz = direction, w = type (as float)
};
static_assert(sizeof(LightUBO) == 48, "LightUBO must be 48 bytes (3x vec4)");

#pragma endregion

HS_NS_END

#endif
