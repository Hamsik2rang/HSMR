//
//  Material.h
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#ifndef __HS_MATERIAL_H__
#define __HS_MATERIAL_H__

#include "Precompile.h"

#include "Resource/Object.h"
#include "Resource/ShaderParameterCollection.h"

#include "Core/Math/Math.h"
#include <unordered_map>
#include <string>

HS_NS_BEGIN

class Shader;
class Image;

// Common material texture types
enum class HS_RESOURCE_API EMaterialTextureType : uint8
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

class HS_RESOURCE_API Material : public Object
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
    
    // Shader parameter management (dynamic parameters)
    void SetShaderParameter(const std::string& name, float value);
    void SetShaderParameter(const std::string& name, const glm::vec2& value);
    void SetShaderParameter(const std::string& name, const glm::vec3& value);
    void SetShaderParameter(const std::string& name, const glm::vec4& value);
    void SetShaderParameter(const std::string& name, int32 value);
    void SetShaderParameter(const std::string& name, const glm::ivec2& value);
    void SetShaderParameter(const std::string& name, const glm::ivec3& value);
    void SetShaderParameter(const std::string& name, const glm::ivec4& value);
    void SetShaderParameter(const std::string& name, const glm::mat3& value);
    void SetShaderParameter(const std::string& name, const glm::mat4& value);
    void SetShaderParameter(const std::string& name, bool value);
    void SetShaderParameter(const std::string& name, uint64 textureHandle);
    
    // Get shader parameter
    template<typename T>
    bool GetShaderParameter(const std::string& name, T& outValue) const;
    bool HasShaderParameter(const std::string& name) const;
    
    // Get all shader parameters
    const ShaderParameterCollection& GetShaderParameters() const { return _shaderParameters; }
    ShaderParameterCollection& GetShaderParameters() { return _shaderParameters; }
    
    // Two-sided rendering
    void SetTwoSided(bool twoSided) { _isTwoSided = twoSided; }
    HS_FORCEINLINE bool IsTwoSided() const { return _isTwoSided; }
    
    // Shader variant support
    void SetShaderDefines(const std::vector<std::string>& defines) { _shaderDefines = defines; }
    const std::vector<std::string>& GetShaderDefines() const { return _shaderDefines; }
    void AddShaderDefine(const std::string& define);
    void RemoveShaderDefine(const std::string& define);
    
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
    
    // Dynamic shader parameters
    ShaderParameterCollection _shaderParameters;
    std::vector<std::string> _shaderDefines;
};

// Template implementation for GetShaderParameter
template<typename T>
bool Material::GetShaderParameter(const std::string& name, T& outValue) const
{
    return _shaderParameters.GetParameter(name, outValue);
}

HS_NS_END


#endif
