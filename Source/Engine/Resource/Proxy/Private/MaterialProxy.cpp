#include "Resource/MaterialProxy.h"
#include "Resource/Material.h"
#include "Resource/Image.h"
#include "Core/Log.h"
#include "RHI/RHIContext.h"
#include <cstring>

HS_NS_BEGIN

MaterialProxy::MaterialProxy(uint64 gameObjectId)
    : ObjectProxy(EType::MATERIAL, gameObjectId)
    , _rhiShader(nullptr)
    , _constantBuffer(nullptr)
    , _lastUpdateHash(0)
    , _constantsHash(0)
{
    // Initialize material constants to default values
    std::memset(&_materialConstants, 0, sizeof(MaterialConstants));
    
    _materialConstants.diffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    _materialConstants.specularColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    _materialConstants.emissionColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    _materialConstants.ambientColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    
    _materialConstants.shininess = 32.0f;
    _materialConstants.opacity = 1.0f;
    _materialConstants.roughness = 0.5f;
    _materialConstants.metallic = 0.0f;
    
    _materialConstants.isTwoSided = 0;
}

MaterialProxy::~MaterialProxy()
{
    ReleaseRenderResources();
}

void MaterialProxy::UpdateFromGameObject(const Object* gameObject)
{
    const Material* material = static_cast<const Material*>(gameObject);
    if (!material || !material->IsValid())
    {
        return;
    }

    // Create a hash for the material state
    uint64 shaderHash = reinterpret_cast<uint64>(material->GetShader());
    uint64 materialHash = shaderHash;

    // Include material properties in hash
    const auto& diffuse = material->GetDiffuseColor();
    materialHash += static_cast<uint64>(diffuse.x * 1000) + 
                   static_cast<uint64>(diffuse.y * 1000) + 
                   static_cast<uint64>(diffuse.z * 1000) + 
                   static_cast<uint64>(diffuse.w * 1000);

    if (materialHash != _lastUpdateHash || _isDirty)
    {
        CreateRHIResources(material);
        UpdateMaterialConstants(material);
        UpdateTextureReferences(material);
        UpdateConstantBuffer();
        
        _lastUpdateHash = materialHash;
        MarkClean();
    }
    else
    {
        // Check if only constants changed
        uint64 constantsHash = 0;
        const auto& props = material->GetDiffuseColor();
        constantsHash += static_cast<uint64>(props.x * 1000);
        
        if (constantsHash != _constantsHash)
        {
            UpdateMaterialConstants(material);
            UpdateConstantBuffer();
            _constantsHash = constantsHash;
        }
    }
}

void MaterialProxy::ReleaseRenderResources()
{
    if (_constantBuffer)
    {
        // TODO: Release RHI constant buffer when RHI is implemented
        _constantBuffer = nullptr;
    }

    // Note: We don't own the shader, so we don't release it
    _rhiShader = nullptr;
    
    _textureGameObjectIds.clear();
}

void MaterialProxy::CreateRHIResources(const Material* material)
{
    if (!material)
    {
        HS_LOG(warning, "MaterialProxy: Cannot create RHI resources from null material");
        return;
    }

    // Set shader reference through ShaderProxy (we don't own this)
    // TODO: Get ShaderProxy from RenderScene and retrieve RHIShader
    // _rhiShader = shaderProxy->GetRHIShader();
    _rhiShader = nullptr;

    // Create constant buffer
    // TODO: Create RHI constant buffer when RHI system is available
    // RHIBufferDesc constantBufferDesc;
    // constantBufferDesc.usage = ERHIBufferUsage::ConstantBuffer;
    // constantBufferDesc.size = sizeof(MaterialConstants);
    // constantBufferDesc.initialData = &_materialConstants;
    // 
    // _constantBuffer = RHIContext::CreateBuffer(constantBufferDesc);

    HS_LOG(info, "MaterialProxy: Created RHI resources for material");
}

void MaterialProxy::UpdateMaterialConstants(const Material* material)
{
    if (!material)
    {
        return;
    }

    // Copy material properties
    _materialConstants.diffuseColor = material->GetDiffuseColor();
    _materialConstants.specularColor = material->GetSpecularColor();
    _materialConstants.emissionColor = material->GetEmissionColor();
    _materialConstants.ambientColor = material->GetAmbientColor();
    
    _materialConstants.shininess = material->GetShininess();
    _materialConstants.opacity = material->GetOpacity();
    _materialConstants.roughness = material->GetRoughness();
    _materialConstants.metallic = material->GetMetallic();
    
    _materialConstants.isTwoSided = material->IsTwoSided() ? 1 : 0;

    // Update texture availability flags
    for (int i = 0; i < static_cast<int>(EMaterialTextureType::MAX_TEXTURE_TYPES); ++i)
    {
        EMaterialTextureType textureType = static_cast<EMaterialTextureType>(i);
        _materialConstants.hasTextures[i] = material->HasTexture(textureType) ? 1 : 0;
    }
}

void MaterialProxy::UpdateTextureReferences(const Material* material)
{
    if (!material)
    {
        return;
    }

    _textureGameObjectIds.clear();

    // Store texture references by their game object IDs
    for (int i = 0; i < static_cast<int>(EMaterialTextureType::MAX_TEXTURE_TYPES); ++i)
    {
        EMaterialTextureType textureType = static_cast<EMaterialTextureType>(i);
        Image* texture = material->GetTexture(textureType);
        
        if (texture && texture->IsValid())
        {
            _textureGameObjectIds[textureType] = texture->GetObjectId();
        }
    }
}

void MaterialProxy::UpdateConstantBuffer()
{
    if (!_constantBuffer)
    {
        return;
    }

    // TODO: Update constant buffer data when RHI system is available
    // RHIContext::UpdateBufferData(_constantBuffer, &_materialConstants, sizeof(MaterialConstants));
}

ImageProxy* MaterialProxy::GetTextureProxy(EMaterialTextureType type) const
{
    auto it = _textureGameObjectIds.find(type);
    if (it == _textureGameObjectIds.end())
    {
        return nullptr;
    }

    // TODO: Get ImageProxy from render thread object manager using the game object ID
    // return RenderThreadObjectManager::GetImageProxy(it->second);
    
    // For now, return nullptr until render thread object manager is implemented
    return nullptr;
}

bool MaterialProxy::HasTexture(EMaterialTextureType type) const
{
    return _textureGameObjectIds.find(type) != _textureGameObjectIds.end();
}

HS_NS_END