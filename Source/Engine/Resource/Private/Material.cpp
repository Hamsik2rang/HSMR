#include "Resource/Material.h"

#include "Resource/Shader.h"
#include "Resource/Image.h"
#include "Core/Log.h"

#include <algorithm>

HS_NS_BEGIN

Material::~Material()
{
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

// =======================
// NEW: Reflection-based parameter block
// =======================

void Material::InitializeParameterBlock()
{
    if (!_shader || !_shader->IsCompiledEx())
    {
        HS_LOG(warning, "[Material] Cannot initialize parameter block: shader not compiled");
        return;
    }

    const ShaderReflectionDataEx& reflection = _shader->GetReflection();

    // Find the first buffer that is NOT named perView, perDraw, or perFrame
    // This will be the "per-material" buffer
    for (const auto& buf : reflection.bufferBindings)
    {
        if (buf.name == "perView" || buf.name == "perDraw" || buf.name == "perFrame" ||
            buf.name == "PerView" || buf.name == "PerDraw" || buf.name == "PerFrame")
        {
            continue;
        }

        _parameterBlock.Initialize(buf);
        HS_LOG(info, "[Material] Parameter block initialized from buffer '%s' (%u bytes)",
               buf.name.c_str(), buf.totalSize);
        return;
    }

    HS_LOG(info, "[Material] No per-material buffer found in shader reflection");
}

void Material::SetParameter(const std::string& name, float value)
{
    if (_parameterBlock.IsInitialized())
    {
        _parameterBlock.SetFloat(name, value);
    }
}

void Material::SetParameter(const std::string& name, const glm::vec2& value)
{
    if (_parameterBlock.IsInitialized())
    {
        _parameterBlock.SetVec2(name, value);
    }
}

void Material::SetParameter(const std::string& name, const glm::vec3& value)
{
    if (_parameterBlock.IsInitialized())
    {
        _parameterBlock.SetVec3(name, value);
    }
}

void Material::SetParameter(const std::string& name, const glm::vec4& value)
{
    if (_parameterBlock.IsInitialized())
    {
        _parameterBlock.SetVec4(name, value);
    }
}

void Material::SetParameter(const std::string& name, const glm::mat4& value)
{
    if (_parameterBlock.IsInitialized())
    {
        _parameterBlock.SetMat4(name, value);
    }
}

void Material::SetParameter(const std::string& name, int32 value)
{
    if (_parameterBlock.IsInitialized())
    {
        _parameterBlock.SetInt(name, value);
    }
}

HS_NS_END
