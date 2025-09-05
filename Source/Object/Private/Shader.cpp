#include "Object/Shader.h"
#include "Object/ObjectManager.h"
#include "ShaderSystem/ShaderCrossCompiler.h"
#include "ShaderSystem/ShaderCacheManager.h"
#include "Core/Log.h"
#include "HAL/FileSystem.h"
#include <fstream>
#include <sstream>

HS_NS_BEGIN

Shader::Shader()
    : Object(EType::SHADER)
    , _shaderType(ShaderType::Surface)
{
    // Setup default compilation options
    _compileOptions = ShaderCrossCompiler::CreateDefaultOptions(ShaderStage::Vertex);
}

Shader::~Shader()
{
}

void Shader::SetVertexShaderSource(const std::string& source)
{
    _vertexSource = source;
    _isCompiled = false;
}

void Shader::SetFragmentShaderSource(const std::string& source)
{
    _fragmentSource = source;
    _isCompiled = false;
}

void Shader::SetComputeShaderSource(const std::string& source)
{
    _computeSource = source;
    _isCompiled = false;
}

bool Shader::LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath)
{
    std::string vertexSource = LoadShaderSourceFromFile(vertexPath);
    if (vertexSource.empty())
    {
        HS_LOG(error, "Failed to load vertex shader: {}", vertexPath);
        return false;
    }

    std::string fragmentSource = LoadShaderSourceFromFile(fragmentPath);
    if (fragmentSource.empty())
    {
        HS_LOG(error, "Failed to load fragment shader: {}", fragmentPath);
        return false;
    }

    SetVertexShaderSource(vertexSource);
    SetFragmentShaderSource(fragmentSource);

    name = (vertexPath + "+" + fragmentPath).c_str();

    HS_LOG(info, "Loaded shader from files: {} + {}", vertexPath, fragmentPath);
    return true;
}

bool Shader::LoadComputeFromFile(const std::string& computePath)
{
    std::string computeSource = LoadShaderSourceFromFile(computePath);
    if (computeSource.empty())
    {
        HS_LOG(error, "Failed to load compute shader: {}", computePath);
        return false;
    }

    SetComputeShaderSource(computeSource);
    _shaderType = ShaderType::Compute;

    name = computePath.c_str();

    HS_LOG(info, "Loaded compute shader from file: {}", computePath);
    return true;
}

void Shader::AddVariant(const std::string& variantName, const std::vector<std::string>& defines)
{
    ShaderVariant variant;
    variant.name = variantName;
    variant.defines = defines;
    variant.isValid = false;
    variant.hash = 0;

    _variants[variantName] = variant;
    _isCompiled = false;

    HS_LOG(info, "Added shader variant: {} with {} defines", variantName, defines.size());
}

void Shader::RemoveVariant(const std::string& variantName)
{
    auto it = _variants.find(variantName);
    if (it != _variants.end())
    {
        _variants.erase(it);
        HS_LOG(info, "Removed shader variant: {}", variantName);
    }
}

bool Shader::HasVariant(const std::string& variantName) const
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
        HS_LOG(error, "Shader variant '{}' not found", variantName);
        return false;
    }

    ShaderVariant& variant = it->second;

    // Compile based on shader type
    bool success = false;
    
    if (_shaderType == ShaderType::Compute)
    {
        if (!_computeSource.empty())
        {
            success = CompileShaderVariant(variant, _computeSource, ShaderStage::Compute);
        }
        else
        {
            HS_LOG(error, "No compute shader source for variant '{}'", variantName);
            return false;
        }
    }
    else
    {
        // For surface shaders, compile vertex first, then fragment
        if (!_vertexSource.empty())
        {
            success = CompileShaderVariant(variant, _vertexSource, ShaderStage::Vertex);
        }
        
        if (success && !_fragmentSource.empty())
        {
            // For now, we compile vertex and fragment separately
            // In a full implementation, you might want to link them together
            ShaderVariant fragmentVariant = variant;
            success = CompileShaderVariant(fragmentVariant, _fragmentSource, ShaderStage::Fragment);
            
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
        HS_LOG(info, "Successfully compiled shader variant: {}", variantName);
    }
    else
    {
        HS_LOG(error, "Failed to compile shader variant: {}", variantName);
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

const ShaderVariant* Shader::GetBestVariant(const std::vector<std::string>& requestedDefines) const
{
    if (_variants.empty())
    {
        return nullptr;
    }

    const ShaderVariant* bestVariant = nullptr;
    uint32 bestScore = 0;

    for (const auto& pair : _variants)
    {
        const ShaderVariant& variant = pair.second;
        if (!variant.isValid)
        {
            continue;
        }

        uint32 score = CalculateVariantScore(variant, requestedDefines);
        if (score > bestScore)
        {
            bestScore = score;
            bestVariant = &variant;
        }
    }

    return bestVariant ? bestVariant : GetDefaultVariant();
}

std::vector<std::string> Shader::GetVariantNames() const
{
    std::vector<std::string> names;
    names.reserve(_variants.size());
    
    for (const auto& pair : _variants)
    {
        names.push_back(pair.first);
    }
    
    return names;
}

std::string Shader::LoadShaderSourceFromFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        HS_LOG(error, "Cannot open shader file: {}", filepath);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

uint64 Shader::GenerateVariantHash(const std::string& source, const std::vector<std::string>& defines)
{
    uint64 hash = 14695981039346656037ULL; // FNV-1a offset basis
    
    // Hash source code
    for (char c : source)
    {
        hash ^= static_cast<uint64>(c);
        hash *= 1099511628211ULL; // FNV-1a prime
    }
    
    // Hash defines
    for (const std::string& define : defines)
    {
        for (char c : define)
        {
            hash ^= static_cast<uint64>(c);
            hash *= 1099511628211ULL;
        }
    }
    
    return hash;
}

bool Shader::CompileShaderVariant(ShaderVariant& variant, const std::string& source, ShaderStage stage)
{
    ShaderCrossCompiler compiler;
    if (!compiler.Initialize())
    {
        return false;
    }

    // Setup compilation options
    ShaderCompileOptions options = _compileOptions;
    options.stage = stage;
    options.defines = variant.defines;

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

uint32 Shader::CalculateVariantScore(const ShaderVariant& variant, const std::vector<std::string>& requestedDefines) const
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