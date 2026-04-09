#include "Renderer/RenderPass/ForwardDebugPass.h"

#include "Core/Hash.h"
#include "Core/Log.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"
#include "RHI/ResourceHandle.h"

#include "Renderer/ShaderLibrary.h"

#include "Resource/Shader.h"

#include "Core/ThirdParty/glm/gtc/constants.hpp"

#include <algorithm>
#include <cstddef>

HS_NS_BEGIN

namespace
{
constexpr uint32 s_debugCircleSegments = 48;
constexpr float s_directionalLightLength = 2.0f;
constexpr float s_directionalLightMarkerSize = 0.35f;

glm::vec3 safeNormalize(const glm::vec3& value, const glm::vec3& fallback)
{
    float len = glm::length(value);
    if (len <= 0.0001f)
    {
        return fallback;
    }
    return value / len;
}

glm::vec3 transformPoint(const glm::mat4& matrix, const glm::vec3& point)
{
    return glm::vec3(matrix * glm::vec4(point, 1.0f));
}

glm::vec3 transformDirection(const glm::mat4& matrix, const glm::vec3& direction, const glm::vec3& fallback)
{
    return safeNormalize(glm::mat3(matrix) * direction, fallback);
}

size_t buildPipelineKey(const PipelineRenderTargetLayout& renderTargetLayout)
{
    return std::hash<PipelineRenderTargetLayout>{}(renderTargetLayout);
}
}

ForwardDebugPass::~ForwardDebugPass()
{
    Shutdown();
}

bool ForwardDebugPass::Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext)
{
    _rhiContext = rhiContext;

    Shader* shader = shaderLibrary->GetOrCompile("DebugLine", EShaderStage::Vertex | EShaderStage::Fragment);
    if (!shader || !shader->IsCompiledEx())
    {
        HS_LOG(error, "[ForwardDebugPass] Failed to compile DebugLine.slang");
        return false;
    }

    const auto* vsBytecode = shader->GetBytecode(EShaderStage::Vertex);
    const auto* fsBytecode = shader->GetBytecode(EShaderStage::Fragment);
    if (!vsBytecode || !fsBytecode)
    {
        HS_LOG(error, "[ForwardDebugPass] Missing DebugLine shader bytecode");
        return false;
    }

    ShaderInfo vsInfo{};
    vsInfo.stage     = EShaderStage::Vertex;
    vsInfo.entryName = shader->GetEntryPoint(EShaderStage::Vertex);
    _vertexShader    = _rhiContext->CreateShader(
        "DebugLineVS", vsInfo,
        reinterpret_cast<const char*>(vsBytecode->data()),
        vsBytecode->size()
    );

    ShaderInfo fsInfo{};
    fsInfo.stage     = EShaderStage::Fragment;
    fsInfo.entryName = shader->GetEntryPoint(EShaderStage::Fragment);
    _fragmentShader  = _rhiContext->CreateShader(
        "DebugLineFS", fsInfo,
        reinterpret_cast<const char*>(fsBytecode->data()),
        fsBytecode->size()
    );

    if (!_vertexShader || !_fragmentShader)
    {
        HS_LOG(error, "[ForwardDebugPass] Failed to create RHI shaders");
        return false;
    }

    _isInitialized = true;
    return true;
}

void ForwardDebugPass::Shutdown()
{
    if (!_rhiContext) return;

    _rhiContext->WaitForIdle();

    resetPipelines();

    if (_vertexBuffer)
    {
        _rhiContext->DestroyBuffer(_vertexBuffer);
        _vertexBuffer = nullptr;
    }

    if (_resourceSet)
    {
        _rhiContext->DestroyResourceSet(_resourceSet);
        _resourceSet = nullptr;
    }

    if (_resourceLayout)
    {
        _rhiContext->DestroyResourceLayout(_resourceLayout);
        _resourceLayout = nullptr;
    }

    if (_fragmentShader)
    {
        _rhiContext->DestroyShader(_fragmentShader);
        _fragmentShader = nullptr;
    }

    if (_vertexShader)
    {
        _rhiContext->DestroyShader(_vertexShader);
        _vertexShader = nullptr;
    }

    _vertices.clear();
    _perViewBuffer  = nullptr;
    _vertexCapacity = 0;
    _vertexCount    = 0;
    _isInitialized  = false;
    _rhiContext     = nullptr;
}

bool ForwardDebugPass::Prepare(const RenderSceneSnapshot& snapshot)
{
    if (!_isInitialized) return false;

    _vertices.clear();
    _vertexCount = 0;

    for (const DebugCameraSnapshot& camera : snapshot.debugCameras)
    {
        addCamera(camera);
    }

    for (const DebugLightSnapshot& light : snapshot.debugLights)
    {
        addLight(light);
    }

    _vertexCount = static_cast<uint32>(_vertices.size());
    if (_vertexCount == 0)
    {
        return true;
    }

    if (_vertexCount > _vertexCapacity)
    {
        if (_vertexBuffer)
        {
            _rhiContext->DestroyBuffer(_vertexBuffer);
            _vertexBuffer = nullptr;
        }

        _vertexCapacity = std::max<uint32>(_vertexCount, _vertexCapacity == 0 ? 256 : _vertexCapacity * 2);
        _vertexBuffer = _rhiContext->CreateBuffer(
            "DebugLineVertexBuffer",
            nullptr,
            sizeof(DebugLineVertex) * _vertexCapacity,
            EBufferUsage::Vertex,
            EBufferMemoryOption::Dynamic
        );
    }

    if (_vertexBuffer)
    {
        _rhiContext->UpdateBuffer(
            _vertexBuffer,
            0,
            _vertices.data(),
            sizeof(DebugLineVertex) * _vertexCount
        );
    }

    return _vertexBuffer != nullptr;
}

void ForwardDebugPass::rebuildResourceBindings(RHIBuffer* perViewBuffer)
{
    if (_resourceSet)
    {
        _rhiContext->DestroyResourceSet(_resourceSet);
        _resourceSet = nullptr;
    }

    if (_resourceLayout)
    {
        _rhiContext->DestroyResourceLayout(_resourceLayout);
        _resourceLayout = nullptr;
    }

    ResourceBinding binding{};
    binding.type       = EResourceType::UniformBuffer;
    binding.stage      = EShaderStage::Vertex;
    binding.binding    = 0;
    binding.arrayCount = 1;
    binding.resource.buffers.push_back(perViewBuffer);
    binding.resource.offsets.push_back(0);

    _resourceLayout = _rhiContext->CreateResourceLayout("DebugLineLayout", &binding, 1);
    if (!_resourceLayout)
    {
        HS_LOG(error, "[ForwardDebugPass] Failed to create resource layout");
        return;
    }

    _resourceSet = _rhiContext->CreateResourceSet("DebugLineSet", _resourceLayout);
    if (!_resourceSet)
    {
        HS_LOG(error, "[ForwardDebugPass] Failed to create resource set");
        return;
    }

    _perViewBuffer = perViewBuffer;
}

void ForwardDebugPass::resetPipelines()
{
    for (auto& [key, pipeline] : _pipelineCache)
    {
        if (pipeline)
        {
            _rhiContext->DestroyGraphicsPipeline(pipeline);
        }
    }
    _pipelineCache.clear();
}

RHIGraphicsPipeline* ForwardDebugPass::GetOrCreatePipeline(const PipelineRenderTargetLayout& renderTargetLayout,
                                                           RHIBuffer* perViewBuffer)
{
    if (!_isInitialized || !perViewBuffer) return nullptr;

    if (perViewBuffer != _perViewBuffer)
    {
        rebuildResourceBindings(perViewBuffer);
        resetPipelines();
    }

    if (!_resourceLayout || !_resourceSet) return nullptr;

    size_t pipelineKey = buildPipelineKey(renderTargetLayout);
    auto it = _pipelineCache.find(pipelineKey);
    if (it != _pipelineCache.end())
    {
        return it->second;
    }

    VertexInputStateDescriptor viDesc{};
    VertexInputLayoutDescriptor layout{};
    layout.binding        = 0;
    layout.stride         = sizeof(DebugLineVertex);
    layout.stepRate       = 1;
    layout.useInstancing  = false;
    viDesc.layouts.push_back(layout);

    VertexInputAttributeDescriptor positionAttribute{};
    positionAttribute.location = 0;
    positionAttribute.binding  = 0;
    positionAttribute.format   = EVertexFormat::Float3;
    positionAttribute.offset   = offsetof(DebugLineVertex, position);
    viDesc.attributes.push_back(positionAttribute);

    VertexInputAttributeDescriptor colorAttribute{};
    colorAttribute.location = 1;
    colorAttribute.binding  = 0;
    colorAttribute.format   = EVertexFormat::Float4;
    colorAttribute.offset   = offsetof(DebugLineVertex, color);
    viDesc.attributes.push_back(colorAttribute);

    InputAssemblyStateDescriptor iaDesc{};
    iaDesc.primitiveTopology = EPrimitiveTopology::LineList;
    iaDesc.isRestartEnable   = false;

    RasterizerStateDescriptor rsDesc{};
    rsDesc.cullMode                = ECullMode::None;
    rsDesc.frontFace               = EFrontFace::CounterClockwise;
    rsDesc.polygonMode             = EPolygonMode::Fill;
    rsDesc.depthClampEnable        = false;
    rsDesc.rasterizerDiscardEnable = false;
    rsDesc.depthBiasEnable         = false;
    rsDesc.lineWidth               = 1.0f;

    DepthStencilStateDescriptor dsDesc{};
    dsDesc.depthTestEnable      = renderTargetLayout.useDepthStencilAttachment;
    dsDesc.depthWriteEnable     = false;
    dsDesc.depthCompareOp       = ECompareOp::LessOrEqual;
    dsDesc.depthBoundTestEnable = false;
    dsDesc.stencilTestEnable    = false;

    ColorBlendStateDescriptor cbDesc{};
    cbDesc.logicOpEnable   = false;
    cbDesc.attachmentCount = renderTargetLayout.colorAttachmentCount;
    cbDesc.attachments.resize(cbDesc.attachmentCount);
    for (size_t i = 0; i < cbDesc.attachments.size(); ++i)
    {
        ColorBlendAttachmentDescriptor& attachment = cbDesc.attachments[i];
        attachment.blendEnable    = true;
        attachment.srcColorFactor = EBlendFactor::SrcAlpha;
        attachment.dstColorFactor = EBlendFactor::OneMinusSrcAlpha;
        attachment.colorBlendOp   = EBlendOp::Add;
        attachment.srcAlphaFactor = EBlendFactor::One;
        attachment.dstAlphaFactor = EBlendFactor::OneMinusSrcAlpha;
        attachment.alphaBlendOp   = EBlendOp::Add;
    }

    ShaderProgramDescriptor shaderDesc{};
    shaderDesc.stages.push_back(_vertexShader);
    shaderDesc.stages.push_back(_fragmentShader);

    GraphicsPipelineInfo gpInfo{};
    gpInfo.shaderDesc         = shaderDesc;
    gpInfo.inputAssemblyDesc  = iaDesc;
    gpInfo.vertexInputDesc    = viDesc;
    gpInfo.rasterizerDesc     = rsDesc;
    gpInfo.depthStencilDesc   = dsDesc;
    gpInfo.colorBlendDesc     = cbDesc;
    gpInfo.renderTargetLayout = renderTargetLayout;
    gpInfo.resourceLayout     = _resourceLayout;

    RHIGraphicsPipeline* pipeline = _rhiContext->CreateGraphicsPipeline("DebugLinePipeline", gpInfo);
    if (!pipeline)
    {
        HS_LOG(error, "[ForwardDebugPass] Failed to create graphics pipeline");
        return nullptr;
    }

    _pipelineCache[pipelineKey] = pipeline;
    return pipeline;
}

void ForwardDebugPass::addLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color)
{
    _vertices.push_back(DebugLineVertex{from, color});
    _vertices.push_back(DebugLineVertex{to, color});
}

void ForwardDebugPass::addCamera(const DebugCameraSnapshot& camera)
{
    constexpr glm::vec4 cameraColor(0.2f, 0.75f, 1.0f, 0.85f);

    glm::vec3 corners[8]{};
    if (camera.projectionType == EDebugCameraProjectionType::Perspective)
    {
        float nearHeight = glm::tan(glm::radians(camera.fov) * 0.5f) * camera.nearPlane;
        float nearWidth  = nearHeight * camera.aspectRatio;
        float farHeight  = glm::tan(glm::radians(camera.fov) * 0.5f) * camera.farPlane;
        float farWidth   = farHeight * camera.aspectRatio;

        corners[0] = transformPoint(camera.worldMatrix, glm::vec3(-nearWidth, -nearHeight, -camera.nearPlane));
        corners[1] = transformPoint(camera.worldMatrix, glm::vec3( nearWidth, -nearHeight, -camera.nearPlane));
        corners[2] = transformPoint(camera.worldMatrix, glm::vec3( nearWidth,  nearHeight, -camera.nearPlane));
        corners[3] = transformPoint(camera.worldMatrix, glm::vec3(-nearWidth,  nearHeight, -camera.nearPlane));
        corners[4] = transformPoint(camera.worldMatrix, glm::vec3(-farWidth, -farHeight, -camera.farPlane));
        corners[5] = transformPoint(camera.worldMatrix, glm::vec3( farWidth, -farHeight, -camera.farPlane));
        corners[6] = transformPoint(camera.worldMatrix, glm::vec3( farWidth,  farHeight, -camera.farPlane));
        corners[7] = transformPoint(camera.worldMatrix, glm::vec3(-farWidth,  farHeight, -camera.farPlane));
    }
    else
    {
        float halfHeight = camera.orthoSize;
        float halfWidth  = camera.orthoSize * camera.aspectRatio;

        corners[0] = transformPoint(camera.worldMatrix, glm::vec3(-halfWidth, -halfHeight, -camera.nearPlane));
        corners[1] = transformPoint(camera.worldMatrix, glm::vec3( halfWidth, -halfHeight, -camera.nearPlane));
        corners[2] = transformPoint(camera.worldMatrix, glm::vec3( halfWidth,  halfHeight, -camera.nearPlane));
        corners[3] = transformPoint(camera.worldMatrix, glm::vec3(-halfWidth,  halfHeight, -camera.nearPlane));
        corners[4] = transformPoint(camera.worldMatrix, glm::vec3(-halfWidth, -halfHeight, -camera.farPlane));
        corners[5] = transformPoint(camera.worldMatrix, glm::vec3( halfWidth, -halfHeight, -camera.farPlane));
        corners[6] = transformPoint(camera.worldMatrix, glm::vec3( halfWidth,  halfHeight, -camera.farPlane));
        corners[7] = transformPoint(camera.worldMatrix, glm::vec3(-halfWidth,  halfHeight, -camera.farPlane));
    }

    for (uint32 i = 0; i < 4; ++i)
    {
        addLine(corners[i], corners[(i + 1) % 4], cameraColor);
        addLine(corners[i + 4], corners[((i + 1) % 4) + 4], cameraColor);
        addLine(corners[i], corners[i + 4], cameraColor);
    }
}

void ForwardDebugPass::addLight(const DebugLightSnapshot& light)
{
    if (!light.isEnabled) return;

    glm::vec3 center  = transformPoint(light.worldMatrix, glm::vec3(0.0f));
    glm::vec3 forward = transformDirection(light.worldMatrix, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    glm::vec3 right   = transformDirection(light.worldMatrix, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 up      = transformDirection(light.worldMatrix, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec4 color(glm::clamp(light.color * std::max(light.intensity, 0.25f), glm::vec3(0.0f), glm::vec3(1.0f)), 0.9f);
    float range = std::max(light.range, 0.1f);

    if (light.type == EDebugLightType::Directional)
    {
        glm::vec3 tip = center + forward * s_directionalLightLength;
        addLine(center, tip, color);
        addLine(tip, tip - forward * s_directionalLightMarkerSize + right * s_directionalLightMarkerSize, color);
        addLine(tip, tip - forward * s_directionalLightMarkerSize - right * s_directionalLightMarkerSize, color);
        addLine(tip, tip - forward * s_directionalLightMarkerSize + up * s_directionalLightMarkerSize, color);
        addLine(tip, tip - forward * s_directionalLightMarkerSize - up * s_directionalLightMarkerSize, color);
        return;
    }

    if (light.type == EDebugLightType::Point)
    {
        addCircle(center, right, up, range, color, s_debugCircleSegments);
        addCircle(center, right, forward, range, color, s_debugCircleSegments);
        addCircle(center, up, forward, range, color, s_debugCircleSegments);
        return;
    }

    float outerRadius = glm::tan(glm::radians(light.outerConeAngle)) * range;
    glm::vec3 coneCenter = center + forward * range;
    addCircle(coneCenter, right, up, outerRadius, color, s_debugCircleSegments);

    for (uint32 i = 0; i < 4; ++i)
    {
        float angle = glm::half_pi<float>() * static_cast<float>(i);
        glm::vec3 edge = coneCenter + right * glm::cos(angle) * outerRadius + up * glm::sin(angle) * outerRadius;
        addLine(center, edge, color);
    }

    glm::vec4 innerColor = color;
    innerColor.a *= 0.45f;
    float innerRadius = glm::tan(glm::radians(light.innerConeAngle)) * range;
    addCircle(coneCenter, right, up, innerRadius, innerColor, s_debugCircleSegments);
}

void ForwardDebugPass::addCircle(const glm::vec3& center,
                                 const glm::vec3& axisA,
                                 const glm::vec3& axisB,
                                 float radius,
                                 const glm::vec4& color,
                                 uint32 segmentCount)
{
    if (segmentCount < 3 || radius <= 0.0f) return;

    for (uint32 i = 0; i < segmentCount; ++i)
    {
        float angle0 = glm::two_pi<float>() * static_cast<float>(i) / static_cast<float>(segmentCount);
        float angle1 = glm::two_pi<float>() * static_cast<float>(i + 1) / static_cast<float>(segmentCount);
        glm::vec3 p0 = center + axisA * glm::cos(angle0) * radius + axisB * glm::sin(angle0) * radius;
        glm::vec3 p1 = center + axisA * glm::cos(angle1) * radius + axisB * glm::sin(angle1) * radius;
        addLine(p0, p1, color);
    }
}

HS_NS_END
