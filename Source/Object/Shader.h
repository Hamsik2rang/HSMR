#ifndef __HS_SHADER_H__
#define __HS_SHADER_H__

#include "Precompile.h"
#include "Object/Object.h"
#include "Object/ShaderParameterCollection.h"
#include "ShaderSystem/ShaderCrossCompiler.h"
#include <unordered_map>
#include <string>

HS_NS_BEGIN

struct ShaderVariant
{
    std::string name;
    std::vector<std::string> defines;
    CompiledShader compiledShader;
    bool isValid = false;
    uint64 hash = 0;
};

enum class ShaderType : uint8
{
    Surface = 0,    // Standard surface shader (vertex + fragment)
    Compute,        // Compute shader
    PostProcess,    // Post-processing effect
    UI,            // UI rendering
    Custom         // Custom shader type
};

class HS_OBJECT_API Shader : public Object
{
public:
    Shader();
    ~Shader() override;

    // Shader source management
    void SetVertexShaderSource(const std::string& source);
    void SetFragmentShaderSource(const std::string& source);
    void SetComputeShaderSource(const std::string& source);
    
    const std::string& GetVertexShaderSource() const { return _vertexSource; }
    const std::string& GetFragmentShaderSource() const { return _fragmentSource; }
    const std::string& GetComputeShaderSource() const { return _computeSource; }

    // Load from files
    bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    bool LoadComputeFromFile(const std::string& computePath);

    // Shader type
    void SetShaderType(ShaderType type) { _shaderType = type; }
    ShaderType GetShaderType() const { return _shaderType; }

    // Variant management
    void AddVariant(const std::string& variantName, const std::vector<std::string>& defines);
    void RemoveVariant(const std::string& variantName);
    bool HasVariant(const std::string& variantName) const;
    
    const ShaderVariant* GetVariant(const std::string& variantName) const;
    const std::unordered_map<std::string, ShaderVariant>& GetAllVariants() const { return _variants; }
    
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
    void SetCompilationOptions(const ShaderCompileOptions& options) { _compileOptions = options; }
    const ShaderCompileOptions& GetCompilationOptions() const { return _compileOptions; }

    // Get best matching variant based on defines
    const ShaderVariant* GetBestVariant(const std::vector<std::string>& requestedDefines) const;

    // Utility
    uint32 GetVariantCount() const { return static_cast<uint32>(_variants.size()); }
    std::vector<std::string> GetVariantNames() const;

private:
    std::string _vertexSource;
    std::string _fragmentSource;
    std::string _computeSource;
    
    ShaderType _shaderType;
    std::unordered_map<std::string, ShaderVariant> _variants;
    ShaderParameterCollection _parameterInterface;
    ShaderCompileOptions _compileOptions;
    
    bool _isCompiled = false;
    
    // Helper methods
    std::string LoadShaderSourceFromFile(const std::string& filepath);
    uint64 GenerateVariantHash(const std::string& source, const std::vector<std::string>& defines);
    bool CompileShaderVariant(ShaderVariant& variant, const std::string& source, ShaderStage stage);
    void ExtractParametersFromReflection(const ShaderReflectionData& reflection);
    uint32 CalculateVariantScore(const ShaderVariant& variant, const std::vector<std::string>& requestedDefines) const;
};

HS_NS_END

#endif