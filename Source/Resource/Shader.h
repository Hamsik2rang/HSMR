#ifndef __HS_SHADER_H__
#define __HS_SHADER_H__

#include "Precompile.h"
#include "Core/Object.h"
#include "Resource/ResourceDefinition.h"

#include "RHI/RHIDefinition.h"
#include "ShaderSystem/ShaderSystemDefinition.h"

#include <unordered_map>
#include <string>

namespace hs { class ShaderCache; }

HS_NS_BEGIN

class HS_RESOURCE_API Shader : public Object
{
public:
    // Legacy constructor: single-stage shader from source string
    Shader(const std::string& source, EShaderStage stage, const std::string& entryPointName);

    // New constructor: multi-stage shader from .slang source for runtime compilation
    Shader(const std::string& shaderName, const std::string& sourceCode, EShaderStage requestedStages,
           const std::vector<std::string>& includePaths);

    ~Shader() override;

    EShaderStage GetShaderStage() const { return _shaderType; }

    // =======================
    // SIMPLIFIED INTERFACE (1:1 Shader-Cache mapping)
    // =======================

    bool IsCompiledSimple() const { return _useSimpleMode && _simpleCompiledData.isValid; }
    const ShaderCompileOutput* GetCompiledData() const;
    void EnableSimpleMode() { _useSimpleMode = true; }
    bool IsSimpleModeEnabled() const { return _useSimpleMode; }

    // Compilation
    bool CompileVariants();
    bool CompileVariant(const std::string& variantName);
    bool IsCompiled() const;

    void SetCompilationOptions(const ShaderCompileOption& options) { _compileOptions = options; }
    const ShaderCompileOption& GetCompilationOptions() const { return _compileOptions; }

    uint32 GetVariantCount() const { return 0; }

    // =======================
    // NEW: Runtime compilation via ShaderSystem
    // =======================

    // Compile using ShaderCompiler (Slang runtime)
    bool Compile();

    // Access reflection and bytecode from ShaderCompileOutputEx
    const ShaderReflectionDataEx& GetReflection() const { return _compileOutputEx.reflection; }
    const std::vector<uint8>* GetBytecode(EShaderStage stage) const;
    const char* GetEntryPoint(EShaderStage stage) const;
    bool IsCompiledEx() const { return _compileOutputEx.isValid; }
    const ShaderCompileOutputEx& GetCompileOutputEx() const { return _compileOutputEx; }

    // Allow ShaderLibrary to inject compile results directly
    void SetCompileOutputEx(ShaderCompileOutputEx&& output) { _compileOutputEx = std::move(output); }

    const std::string& GetShaderName() const { return _shaderName; }

private:
    void extractParametersFromReflection(const ShaderReflectionData& reflection);

    // Shader source storage
    std::string _source;           // Original shader source
    std::string _entryPointName;   // Entry point function name
    EShaderStage _shaderType;      // Shader stage type

    // Simple mode data (1:1 mapping)
    bool _useSimpleMode = false;
    ShaderCompileOutput _simpleCompiledData;

    // Legacy variant system data
    std::string _vertexSource;
    std::string _fragmentSource;
    std::string _computeSource;
    bool _isCompiled = false;

    ShaderCompileOption _compileOptions;

    // NEW: Runtime compilation data
    std::string _shaderName;
    std::vector<std::string> _includePaths;
    EShaderStage _requestedStages = EShaderStage::None;
    ShaderCompileOutputEx _compileOutputEx;
};

HS_NS_END

#endif
