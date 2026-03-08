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
    glm::mat4x4 modelMatrix;
    glm::mat4x4 inverseModelMatrix;
};

struct HS_SHADER_ALIGNED PerView
{
    glm::mat4x4 viewMatrix;
    glm::mat4x4 projectionMatrix;
    glm::mat4x4 viewProjectionMatrix;
    glm::mat4x4 inverseViewMatrix;
    glm::mat4x4 inverseProjectionMatrix;
    glm::mat4x4 inverseViewProjectionMatrix;
    union
    {
        glm::vec3 camPosAndTime;
        struct
        {
            glm::vec3 cameraPosition; // w: padding
            float time;
        };
    };
    union
    {
        glm::vec4 resolutionAndPadding;
        struct
        {

            glm::vec2 resolution;
            glm::vec2 padding;
        };
    };
};

struct HS_SHADER_ALIGNED LightUBO
{
    glm::vec4 position;
    union
    {
        struct
        {
            glm::vec3 color;
            float intensity;
        };
        glm::vec4 colorAndIntensity;
    };
    union
    {
        struct
        {
            glm::vec3 direction;
            int type;
        };
        glm::vec4 directionAndType;
    };
};
#pragma endregion

HS_NS_END

#endif
