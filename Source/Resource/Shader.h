#ifndef __HS_SHADER_H__
#define __HS_SHADER_H__

#include "Precompile.h"
#include "Resource/Object.h"
#include "Resource/ShaderParameterCollection.h"
#include "Resource/ShaderVariant.h"

#include "RHI/RHIDefinition.h"

#include "ShaderSystem/ShaderCrossCompiler.h"

#include <unordered_map>
#include <string>

namespace HS { class ShaderCache; }

HS_NS_BEGIN

class HS_RESOURCE_API Shader : public Object
{
public:
    Shader(const std::string& source, EShaderStage stage, const std::string& entryPointName);
    ~Shader() override;

    EShaderStage GetShaderStage() const { return _shaderType; }

    // =======================
    // SIMPLIFIED INTERFACE (1:1 Shader-Cache mapping)
    // =======================
    
    // Simple compilation interface - bypasses variant system
    bool CompileSimple();
    bool IsCompiledSimple() const { return _useSimpleMode && _simpleCompiledData.isValid; }
    
    // Get compiled shader data directly
    const ShaderCompileOutput* GetCompiledData() const;
    
    // Enable simple mode (disables variant system)
    void EnableSimpleMode() { _useSimpleMode = true; }
    bool IsSimpleModeEnabled() const { return _useSimpleMode; }

    // =======================
    // LEGACY VARIANT SYSTEM (will be removed in future phases)
    // =======================

    bool HasCache(const ShaderVariant& variant) const;
	ShaderCache* GetCache(const ShaderVariant& variant) const;
    
    // Legacy variant methods (not used in simple mode)
    const ShaderVariant* GetVariant(uint32 variantHash) const { return nullptr; }
    // const std::unordered_map<ShaderVariant, ShaderCache*>& GetAllVariants() const { return _variants; }  // Temporarily disabled
    
    // Get default variant (usually the one with no defines)
    const ShaderVariant* GetDefaultVariant() const;

    // Parameter interface registration (from reflection)
    void RegisterParameterInterface(const ShaderReflectionData& reflection);
    const ShaderParameterCollection& GetParameterInterface() const { return _parameterInterface; }

    // Compilation
    bool CompileVariants();
    bool CompileVariant(const std::string& variantName);
    bool IsCompiled() const;
    
    // Compilation options
    void SetCompilationOptions(const ShaderCompileOption& options) { _compileOptions = options; }
    const ShaderCompileOption& GetCompilationOptions() const { return _compileOptions; }

    // Utility
    uint32 GetVariantCount() const { return 0; }  // Simplified: always 0 variants

private:
    // Simple mode compilation
    bool compileSimpleShader();
    void extractParametersFromReflection(const ShaderReflectionData& reflection);
    
    // Legacy variant system methods
    void addCache(const ShaderVariant& variant);
    void removeCache(const ShaderVariant& variant);
    bool compileShaderCache(ShaderVariant& variant, const std::string& source, EShaderStage stage);
    uint32 calculateVariantScore(const ShaderVariant& variant, const std::vector<std::string>& requestedDefines) const;
    
    // Shader source storage
    std::string _source;           // Original shader source
    std::string _entryPointName;   // Entry point function name
    EShaderStage _shaderType;      // Shader stage type
    
    // Simple mode data (1:1 mapping)
    bool _useSimpleMode = false;
    ShaderCompileOutput _simpleCompiledData;
    
    // Legacy variant system data (will be removed)
    std::string _vertexSource;
    std::string _fragmentSource;
    std::string _computeSource;
    // std::unordered_map<ShaderVariant, ShaderCache*> _variants;  // Temporarily disabled
    bool _isCompiled = false;
    
    // Common data
    ShaderParameterCollection _parameterInterface;
    ShaderCompileOption _compileOptions;
};

HS_NS_END

#endif