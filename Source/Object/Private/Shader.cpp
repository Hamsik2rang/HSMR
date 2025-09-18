#include "Object/Shader.h"
#include "Object/ObjectManager.h"
#include "ShaderSystem/ShaderCrossCompiler.h"
#include "Core/Log.h"
#include "HAL/FileSystem.h"
#include <fstream>
#include <sstream>

HS_NS_BEGIN

Shader::Shader(const std::string& source, EShaderStage stage, const std::string& entryPointName)
	: Object(Object::EType::SHADER), _source(source), _shaderType(stage), _entryPointName(entryPointName)
{
    // Initialize simple mode compilation options
    _compileOptions.stage = stage;
    _compileOptions.entryPoint = entryPointName;
    _compileOptions.targetLanguage = EShaderLanguage::SPIRV; // Default to SPIRV
    
    // Default to simple mode for new shaders
    _useSimpleMode = true;
}

Shader::~Shader()
{
    // Cleanup will be handled by destructors
}

// =======================
// SIMPLIFIED INTERFACE IMPLEMENTATION
// =======================

bool Shader::CompileSimple()
{
    if (!_useSimpleMode)
    {
        HS_LOG(error, "CompileSimple() called but simple mode is not enabled");
        return false;
    }

    return compileSimpleShader();
}

const ShaderCompileOutput* Shader::GetCompiledData() const
{
    if (_useSimpleMode && _simpleCompiledData.isValid)
    {
        return &_simpleCompiledData;
    }
    return nullptr;
}

bool Shader::compileSimpleShader()
{
    ShaderCrossCompiler compiler;
    
    // Create compile input
    ShaderCompileInput input;
    input.option = _compileOptions;
    input.shaderName = name ? name : "unnamed_shader";
    input.sourceCode = _source;

    // Compile shader
    bool success = compiler.CompileShader(input, _simpleCompiledData);
    
    if (success && _simpleCompiledData.code && _simpleCompiledData.sourceCodeLen > 0)
    {
        _simpleCompiledData.isValid = true;
        // Register parameter interface from reflection
        RegisterParameterInterface(_simpleCompiledData.reflection);
        HS_LOG(info, "Successfully compiled shader: %s", input.shaderName.c_str());
        return true;
    }
    else
    {
        _simpleCompiledData.isValid = false;
        HS_LOG(error, "Failed to compile shader: %s - %s", 
               input.shaderName.c_str(), _simpleCompiledData.diagnostics.c_str());
        return false;
    }
}

// =======================
// LEGACY VARIANT SYSTEM (temporary compatibility)
// =======================

bool Shader::HasCache(const ShaderVariant& variant) const
{
    // For now, redirect to simple mode check
    if (_useSimpleMode)
    {
        return _simpleCompiledData.isValid;
    }
    
    // Legacy implementation would go here
    return false;
}

ShaderCache* Shader::GetCache(const ShaderVariant& variant) const
{
    // Legacy implementation - not used in simple mode
    return nullptr;
}

const ShaderVariant* Shader::GetDefaultVariant() const
{
    // Not used in simple mode
    return nullptr;
}

bool Shader::CompileVariants()
{
    if (_useSimpleMode)
    {
        // In simple mode, just compile the single shader
        return CompileSimple();
    }
    
    // Legacy variant compilation would go here
    return false;
}

bool Shader::CompileVariant(const std::string& variantName)
{
    if (_useSimpleMode)
    {
        // In simple mode, ignore variant name and compile single shader
        return CompileSimple();
    }
    
    // Legacy variant compilation would go here
    return false;
}

bool Shader::IsCompiled() const
{
    if (_useSimpleMode)
    {
        return _simpleCompiledData.isValid;
    }
    
    return _isCompiled;
}

// =======================
// COMMON FUNCTIONALITY
// =======================

void Shader::RegisterParameterInterface(const ShaderReflectionData& reflection)
{
    _parameterInterface.Clear();
    extractParametersFromReflection(reflection);
}

void Shader::extractParametersFromReflection(const ShaderReflectionData& reflection)
{
    uint32 offset = 0;

    // Process uniform buffers
    for (const auto& buffer : reflection.uniformBuffers)
    {
        _parameterInterface.RegisterParameter(
            buffer.name, 
            ShaderParameterType::Buffer, 
            offset, 
            buffer.size
        );
        offset += buffer.size;
    }

    // Process textures
    for (const auto& texture : reflection.textures)
    {
        ShaderParameterType textureType = (texture.dimension == 2) ? 
            ShaderParameterType::Texture2D : ShaderParameterType::TextureCube;
        
        _parameterInterface.RegisterParameter(
            texture.name,
            textureType,
            0, // Textures don't take space in constant buffer
            0
        );
    }

    // Process samplers
    for (const auto& sampler : reflection.samplers)
    {
        // Samplers are typically combined with textures in modern APIs
        _parameterInterface.RegisterParameter(
            sampler.name,
            ShaderParameterType::Texture2D,
            0,
            0
        );
    }
}

// =======================
// LEGACY VARIANT SYSTEM STUBS (will be removed)
// =======================

void Shader::addCache(const ShaderVariant& variant)
{
    // Legacy implementation stub
}

void Shader::removeCache(const ShaderVariant& variant)
{
    // Legacy implementation stub
}

bool Shader::compileShaderCache(ShaderVariant& variant, const std::string& source, EShaderStage stage)
{
    // Legacy implementation stub
    return false;
}

uint32 Shader::calculateVariantScore(const ShaderVariant& variant, const std::vector<std::string>& requestedDefines) const
{
    // Legacy implementation stub
    return 0;
}

HS_NS_END