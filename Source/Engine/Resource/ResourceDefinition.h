//
//  ResourceDefinition.h
//  HSMR
//
//  Created by Yongsik Im on 10/26/25.
//
#ifndef __HS_RESOURCE_DEFINITION_H__
#define __HS_RESOURCE_DEFINITION_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"
#include "ShaderSystem/ShaderSystemDefinition.h"

#include "Core/Hash.h"
#include "Core/Math/Common.h"

#include <unordered_map>

#define HS_SHADER_ALIGNED alignas(16)

HS_NS_BEGIN

// TODO: 리플렉션으로 자동 구성하는게 이상적임
#pragma region ShaderInput
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

#pragma endregion




enum class ShaderDebugInfoLevel
{
    None = 0,
    Minimal,
    Standard,
    Maximal
};

enum class ShaderOptimizationLevel
{
    None = 0,
    Standard,
    High,
    Maximal
};

struct ShaderReflectionData
{
    struct BufferBinding
    {
        std::string name;
        uint32 binding;
        uint32 size;
        EShaderStage stage;
    };
    
    struct TextureBinding
    {
        std::string name;
        uint32 binding;
        uint32 dimension;
        EShaderStage stage;
    };
    
    struct SamplerBinding
    {
        std::string name;
        uint32 binding;
        EShaderStage stage;
    };
    
    std::vector<BufferBinding> uniformBuffers;
    std::vector<BufferBinding> storageBuffers;
    std::vector<TextureBinding> textures;
    std::vector<SamplerBinding> samplers;
};

struct ShaderPredefine
{
    const char* name;
    const char* value;
};

struct ShaderCompileOption
{
    EShaderStage stage;
    std::string entryPoint = "main";
    EShaderLanguage targetLanguage;
    ShaderDebugInfoLevel debugInfoLevel = ShaderDebugInfoLevel::Maximal;
    ShaderOptimizationLevel optimizationLevel = ShaderOptimizationLevel::None;
    std::vector<ShaderPredefine> macros;
    std::vector<std::string> includePaths;
};

struct ShaderCompileInput
{
    ShaderCompileOption option;
    std::string shaderName;
    std::string sourceCode;
};

struct ShaderCompileOutput
{
    Scoped<char[]> code;
    size_t sourceCodeLen = 0;
    
    std::string diagnostics;
    ShaderReflectionData reflection;
    bool isValid = false;
};

namespace ShaderSystemUtil
{
std::string GetShaderStageString(EShaderStage stage);

std::string GetShaderLanguageString(EShaderLanguage language);
}

// Reflection-based byte buffer for material parameters
class HS_API MaterialParameterBlock
{
public:
    MaterialParameterBlock() = default;

    void Initialize(const ShaderBufferBindingInfo& bufferInfo);

    bool SetFloat(const std::string& name, float value);
    bool SetVec2(const std::string& name, const glm::vec2& value);
    bool SetVec3(const std::string& name, const glm::vec3& value);
    bool SetVec4(const std::string& name, const glm::vec4& value);
    bool SetMat3(const std::string& name, const glm::mat3& value);
    bool SetMat4(const std::string& name, const glm::mat4& value);
    bool SetInt(const std::string& name, int32 value);

    const void* GetData() const { return _data.empty() ? nullptr : _data.data(); }
    uint32 GetSize() const { return static_cast<uint32>(_data.size()); }
    bool IsDirty() const { return _dirty; }
    void ClearDirty() { _dirty = false; }
    bool IsInitialized() const { return !_data.empty(); }

    const std::string& GetBufferName() const { return _bufferName; }
    uint32 GetBinding() const { return _binding; }
    uint32 GetSet() const { return _set; }

private:
    bool setRaw(const std::string& name, const void* data, uint32 dataSize);

    std::vector<uint8> _data;
    std::unordered_map<uint32, std::pair<uint32, uint32>> _memberMap; // nameHash -> (offset, size)
    std::string _bufferName;
    uint32 _binding = 0;
    uint32 _set = 0;
    bool _dirty = false;
};


HS_NS_END

#endif
