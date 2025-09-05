#ifndef __HS_MATERIAL_PROXY_H__
#define __HS_MATERIAL_PROXY_H__

#include "Precompile.h"
#include "Object/ObjectProxy.h"
#include "Object/Material.h"
#include "Core/Math/Math.h"
#include <unordered_map>

HS_NS_BEGIN

class Material;
class RHIShader;
class RHIBuffer;
class ImageProxy;

struct MaterialConstants
{
    glm::vec4 diffuseColor;
    glm::vec4 specularColor;
    glm::vec4 emissionColor;
    glm::vec4 ambientColor;
    
    float shininess;
    float opacity;
    float roughness;
    float metallic;
    
    int32 hasTextures[static_cast<int>(EMaterialTextureType::MAX_TEXTURE_TYPES)];
    int32 isTwoSided;
    int32 padding[3]; // Align to 16 bytes
};

class HS_OBJECT_API MaterialProxy : public ObjectProxy
{
public:
    explicit MaterialProxy(uint64 gameObjectId);
    ~MaterialProxy() override;

    void UpdateFromGameObject(const Object* gameObject) override;
    void ReleaseRenderResources() override;

    RHIShader* GetRHIShader() const { return _rhiShader; }
    RHIBuffer* GetConstantBuffer() const { return _constantBuffer; }
    
    ImageProxy* GetTextureProxy(EMaterialTextureType type) const;
    bool HasTexture(EMaterialTextureType type) const;
    
    const MaterialConstants& GetMaterialConstants() const { return _materialConstants; }
    
    bool HasValidShader() const { return _rhiShader != nullptr; }
    bool IsTwoSided() const { return _materialConstants.isTwoSided != 0; }

private:
    void CreateRHIResources(const Material* material);
    void UpdateMaterialConstants(const Material* material);
    void UpdateTextureReferences(const Material* material);
    void UpdateConstantBuffer();

    RHIShader* _rhiShader;
    RHIBuffer* _constantBuffer;
    
    MaterialConstants _materialConstants;
    std::unordered_map<EMaterialTextureType, uint64> _textureGameObjectIds;
    
    uint64 _lastUpdateHash;
    uint64 _constantsHash;
};

HS_NS_END

#endif