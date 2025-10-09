//
//  ShaderSystemDefinition.h
//  ShaderSystem
//
//  Created by Yongsik Im on 9/11/25.
//
#ifndef __HS_SHADER_SYSTEM_DEFINITION_H__
#define __HS_SHADER_SYSTEM_DEFINITION_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"

#include "Core/Math/Common.h"

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
	NONE = 0,
	MINIMAL,
	STANDARD,
	MAXIMAL
};

enum class ShaderOptimizationLevel
{
	NONE = 0,
	STANDARD,
	HIGH,
	MAXIMAL
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
	ShaderDebugInfoLevel debugInfoLevel = ShaderDebugInfoLevel::MAXIMAL;
	ShaderOptimizationLevel optimizationLevel = ShaderOptimizationLevel::NONE;
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


HS_NS_END

#endif