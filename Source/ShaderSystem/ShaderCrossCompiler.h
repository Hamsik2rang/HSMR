#ifndef __HS_SHADER_CROSS_COMPILER_H__
#define __HS_SHADER_CROSS_COMPILER_H__

#include "Precompile.h"
#include "Core/Hash.h"
#include <vector>
#include <string>

HS_NS_BEGIN

enum class ShaderStage : uint32
{
    Vertex = 0,
    Fragment = 1,
    Geometry = 2,
    Hull = 3,
    Domain = 4,
    Compute = 5
};

enum class ShaderLanguage : uint32
{
    SPIRV = 0,
    MSL = 1,
    HLSL = 2
};

struct ShaderCompileOptions
{
    ShaderStage stage = ShaderStage::Vertex;
    ShaderLanguage targetLanguage = ShaderLanguage::SPIRV;
    std::string entryPoint = "main";
    std::string profile;
    bool enableDebugInfo = false;
    bool enableOptimization = true;
    std::vector<std::string> defines;
    std::vector<std::string> includePaths;
};

struct ShaderReflectionData
{
    struct BufferBinding
    {
        std::string name;
        uint32 binding;
        uint32 size;
        ShaderStage stage;
    };
    
    struct TextureBinding
    {
        std::string name;
        uint32 binding;
        uint32 dimension;
        ShaderStage stage;
    };
    
    struct SamplerBinding
    {
        std::string name;
        uint32 binding;
        ShaderStage stage;
    };
    
    std::vector<BufferBinding> uniformBuffers;
    std::vector<BufferBinding> storageBuffers;
    std::vector<TextureBinding> textures;
    std::vector<SamplerBinding> samplers;
};

struct CompiledShader
{
    std::vector<uint32> spirvBytecode;
    std::string compiledSource;
    ShaderReflectionData reflection;
    uint64 hash;
    bool isValid = false;
};

class HS_SHADERSYSTEM_API ShaderCrossCompiler
{
public:
    ShaderCrossCompiler();
    ~ShaderCrossCompiler();

    bool Initialize();
    void Shutdown();

    CompiledShader CompileShader(
        const std::string& sourceCode,
        const std::string& filename,
        const ShaderCompileOptions& options
    );

    CompiledShader CompileFromFile(
        const std::string& filepath,
        const ShaderCompileOptions& options
    );

    bool IsInitialized() const { return _initialized; }

    static ShaderCompileOptions CreateDefaultOptions(ShaderStage stage);
    static ShaderCompileOptions CreateVulkanOptions(ShaderStage stage);
    static ShaderCompileOptions CreateMetalOptions(ShaderStage stage);

private:
    bool _initialized = false;
    
    CompiledShader CompileSlangToSpirv(
        const std::string& sourceCode,
        const std::string& filename,
        const ShaderCompileOptions& options
    );
    
    bool CrossCompileSpirv(
        const std::vector<uint32>& spirvBytecode,
        const ShaderCompileOptions& options,
        CompiledShader& outShader
    );
    
    ShaderReflectionData ExtractReflection(const std::vector<uint32>& spirvBytecode);
};

HS_NS_END

#endif