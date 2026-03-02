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

#pragma region RenderGraph
enum class ERGPassFlag
{
    None           = 0,
    Raster         = 1 << 0,
    Compute        = 1 << 1,
    AsyncCompute   = 1 << 2,
    Copy           = 1 << 3,
    NeverCull      = 1 << 4,
    SkipRenderPass = 1 << 5,
    NeverMerge     = 1 << 6,
    NeverParallel  = 1 << 7,
};

enum class ERGBufferAccess
{
    ReadOnly = 0,
    ReadWrite, // SSBO
};

enum class ERGTextureAccess
{
    ReadOnly = 0,
    ColorAttachmentWrite,
    ReadWrite, // ← General 레이아웃, UAV에 해당
    DepthAttachmentRead,
    DepthAttachmentWrite,
    DepthStencilAttachmentRead,
    DepthStencilAttachmentWrite,
    TransferRead,
    TransferWrite,
    ComputeShaderRead,
    ComputeShaderWrite, // ← Compute UAV Write에 해당
    FragmentShaderReadSampledImageOrUniformTexelBuffer,
    Present
};

struct ERGTextureDescriptor
{
    TextureInfo info;
    ERGTextureAccess access;
    const char* name;
};

struct ERGBufferDescriptor
{
    BufferInfo info;
    ERGBufferAccess access;
    const char* name;
};

#pragma endregion

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
        glm::vec4 colorAndIntensity;
        struct
        {
            glm::vec3 color;
            float intensity;
        };
    };
    union
    {
        glm::vec4 directionAndType;
        struct
        {
            glm::vec3 direction;
            int type;
        };
    };
};
#pragma endregion

HS_NS_END

#endif
