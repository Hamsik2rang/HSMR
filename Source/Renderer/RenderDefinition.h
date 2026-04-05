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

struct HS_SHADER_ALIGNED PerView
{
    HS_SHADER_ALIGNED glm::mat4x4 viewMatrix;
    HS_SHADER_ALIGNED glm::mat4x4 projectionMatrix;
    HS_SHADER_ALIGNED glm::mat4x4 viewProjectionMatrix;
    HS_SHADER_ALIGNED glm::mat4x4 inverseViewMatrix;
    HS_SHADER_ALIGNED glm::mat4x4 inverseProjectionMatrix;
    HS_SHADER_ALIGNED glm::mat4x4 inverseViewProjectionMatrix;
    HS_SHADER_ALIGNED glm::vec3 cameraPosition; // w: padding
    HS_SHADER_ALIGNED float time;
    HS_SHADER_ALIGNED glm::vec2 resolution;
    HS_SHADER_ALIGNED glm::vec2 padding;
};

struct HS_SHADER_ALIGNED LightUBO
{
    HS_SHADER_ALIGNED glm::vec4 position;
    HS_SHADER_ALIGNED glm::vec3 color;
    HS_SHADER_ALIGNED float intensity;
    HS_SHADER_ALIGNED glm::vec3 direction;
    HS_SHADER_ALIGNED int type;
};
#pragma endregion

HS_NS_END

#endif
