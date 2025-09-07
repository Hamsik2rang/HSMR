#include "Object/ShaderProxy.h"
#include "Object/Shader.h"
#include "Core/Log.h"
#include "RHI/RHIContext.h"

HS_NS_BEGIN

ShaderProxy::ShaderProxy(uint64 gameObjectId)
    : ObjectProxy(EType::SHADER, gameObjectId)
    , _lastUpdateHash(0)
    , _parameterInterfaceHash(0)
{
}

ShaderProxy::~ShaderProxy()
{
    ReleaseRenderResources();
}

void ShaderProxy::UpdateFromGameObject(const Object* gameObject)
{
    const Shader* shader = static_cast<const Shader*>(gameObject);
    if (!shader || !shader->IsValid())
    {
        return;
    }

    // Generate hash for change detection
    uint64 currentHash = shader->GetObjectId();
    currentHash += static_cast<uint64>(shader->GetVariantCount());

    if (currentHash != _lastUpdateHash || _isDirty)
    {
        CreateRHIResources(shader);
        _lastUpdateHash = currentHash;
        MarkClean();
    }

    // Check if parameter interface changed
    uint64 parameterHash = shader->GetParameterInterface().GetParametersHash();
    if (parameterHash != _parameterInterfaceHash)
    {
        UpdateParameterInterface(shader);
        _parameterInterfaceHash = parameterHash;
    }
}

void ShaderProxy::ReleaseRenderResources()
{
    for (auto& pair : _compiledVariants)
    {
        ReleaseVariant(pair.second);
    }
    _compiledVariants.clear();
}

RHIShader* ShaderProxy::GetRHIShader(const std::string& variantName) const
{
    auto it = _compiledVariants.find(variantName);
    return (it != _compiledVariants.end() && it->second.isValid) ? it->second.rhiShader : nullptr;
}

RHIShader* ShaderProxy::GetRHIShaderWithDefines(const std::vector<std::string>& defines) const
{
    const CompiledShaderVariant* variant = GetBestVariant(defines);
    return variant ? variant->rhiShader : nullptr;
}

const CompiledShaderVariant* ShaderProxy::GetBestVariant(const std::vector<std::string>& defines) const
{
    if (_compiledVariants.empty())
    {
        return nullptr;
    }

    const CompiledShaderVariant* bestVariant = nullptr;
    uint32 bestScore = 0;

    for (const auto& pair : _compiledVariants)
    {
        const CompiledShaderVariant& variant = pair.second;
        if (!variant.isValid)
        {
            continue;
        }

        uint32 score = CalculateVariantScore(variant, defines);
        if (score > bestScore)
        {
            bestScore = score;
            bestVariant = &variant;
        }
    }

    return bestVariant ? bestVariant : GetDefaultVariant();
}

const CompiledShaderVariant* ShaderProxy::GetDefaultVariant() const
{
    // Try to find "default" variant first
    auto it = _compiledVariants.find("default");
    if (it != _compiledVariants.end() && it->second.isValid)
    {
        return &it->second;
    }

    // Otherwise, return the first valid variant
    for (const auto& pair : _compiledVariants)
    {
        if (pair.second.isValid)
        {
            return &pair.second;
        }
    }

    return nullptr;
}

bool ShaderProxy::HasVariant(const std::string& variantName) const
{
    auto it = _compiledVariants.find(variantName);
    return it != _compiledVariants.end() && it->second.isValid;
}

bool ShaderProxy::HasValidShaders() const
{
    for (const auto& pair : _compiledVariants)
    {
        if (pair.second.isValid)
        {
            return true;
        }
    }
    return false;
}

void ShaderProxy::CreateRHIResources(const Shader* shader)
{
    if (!shader || !shader->IsCompiled())
    {
        HS_LOG(warning, "ShaderProxy: Cannot create RHI resources from uncompiled shader");
        return;
    }

    ReleaseRenderResources();
    UpdateVariants(shader);
}

void ShaderProxy::UpdateVariants(const Shader* shader)
{
    const auto& gameVariants = shader->GetAllVariants();
    
    for (const auto& pair : gameVariants)
    {
        const std::string& variantName = pair.first;
        const ShaderVariant& gameVariant = pair.second;
        
        if (!gameVariant.isValid)
        {
            continue;
        }

        CompiledShaderVariant renderVariant = CreateVariantFromCompiled(variantName, gameVariant);
        if (renderVariant.isValid)
        {
            _compiledVariants[variantName] = renderVariant;
            HS_LOG(info, "ShaderProxy: Created RHI resources for variant: %s", variantName.c_str());
        }
        else
        {
            HS_LOG(warning, "ShaderProxy: Failed to create RHI resources for variant: %s", variantName.c_str());
        }
    }
}

void ShaderProxy::UpdateParameterInterface(const Shader* shader)
{
    _parameterInterface.CopyFrom(shader->GetParameterInterface());
}

void ShaderProxy::ReleaseVariant(CompiledShaderVariant& variant)
{
    if (variant.rhiShader)
    {
        // TODO: Release RHI shader when RHI system is available
        // RHIContext::ReleaseShader(variant.rhiShader);
        variant.rhiShader = nullptr;
    }
    variant.isValid = false;
}

CompiledShaderVariant ShaderProxy::CreateVariantFromCompiled(const std::string& variantName, 
                                                           const ShaderVariant& gameVariant)
{
    CompiledShaderVariant renderVariant;
    renderVariant.variantName = variantName;
    renderVariant.defines = gameVariant.defines;
    renderVariant.hash = gameVariant.hash;
    renderVariant.isValid = false;
    renderVariant.rhiShader = nullptr;

    // TODO: Create RHI shader when RHI system is available
    // For now, we'll just mark it as valid if the compiled shader is valid
    if (gameVariant.compiledShader.isValid)
    {
        // RHIShaderDesc shaderDesc;
        // 
        // if (!gameVariant.compiledShader.spirvBytecode.empty())
        // {
        //     shaderDesc.spirvBytecode = gameVariant.compiledShader.spirvBytecode.data();
        //     shaderDesc.spirvSize = gameVariant.compiledShader.spirvBytecode.size() * sizeof(uint32);
        // }
        // else if (!gameVariant.compiledShader.compiledSource.empty())
        // {
        //     shaderDesc.sourceCode = gameVariant.compiledShader.compiledSource.c_str();
        // }
        // else
        // {
        //     return renderVariant; // Invalid shader
        // }
        // 
        // renderVariant.rhiShader = RHIContext::CreateShader(shaderDesc);
        // renderVariant.isValid = (renderVariant.rhiShader != nullptr);

        // For now, just mark as valid
        renderVariant.isValid = true;
    }

    return renderVariant;
}

uint32 ShaderProxy::CalculateVariantScore(const CompiledShaderVariant& variant, 
                                        const std::vector<std::string>& requestedDefines) const
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