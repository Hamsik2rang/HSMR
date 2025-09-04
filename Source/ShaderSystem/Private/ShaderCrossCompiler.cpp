#include "ShaderSystem/ShaderCrossCompiler.h"

#include "ShaderSystem/Slang/SlangCompiler.h"
#include "ShaderSystem/Spirv/SpirvCrossHelper.h"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

namespace HS
{
    uint64 HashString(const std::string& str)
    {
        uint64 hash = 14695981039346656037ULL;
        for (char c : str)
        {
            hash ^= static_cast<uint64>(c);
            hash *= 1099511628211ULL;
        }
        return hash;
    }
}

HS_NS_BEGIN

ShaderCrossCompiler::ShaderCrossCompiler()
{
}

ShaderCrossCompiler::~ShaderCrossCompiler()
{
    Shutdown();
}

bool ShaderCrossCompiler::Initialize()
{
    if (_initialized)
    {
        return true;
    }

    HS_LOG(info, "Initializing ShaderCrossCompiler...");
    
    _initialized = true;
    return true;
}

void ShaderCrossCompiler::Shutdown()
{
    if (!_initialized)
    {
        return;
    }

    HS_LOG(info, "Shutting down ShaderCrossCompiler...");
    _initialized = false;
}

CompiledShader ShaderCrossCompiler::CompileShader(
    const std::string& sourceCode,
    const std::string& filename,
    const ShaderCompileOptions& options)
{
    CompiledShader result;
    
    if (!_initialized)
    {
        HS_LOG(error, "ShaderCrossCompiler not initialized");
        return result;
    }

    HS_LOG(info, "Compiling shader: {}", filename);

    result = CompileSlangToSpirv(sourceCode, filename, options);
    
    if (!result.isValid)
    {
        HS_LOG(error, "Failed to compile shader to SPIR-V: {}", filename);
        return result;
    }

    if (!CrossCompileSpirv(result.spirvBytecode, options, result))
    {
        HS_LOG(error, "Failed to cross-compile SPIR-V: {}", filename);
        result.isValid = false;
        return result;
    }

    result.reflection = ExtractReflection(result.spirvBytecode);

    std::stringstream ss;
    ss << filename << "_" << static_cast<uint32>(options.stage) << "_" << static_cast<uint32>(options.targetLanguage);
    result.hash = HashString(ss.str() + sourceCode);

    HS_LOG(info, "Successfully compiled shader: {}", filename);
    return result;
}

CompiledShader ShaderCrossCompiler::CompileFromFile(
    const std::string& filepath,
    const ShaderCompileOptions& options)
{
    CompiledShader result;
    
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        HS_LOG(error, "Failed to open shader file: {}", filepath);
        return result;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string sourceCode = buffer.str();
    return CompileShader(sourceCode, filepath, options);
}

CompiledShader ShaderCrossCompiler::CompileSlangToSpirv(
    const std::string& sourceCode,
    const std::string& filename,
    const ShaderCompileOptions& options)
{
    CompiledShader result;
    
    SlangCompiler slangCompiler;
    if (!slangCompiler.Initialize())
    {
        HS_LOG(error, "Failed to initialize Slang compiler");
        return result;
    }

    SlangCompileRequest request;
    request.sourceCode = sourceCode;
    request.filename = filename;
    request.entryPoint = options.entryPoint;
    request.stage = static_cast<SlangStage>(static_cast<uint32>(options.stage));
    request.defines = options.defines;
    request.includePaths = options.includePaths;
    request.enableDebugInfo = options.enableDebugInfo;
    request.enableOptimization = options.enableOptimization;

    SlangCompileResult slangResult = slangCompiler.CompileToSpirv(request);
    
    if (!slangResult.success)
    {
        HS_LOG(error, "Slang compilation failed: {}", slangResult.diagnostics);
        return result;
    }

    if (!slangResult.diagnostics.empty())
    {
        HS_LOG(warning, "Slang compilation warnings: {}", slangResult.diagnostics);
    }

    result.spirvBytecode = std::move(slangResult.spirvBytecode);
    result.isValid = true;
    
    return result;
}

bool ShaderCrossCompiler::CrossCompileSpirv(
    const std::vector<uint32>& spirvBytecode,
    const ShaderCompileOptions& options,
    CompiledShader& outShader)
{
    if (options.targetLanguage == ShaderLanguage::SPIRV)
    {
        return true;
    }

    SpirvCrossHelper helper;
    if (!helper.Initialize())
    {
        HS_LOG(error, "Failed to initialize SPIRV-Cross helper");
        return false;
    }

    switch (options.targetLanguage)
    {
    case ShaderLanguage::MSL:
        outShader.compiledSource = helper.CrossCompileToMSL(spirvBytecode, options.enableDebugInfo);
        break;
        
    case ShaderLanguage::HLSL:
        outShader.compiledSource = helper.CrossCompileToHLSL(spirvBytecode, options.enableDebugInfo);
        break;
        
    default:
        HS_LOG(error, "Unsupported target shader language: {}", static_cast<uint32>(options.targetLanguage));
        return false;
    }

    if (outShader.compiledSource.empty())
    {
        HS_LOG(error, "Cross-compilation resulted in empty source code");
        return false;
    }

    return true;
}

ShaderReflectionData ShaderCrossCompiler::ExtractReflection(const std::vector<uint32>& spirvBytecode)
{
    ShaderReflectionData reflectionData;
    
    SpirvCrossHelper helper;
    if (!helper.Initialize())
    {
        HS_LOG(error, "Failed to initialize SPIRV-Cross helper for reflection");
        return reflectionData;
    }

    SpirvReflectionData spirvReflection = helper.ExtractReflection(spirvBytecode);
    
    for (const auto& buffer : spirvReflection.uniformBuffers)
    {
        ShaderReflectionData::BufferBinding binding;
        binding.name = buffer.name;
        binding.binding = buffer.binding;
        binding.size = buffer.size;
        binding.stage = ShaderStage::Vertex;
        reflectionData.uniformBuffers.push_back(binding);
    }
    
    for (const auto& buffer : spirvReflection.storageBuffers)
    {
        ShaderReflectionData::BufferBinding binding;
        binding.name = buffer.name;
        binding.binding = buffer.binding;
        binding.size = buffer.size;
        binding.stage = ShaderStage::Vertex;
        reflectionData.storageBuffers.push_back(binding);
    }
    
    for (const auto& image : spirvReflection.sampledImages)
    {
        ShaderReflectionData::TextureBinding binding;
        binding.name = image.name;
        binding.binding = image.binding;
        binding.dimension = static_cast<uint32>(image.imageType.dim);
        binding.stage = ShaderStage::Vertex;
        reflectionData.textures.push_back(binding);
    }
    
    for (const auto& sampler : spirvReflection.samplers)
    {
        ShaderReflectionData::SamplerBinding binding;
        binding.name = sampler.name;
        binding.binding = sampler.binding;
        binding.stage = ShaderStage::Vertex;
        reflectionData.samplers.push_back(binding);
    }

    return reflectionData;
}

ShaderCompileOptions ShaderCrossCompiler::CreateDefaultOptions(ShaderStage stage)
{
    ShaderCompileOptions options;
    options.stage = stage;
    options.targetLanguage = ShaderLanguage::SPIRV;
    options.entryPoint = "main";
    options.enableDebugInfo = false;
    options.enableOptimization = true;
    return options;
}

ShaderCompileOptions ShaderCrossCompiler::CreateVulkanOptions(ShaderStage stage)
{
    ShaderCompileOptions options = CreateDefaultOptions(stage);
    options.targetLanguage = ShaderLanguage::SPIRV;
    return options;
}

ShaderCompileOptions ShaderCrossCompiler::CreateMetalOptions(ShaderStage stage)
{
    ShaderCompileOptions options = CreateDefaultOptions(stage);
    options.targetLanguage = ShaderLanguage::MSL;
    return options;
}

HS_NS_END