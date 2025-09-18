#include "Object/Shader.h"
#include "Object/ObjectManager.h"
#include "ShaderSystem/ShaderCrossCompiler.h"
#include "Core/Log.h"
#include "HAL/FileSystem.h"
#include <fstream>
#include <sstream>

HS_NS_BEGIN

Shader::Shader(const std::string& source, EShaderStage stage, const std::string& entryPointName)
	: Object(Object::EType::SHADER)
{

}

Shader::~Shader()
{

}


void Shader::AddCache(const std::string& variantName, const std::vector<std::string>& defines)
{
    ShaderVariant* variant = new ShaderVariant();
    variant->macros = defines;

    _variants[variant] = variant;
    _isCompiled = false;

    HS_LOG(info, "Added shader variant: %s with %zu defines", variantName.c_str(), defines.size());
}

void Shader::RemoveCache(const std::string& variantName)
{
    auto it = _variants.find(variantName);
    if (it != _variants.end())
    {
        _variants.erase(it);
        HS_LOG(info, "Removed shader variant: %s", variantName.c_str());
    }
}

bool Shader::HasCache(const std::string& variantName) const
{
    return _variants.find(variantName) != _variants.end();
}

const ShaderVariant* Shader::GetVariant(const std::string& variantName) const
{
    auto it = _variants.find(variantName);
    return (it != _variants.end()) ? &it->second : nullptr;
}

const ShaderVariant* Shader::GetDefaultVariant() const
{
    // Try to find "default" variant first
    auto it = _variants.find("default");
    if (it != _variants.end() && it->second.isValid)
    {
        return &it->second;
    }

    // Otherwise, return the first valid variant
    for (const auto& pair : _variants)
    {
        if (pair.second.isValid)
        {
            return &pair.second;
        }
    }

    return nullptr;
}

void Shader::RegisterParameterInterface(const ShaderReflectionData& reflection)
{
    _parameterInterface.Clear();
    ExtractParametersFromReflection(reflection);
}

bool Shader::CompileVariants()
{
    if (_variants.empty())
    {
        // Create default variant if none exist
        AddVariant("default", {});
    }

    bool allSuccess = true;
    for (auto& pair : _variants)
    {
        if (!CompileVariant(pair.first))
        {
            allSuccess = false;
        }
    }

    _isCompiled = allSuccess;
    return allSuccess;
}

bool Shader::CompileVariant(const std::string& variantName)
{
    auto it = _variants.find(variantName);
    if (it == _variants.end())
    {
        HS_LOG(error, "Shader variant '%s' not found", variantName.c_str());
        return false;
    }

    ShaderVariant& variant = it->second;

    // Compile based on shader type
    bool success = false;
    
    if (_shaderType == EShaderStage::COMPUTE)
    {
        if (!_computeSource.empty())
        {
            success = CompileShaderVariant(variant, _computeSource, EShaderStage::COMPUTE);
        }
        else
        {
            HS_LOG(error, "No compute shader source for variant '%s'", variantName.c_str());
            return false;
        }
    }
    else
    {
        // For surface shaders, compile vertex first, then fragment
        if (!_vertexSource.empty())
        {
            success = CompileShaderVariant(variant, _vertexSource, EShaderStage::VERTEX);
        }
        
        if (success && !_fragmentSource.empty())
        {
            // For now, we compile vertex and fragment separately
            // In a full implementation, you might want to link them together
            ShaderVariant fragmentVariant = variant;
            success = CompileShaderVariant(fragmentVariant, _fragmentSource, EShaderStage::FRAGMENT);
            
            if (success)
            {
                // You might want to combine reflection data here
                // For now, we'll use the vertex shader reflection
            }
        }
    }

    if (success)
    {
        // Register parameter interface from reflection
        RegisterParameterInterface(variant.compiledShader.reflection);
        HS_LOG(info, "Successfully compiled shader variant: %s", variantName.c_str());
    }
    else
    {
        HS_LOG(error, "Failed to compile shader variant: %s", variantName.c_str());
    }

    return success;
}

bool Shader::IsCompiled() const
{
    if (_variants.empty())
    {
        return false;
    }

    // Check if at least one variant is compiled
    for (const auto& pair : _variants)
    {
        if (pair.second.isValid)
        {
            return true;
        }
    }

    return false;
}

bool Shader::compileShaderVariant(ShaderVariant& variant, const std::string& source, EShaderStage stage)
{
    ShaderCrossCompiler compiler;
    if (!compiler.Initialize())
    {
        return false;
    }

    // Setup compilation options
    ShaderCompileOption options = _compileOptions;
    options.stage = stage;
    options.macros = variant.macros;

    // Create source with defines prepended
    std::string finalSource;
    for (const std::string& define : variant.defines)
    {
        finalSource += "#define " + define + "\n";
    }
    finalSource += source;

    // Compile shader
    CompiledShader compiled = compiler.CompileShader(finalSource, name ? name : "unknown", options);
    
    if (compiled.isValid)
    {
        variant.compiledShader = std::move(compiled);
        variant.hash = GenerateVariantHash(source, variant.defines);
        variant.isValid = true;
        return true;
    }

    return false;
}

void Shader::ExtractParametersFromReflection(const ShaderReflectionData& reflection)
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
        // We'll register them as texture parameters for now
        _parameterInterface.RegisterParameter(
            sampler.name,
            ShaderParameterType::Texture2D,
            0,
            0
        );
    }
}

uint32 Shader::calculateVariantScore(const ShaderVariant& variant, const std::vector<std::string>& requestedDefines) const
{
    uint32 score = 0;
    
    // Perfect match gets highest score
    if (variant.defines.size() == requestedDefines.size())
    {
        bool perfectMatch = true;
        for (const std::string& requested : requestedDefines)
        {
            bool found = false;
            for (const std::string& variantDefine : variant.defines)
            {
                if (requested == variantDefine)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                perfectMatch = false;
                break;
            }
        }
        
        if (perfectMatch)
        {
            return 1000; // Perfect match
        }
    }
    
    // Count matching defines
    for (const std::string& requested : requestedDefines)
    {
        for (const std::string& variantDefine : variant.defines)
        {
            if (requested == variantDefine)
            {
                score += 10;
                break;
            }
        }
    }
    
    // Penalize extra defines in variant
    if (variant.defines.size() > requestedDefines.size())
    {
        score -= static_cast<uint32>(variant.defines.size() - requestedDefines.size());
    }
    
    return score;
}

HS_NS_END