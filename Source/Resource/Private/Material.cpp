#include "Resource/Material.h"

#include "Resource/Shader.h"
#include "Resource/Image.h"
#include "ShaderSystem/ShaderBindingUtils.h"
#include "Core/Log.h"

#include <algorithm>

HS_NS_BEGIN

Material::~Material()
{
    _textures.clear();
    _ownedTextures.clear();
    _ownedShader.reset();
}

void Material::SetShader(Shader* shader)
{
    _ownedShader.reset();
    _shader = shader;
    _shaderNameHint = shader ? shader->GetShaderName() : std::string();
    markResourceDirty();
}

void Material::SetOwnedShader(Scoped<Shader>&& shader)
{
    _ownedShader = std::move(shader);
    _shader = _ownedShader.get();
    _shaderNameHint = _shader ? _shader->GetShaderName() : std::string();
    markResourceDirty();
}

void Material::SetTexture(EMaterialTextureType type, Image* texture)
{
    if (type >= EMaterialTextureType::MaxTextureTypes)
    {
        HS_LOG(error, "Invalid texture type: %d", static_cast<int>(type));
        return;
    }

    _ownedTextures.erase(type);
    _textures[type] = texture;
    markResourceDirty();
}

void Material::SetOwnedTexture(EMaterialTextureType type, Scoped<Image>&& texture, const std::string& sourcePath)
{
    if (type >= EMaterialTextureType::MaxTextureTypes)
    {
        HS_LOG(error, "Invalid texture type: %d", static_cast<int>(type));
        return;
    }

    Image* texturePtr = texture.get();
    if (texturePtr)
    {
        if (!sourcePath.empty())
        {
            texturePtr->SetSourceAssetPath(sourcePath);
        }
        _textures[type] = texturePtr;
        _ownedTextures[type] = std::move(texture);
        if (!sourcePath.empty())
        {
            _textureAssetPaths[type] = sourcePath;
        }
        markResourceDirty();
    }
    else
    {
        _textures[type] = nullptr;
        _ownedTextures.erase(type);
        _textureAssetPaths.erase(type);
        markResourceDirty();
    }
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

void Material::SetTextureAssetPath(EMaterialTextureType type, const std::string& path)
{
    if (path.empty())
    {
        _textureAssetPaths.erase(type);
        markResourceDirty();
        return;
    }

    _textureAssetPaths[type] = path;
    markResourceDirty();
}

const std::string& Material::GetTextureAssetPath(EMaterialTextureType type) const
{
    static const std::string emptyPath;
    auto it = _textureAssetPaths.find(type);
    if (it != _textureAssetPaths.end())
    {
        return it->second;
    }
    return emptyPath;
}

bool Material::HasTextureAssetPath(EMaterialTextureType type) const
{
    auto it = _textureAssetPaths.find(type);
    return it != _textureAssetPaths.end() && !it->second.empty();
}

// Shader variant support
void Material::AddShaderDefine(const std::string& define)
{
    auto it = std::find(_shaderDefines.begin(), _shaderDefines.end(), define);
    if (it == _shaderDefines.end())
    {
        _shaderDefines.push_back(define);
        markResourceDirty();
    }
}

void Material::RemoveShaderDefine(const std::string& define)
{
    auto it = std::find(_shaderDefines.begin(), _shaderDefines.end(), define);
    if (it != _shaderDefines.end())
    {
        _shaderDefines.erase(it);
        markResourceDirty();
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

    for (const auto& buf : reflection.bufferBindings)
    {
        if (IsReservedGlobalBufferName(buf.name))
        {
            continue;
        }

        auto hasMember = [&buf](const char* memberName) -> bool
        {
            return std::any_of(
                buf.members.begin(),
                buf.members.end(),
                [memberName](const ShaderBufferMember& member)
                {
                    return member.name == memberName;
                });
        };

        _parameterBlock.Initialize(buf);
        if (hasMember("diffuseColor")) _parameterBlock.SetVec4("diffuseColor", _diffuseColor);
        if (hasMember("specularColor")) _parameterBlock.SetVec4("specularColor", _specularColor);
        if (hasMember("emissionColor")) _parameterBlock.SetVec4("emissionColor", _emissionColor);
        if (hasMember("ambientColor")) _parameterBlock.SetVec4("ambientColor", _ambientColor);
        if (hasMember("shininess")) _parameterBlock.SetFloat("shininess", _shininess);
        if (hasMember("opacity")) _parameterBlock.SetFloat("opacity", _opacity);
        if (hasMember("roughness")) _parameterBlock.SetFloat("roughness", _roughness);
        if (hasMember("metallic")) _parameterBlock.SetFloat("metallic", _metallic);
        if (hasMember("twoSided")) _parameterBlock.SetInt("twoSided", _isTwoSided ? 1 : 0);
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
        markResourceDirty();
    }
}

void Material::SetParameter(const std::string& name, const glm::vec2& value)
{
    if (_parameterBlock.IsInitialized())
    {
        _parameterBlock.SetVec2(name, value);
        markResourceDirty();
    }
}

void Material::SetParameter(const std::string& name, const glm::vec3& value)
{
    if (_parameterBlock.IsInitialized())
    {
        _parameterBlock.SetVec3(name, value);
        markResourceDirty();
    }
}

void Material::SetParameter(const std::string& name, const glm::vec4& value)
{
    if (_parameterBlock.IsInitialized())
    {
        _parameterBlock.SetVec4(name, value);
        markResourceDirty();
    }
}

void Material::SetParameter(const std::string& name, const glm::mat4& value)
{
    if (_parameterBlock.IsInitialized())
    {
        _parameterBlock.SetMat4(name, value);
        markResourceDirty();
    }
}

void Material::SetParameter(const std::string& name, int32 value)
{
    if (_parameterBlock.IsInitialized())
    {
        _parameterBlock.SetInt(name, value);
        markResourceDirty();
    }
}

HS_NS_END
