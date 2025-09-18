#ifndef __HS_SHADER_H__
#define __HS_SHADER_H__

#include "Precompile.h"
#include "Object/Object.h"
#include "Object/ShaderParameterCollection.h"
#include "Object/ShaderVariant.h"

#include "RHI/RHIDefinition.h"

#include "ShaderSystem/ShaderCrossCompiler.h"

#include <unordered_map>
#include <string>

namespace HS { class ShaderCache; }

HS_NS_BEGIN

class HS_OBJECT_API Shader : public Object
{
public:
    Shader(const std::string& source, EShaderStage stage, const std::string& entryPointName);
    ~Shader() override;

    EShaderStage GetShaderStage() const { return _shaderType; }

    // Variant management

    bool HasCache(const ShaderVariant& variant) const;
	ShaderCache* GetCache(const ShaderVariant& variant) const;
    
    HS_FORCEINLINE const ShaderVariant* GetVariant(uint32 variantHash) const { if (_variants.find(variantHash) != _variants.end()) return _variants[variantHash]; return nullptr; }
    HS_FORCEINLINE const std::unordered_map<uint32, ShaderVariant*>& GetAllVariants() const { return _variants; }
    
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
    uint32 GetVariantCount() const { return static_cast<uint32>(_variants.size()); }

private:
    void addCache(const ShaderVariant& variant);
    void removeCache(const ShaderVariant& variant);

    bool compileShaderCache(ShaderVariant& variant, const std::string& source, EShaderStage stage);
    void extractParametersFromReflection(const ShaderReflectionData& reflection);
    uint32 calculateVariantScore(const ShaderVariant& variant, const std::vector<std::string>& requestedDefines) const;
    
    std::string _vertexSource;
    std::string _fragmentSource;
    std::string _computeSource;
    
    EShaderStage _shaderType;
    std::unordered_map<ShaderVariant/*Hash of ShaderVariant*/, ShaderCache*> _variants;
    ShaderParameterCollection _parameterInterface;
    ShaderCompileOption _compileOptions;
    
    bool _isCompiled = false;
};

HS_NS_END

#endif