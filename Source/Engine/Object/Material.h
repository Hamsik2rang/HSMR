//
//  Material.h
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#ifndef __HS_MATERIAL_H__
#define __HS_MATERIAL_H__

#include "Precompile.h"

#include "Engine/Object/Object.h"
#include "Engine/Renderer/RenderDefinition.h"

#include "MSL/BuiltInMaterialLayout.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

HS_NS_BEGIN

class Shader;
class Image;

// Common material texture types
enum class EMaterialTextureType : uint8
{
    DIFFUSE = 0,
    SPECULAR,
    NORMAL,
    EMISSION,
    AMBIENT,
    ROUGHNESS,
    METALLIC,
    AMBIENT_OCCLUSION,
    // Add more as needed
    MAX_TEXTURE_TYPES
};

class Material : public Object
{
public:
    Material() : Object(EType::MATERIAL), _shader(nullptr) {}
    ~Material() override;
    
    // Shader management
    void SetShader(Shader* shader);
    HS_FORCEINLINE Shader* GetShader() const { return _shader; }
    
    // Texture management
    void SetTexture(EMaterialTextureType type, Image* texture);
    Image* GetTexture(EMaterialTextureType type) const;
    bool HasTexture(EMaterialTextureType type) const;
    
    // Material properties
    void SetDiffuseColor(const glm::vec4& color) { _diffuseColor = color; }
    void SetSpecularColor(const glm::vec4& color) { _specularColor = color; }
    void SetEmissionColor(const glm::vec4& color) { _emissionColor = color; }
    void SetAmbientColor(const glm::vec4& color) { _ambientColor = color; }
    
    void SetShininess(float shininess) { _shininess = shininess; }
    void SetOpacity(float opacity) { _opacity = opacity; }
    void SetRoughness(float roughness) { _roughness = roughness; }
    void SetMetallic(float metallic) { _metallic = metallic; }
    
    HS_FORCEINLINE const glm::vec4& GetDiffuseColor() const { return _diffuseColor; }
    HS_FORCEINLINE const glm::vec4& GetSpecularColor() const { return _specularColor; }
    HS_FORCEINLINE const glm::vec4& GetEmissionColor() const { return _emissionColor; }
    HS_FORCEINLINE const glm::vec4& GetAmbientColor() const { return _ambientColor; }
    
    HS_FORCEINLINE float GetShininess() const { return _shininess; }
    HS_FORCEINLINE float GetOpacity() const { return _opacity; }
    HS_FORCEINLINE float GetRoughness() const { return _roughness; }
    HS_FORCEINLINE float GetMetallic() const { return _metallic; }
    
    // Shader parameter management (for custom parameters)
    void SetShaderParameter(const char* name, float value);
    void SetShaderParameter(const char* name, const glm::vec2& value);
    void SetShaderParameter(const char* name, const glm::vec3& value);
    void SetShaderParameter(const char* name, const glm::vec4& value);
    void SetShaderParameter(const char* name, const glm::mat4& value);
    
    // Two-sided rendering
    void SetTwoSided(bool twoSided) { _isTwoSided = twoSided; }
    HS_FORCEINLINE bool IsTwoSided() const { return _isTwoSided; }
    
private:
    Shader* _shader;
    
    // Textures
    std::unordered_map<EMaterialTextureType, Image*> _textures;
    
    // Basic material properties
    glm::vec4 _diffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 _specularColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 _emissionColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 _ambientColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    
    float _shininess = 32.0f;
    float _opacity = 1.0f;
    float _roughness = 0.5f;
    float _metallic = 0.0f;
    
    bool _isTwoSided = false;
    
    // Custom shader parameters
    // TODO: Implement parameter storage if needed
};


HS_NS_END


#endif
