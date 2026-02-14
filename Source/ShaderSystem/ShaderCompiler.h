//
//  ShaderCompiler.h
//  HSMR
//
//  Runtime Slang shader compilation with reflection extraction.
//
#ifndef __HS_SHADER_COMPILER_H__
#define __HS_SHADER_COMPILER_H__

#include "Precompile.h"
#include "ShaderSystem/ShaderSystemDefinition.h"

#include <string>
#include <vector>

namespace slang { struct IGlobalSession; }
namespace slang { struct ISession; }

HS_NS_BEGIN

struct HS_SHADER_SYSTEM_API ShaderCompileRequest
{
    std::string shaderName;
    std::string sourceCode;
    std::string sourceFilePath;             // For include path resolution
    std::vector<std::string> includePaths;
    std::vector<std::pair<std::string, std::string>> defines;
    EShaderStage requestedStages = EShaderStage::Vertex | EShaderStage::Fragment;
};

class HS_SHADER_SYSTEM_API ShaderCompiler
{
public:
    ShaderCompiler();
    ~ShaderCompiler();

    bool Initialize();
    void Finalize();

    ShaderCompileOutputEx Compile(const ShaderCompileRequest& request);
    ShaderCompileOutputEx CompileFromFile(const std::string& filePath,
                                          EShaderStage requestedStages = EShaderStage::Vertex | EShaderStage::Fragment);

    EShaderLanguage GetTargetLanguage() const;

private:
    void extractReflection(void* linkedProgram, int targetIndex,
                           const ShaderCompileRequest& request,
                           ShaderReflectionDataEx& outReflection);

    void extractVertexInputFromEntryPoint(void* entryPointReflection,
                                          ShaderVertexInputLayout& outLayout);

    void extractBufferBindings(void* paramLayout, EShaderStage stage,
                               std::vector<ShaderBufferBindingInfo>& outBindings);

    void extractTextureBindings(void* paramLayout, EShaderStage stage,
                                std::vector<ShaderTextureBindingInfo>& outTextures,
                                std::vector<ShaderSamplerBindingInfo>& outSamplers);

    EVertexFormat scalarTypeToVertexFormat(int scalarType, uint32 rows, uint32 cols) const;

    slang::IGlobalSession* _globalSession = nullptr;
    bool _initialized = false;
};

HS_NS_END

#endif
