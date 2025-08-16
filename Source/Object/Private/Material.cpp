#include "Object/Material.h"
//#include "Object/Shader.h"

#include "Object/Image.h"
#include "Core/Log.h"
#include "Core/Math/Vec2f.h"

HS_NS_BEGIN

Material::~Material()
{
    // Note: We don't delete textures here as they might be shared between materials
    // ObjectManager should handle texture lifecycle
    _textures.clear();
}

void Material::SetShader(RHIShader* shader)
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

void Material::SetShaderParameter(const char* name, float value)
{
    // TODO: Implement parameter storage and binding
    // This would interact with the shader system to set uniform values
}

void Material::SetShaderParameter(const char* name, const Vec2f& value)
{
    // TODO: Implement parameter storage and binding
}

void Material::SetShaderParameter(const char* name, const glm::vec3& value)
{
    // TODO: Implement parameter storage and binding
}

void Material::SetShaderParameter(const char* name, const glm::vec4& value)
{
    // TODO: Implement parameter storage and binding
}

void Material::SetShaderParameter(const char* name, const glm::mat4& value)
{
    // TODO: Implement parameter storage and binding
}

HS_NS_END
