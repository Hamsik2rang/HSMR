#ifndef __HS_SHADER_PROXY_H__
#define __HS_SHADER_PROXY_H__

#include "Precompile.h"
#include "Resource/ObjectProxy.h"
#include "Resource/ShaderParameterCollection.h"
#include "RHI/RHIDefinition.h"
#include <unordered_map>
#include <string>

HS_NS_BEGIN

class Shader;
class RHIShader;
class RHIBuffer;

struct CompiledShaderVariant
{
    RHIShader* rhiShader;
    std::string variantName;
    std::vector<std::string> defines;
    uint64 hash;
    bool isValid;
};

class HS_RESOURCE_API ShaderProxy : public ObjectProxy
{
public:
    explicit ShaderProxy(uint64 gameObjectId);
    ~ShaderProxy() override;

    void UpdateFromGameObject(const Object* gameObject) override;
    void ReleaseRenderResources() override;

    // Get RHI shader for specific variant
    RHIShader* GetRHIShader(const std::string& variantName = "default") const;
    RHIShader* GetRHIShaderWithDefines(const std::vector<std::string>& defines) const;

    // Get best matching variant
    const CompiledShaderVariant* GetBestVariant(const std::vector<std::string>& defines) const;
    const CompiledShaderVariant* GetDefaultVariant() const;

    // Parameter interface
    const ShaderParameterCollection& GetParameterInterface() const { return _parameterInterface; }

    // Variant management
    bool HasVariant(const std::string& variantName) const;
    const std::unordered_map<std::string, CompiledShaderVariant>& GetAllVariants() const { return _compiledVariants; }

    // Utility
    uint32 GetVariantCount() const { return static_cast<uint32>(_compiledVariants.size()); }
    bool HasValidShaders() const;

private:
    std::unordered_map<std::string, CompiledShaderVariant> _compiledVariants;
    ShaderParameterCollection _parameterInterface;
    
    uint64 _lastUpdateHash;
    uint64 _parameterInterfaceHash;

    void CreateRHIResources(const Shader* shader);
    void UpdateVariants(const Shader* shader);
    void UpdateParameterInterface(const Shader* shader);
    void ReleaseVariant(CompiledShaderVariant& variant);
    
    CompiledShaderVariant CreateVariantFromCompiled(const std::string& variantName, 
                                                   const struct ShaderVariant& gameVariant);
    
    uint32 CalculateVariantScore(const CompiledShaderVariant& variant, 
                               const std::vector<std::string>& requestedDefines) const;
};

HS_NS_END

#endif