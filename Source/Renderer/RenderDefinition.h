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

#define HS_SHADER_ALIGNED alignas(16)

HS_NS_BEGIN

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

    glm::vec4 cameraPosition; // w: padding
};

struct HS_SHADER_ALIGNED PerFrame
{
    glm::vec2 viewportSize;
    float time;
    float padding;
};

HS_NS_END

#endif
