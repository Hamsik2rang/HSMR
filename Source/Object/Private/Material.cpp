#include "Object/Material.h"
#include "Object/Shader.h"
#include "Object/Image.h"
#include "Core/Log.h"
#include <algorithm>

HS_NS_BEGIN

Material::~Material()
{
    // Note: We don't delete textures here as they might be shared between materials
    // ObjectManager should handle texture lifecycle
    _textures.clear();
}

void Material::SetShader(Shader* shader)
{
    _shader = shader;
    
    // Update parameter interface when shader changes
    if (_shader && _shader->IsCompiled())
    {
        _shaderParameters.CopyFrom(_shader->GetParameterInterface());
    }
}

void Material::SetTexture(EMaterialTextureType type, Image* texture)
{
    if (type >= EMaterialTextureType::MAX_TEXTURE_TYPES)
    {
        HS_LOG(error, "Invalid texture type: %d", static_cast<int>(type));
        return;
    }
    
    _textures[type] = texture;
}

Image* Material::GetTexture(EMaterialTextureType type) const
{
    auto it = _textures.find(type);
    if (it != _textures.end())
    {
        return it->second;
    }
    return nullptr;
}

bool Material::HasTexture(EMaterialTextureType type) const
{
    return _textures.find(type) != _textures.end() && _textures.at(type) != nullptr;
}

// Dynamic shader parameter setters
void Material::SetShaderParameter(const std::string& name, float value)
{
    _shaderParameters.SetParameter(name, value);
}

void Material::SetShaderParameter(const std::string& name, const glm::vec2& value)
{
    _shaderParameters.SetParameter(name, value);
}

void Material::SetShaderParameter(const std::string& name, const glm::vec3& value)
{
    _shaderParameters.SetParameter(name, value);
}

void Material::SetShaderParameter(const std::string& name, const glm::vec4& value)
{
    _shaderParameters.SetParameter(name, value);
}

void Material::SetShaderParameter(const std::string& name, int32 value)
{
    _shaderParameters.SetParameter(name, value);
}

void Material::SetShaderParameter(const std::string& name, const glm::ivec2& value)
{
    _shaderParameters.SetParameter(name, value);
}

void Material::SetShaderParameter(const std::string& name, const glm::ivec3& value)
{
    _shaderParameters.SetParameter(name, value);
}

void Material::SetShaderParameter(const std::string& name, const glm::ivec4& value)
{
    _shaderParameters.SetParameter(name, value);
}

void Material::SetShaderParameter(const std::string& name, const glm::mat3& value)
{
    _shaderParameters.SetParameter(name, value);
}

void Material::SetShaderParameter(const std::string& name, const glm::mat4& value)
{
    _shaderParameters.SetParameter(name, value);
}

void Material::SetShaderParameter(const std::string& name, bool value)
{
    _shaderParameters.SetParameter(name, value);
}

void Material::SetShaderParameter(const std::string& name, uint64 textureHandle)
{
    _shaderParameters.SetParameter(name, textureHandle);
}

bool Material::HasShaderParameter(const std::string& name) const
{
    return _shaderParameters.HasParameter(name);
}

// Shader variant support
void Material::AddShaderDefine(const std::string& define)
{
    auto it = std::find(_shaderDefines.begin(), _shaderDefines.end(), define);
    if (it == _shaderDefines.end())
    {
        _shaderDefines.push_back(define);
    }
}

void Material::RemoveShaderDefine(const std::string& define)
{
    auto it = std::find(_shaderDefines.begin(), _shaderDefines.end(), define);
    if (it != _shaderDefines.end())
    {
        _shaderDefines.erase(it);
    }
}

HS_NS_END
