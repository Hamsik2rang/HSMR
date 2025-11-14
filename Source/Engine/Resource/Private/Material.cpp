#include "Resource/Material.h"

#include "Resource/Shader.h"
#include "Resource/Image.h"
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
