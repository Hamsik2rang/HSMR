#include "Resource/MaterialSerializer.h"

#include "Core/HAL/FileSystem.h"
#include "Core/SystemContext.h"
#include "Resource/Image.h"
#include "Resource/Material.h"
#include "Resource/ObjectManager.h"
#include "Resource/Shader.h"
#include "ShaderSystem/ShaderSystemDefinition.h"

#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

HS_NS_BEGIN

namespace
{
json serializeVec2(const glm::vec2& value)
{
    return json::array({value.x, value.y});
}

json serializeVec3(const glm::vec3& value)
{
    return json::array({value.x, value.y, value.z});
}

json serializeVec4(const glm::vec4& value)
{
    return json::array({value.x, value.y, value.z, value.w});
}

bool deserializeVec2(const json& node, glm::vec2& outValue)
{
    if (!node.is_array() || node.size() < 2)
    {
        return false;
    }

    outValue = glm::vec2(node[0].get<float>(), node[1].get<float>());
    return true;
}

bool deserializeVec3(const json& node, glm::vec3& outValue)
{
    if (!node.is_array() || node.size() < 3)
    {
        return false;
    }

    outValue = glm::vec3(node[0].get<float>(), node[1].get<float>(), node[2].get<float>());
    return true;
}

bool deserializeVec4(const json& node, glm::vec4& outValue)
{
    if (!node.is_array() || node.size() < 4)
    {
        return false;
    }

    outValue = glm::vec4(node[0].get<float>(), node[1].get<float>(), node[2].get<float>(), node[3].get<float>());
    return true;
}

std::string getShaderPathFromName(const std::string& shaderName)
{
    if (shaderName.empty())
    {
        return "";
    }

    SystemContext* sysContext = SystemContext::Get();
    if (!sysContext || sysContext->assetDirectory.empty())
    {
        return "";
    }

    return sysContext->assetDirectory + "Shaders/" + shaderName + ".slang";
}

Scoped<Shader> loadOwnedShaderFromName(const std::string& shaderName)
{
    std::string shaderPath = getShaderPathFromName(shaderName);
    if (shaderPath.empty() || !FileSystem::Exist(shaderPath))
    {
        return nullptr;
    }

    FileHandle handle = nullptr;
    if (!FileSystem::Open(shaderPath, EFileAccess::ReadOnly, handle))
    {
        return nullptr;
    }

    const size_t fileSize = FileSystem::GetSize(handle);
    std::string sourceCode(fileSize, '\0');
    FileSystem::Read(handle, sourceCode.data(), fileSize);
    FileSystem::Close(handle);

    std::vector<std::string> includePaths;
    includePaths.push_back(FileSystem::GetDirectory(shaderPath));

    Scoped<Shader> shader = ObjectManager::LoadShaderFromSource(
        shaderName,
        sourceCode,
        EShaderStage::Vertex | EShaderStage::Fragment,
        includePaths);

    if (shader && !shader->Compile())
    {
        shader.reset();
    }

    return shader;
}

std::string getTextureTypeKey(EMaterialTextureType type)
{
    switch (type)
    {
    case EMaterialTextureType::Diffuse:          return "diffuse";
    case EMaterialTextureType::Specular:         return "specular";
    case EMaterialTextureType::Normal:           return "normal";
    case EMaterialTextureType::Emission:         return "emission";
    case EMaterialTextureType::Ambient:          return "ambient";
    case EMaterialTextureType::Roughness:        return "roughness";
    case EMaterialTextureType::Metallic:         return "metallic";
    case EMaterialTextureType::AmbientOcclusion: return "ambientOcclusion";
    case EMaterialTextureType::MaxTextureTypes:  break;
    }

    return "unknown";
}

void serializeParameterBlock(json& parameterNode, const Material& material)
{
    const MaterialParameterBlock* parameterBlock = material.GetParameterBlock();
    Shader* shader = material.GetShader();
    if (!parameterBlock || !shader || !shader->IsCompiledEx())
    {
        return;
    }

    const ShaderBufferBindingInfo* bufferInfo = shader->GetReflection().FindBuffer(parameterBlock->GetBufferName());
    if (!bufferInfo)
    {
        return;
    }

    const uint8* data = static_cast<const uint8*>(parameterBlock->GetData());
    if (!data)
    {
        return;
    }

    for (const ShaderBufferMember& member : bufferInfo->members)
    {
        const uint8* memberData = data + member.offset;
        switch (member.category)
        {
        case ShaderBufferMember::Category::Scalar:
            if (member.baseType == ShaderBufferMember::BaseType::Float && member.size >= sizeof(float))
            {
                parameterNode[member.name] = *reinterpret_cast<const float*>(memberData);
            }
            else if (member.baseType == ShaderBufferMember::BaseType::Int && member.size >= sizeof(int32))
            {
                parameterNode[member.name] = *reinterpret_cast<const int32*>(memberData);
            }
            break;
        case ShaderBufferMember::Category::Vector:
            if (member.baseType != ShaderBufferMember::BaseType::Float)
            {
                break;
            }

            if (member.rowCount == 2 && member.size >= sizeof(glm::vec2))
            {
                parameterNode[member.name] = serializeVec2(*reinterpret_cast<const glm::vec2*>(memberData));
            }
            else if (member.rowCount == 3 && member.size >= sizeof(glm::vec3))
            {
                parameterNode[member.name] = serializeVec3(*reinterpret_cast<const glm::vec3*>(memberData));
            }
            else if (member.rowCount == 4 && member.size >= sizeof(glm::vec4))
            {
                parameterNode[member.name] = serializeVec4(*reinterpret_cast<const glm::vec4*>(memberData));
            }
            break;
        default:
            break;
        }
    }
}

void deserializeParameterBlock(const json& parameterNode, Material& material)
{
    if (!parameterNode.is_object())
    {
        return;
    }

    Shader* shader = material.GetShader();
    if (!shader || !shader->IsCompiledEx())
    {
        return;
    }

    if (!material.GetParameterBlock())
    {
        material.InitializeParameterBlock();
    }

    MaterialParameterBlock* parameterBlock = material.GetParameterBlock();
    if (!parameterBlock)
    {
        return;
    }

    const ShaderBufferBindingInfo* bufferInfo = shader->GetReflection().FindBuffer(parameterBlock->GetBufferName());
    if (!bufferInfo)
    {
        return;
    }

    for (const ShaderBufferMember& member : bufferInfo->members)
    {
        auto it = parameterNode.find(member.name);
        if (it == parameterNode.end())
        {
            continue;
        }

        switch (member.category)
        {
        case ShaderBufferMember::Category::Scalar:
            if (member.baseType == ShaderBufferMember::BaseType::Float && it->is_number())
            {
                material.SetParameter(member.name, it->get<float>());
            }
            else if (member.baseType == ShaderBufferMember::BaseType::Int && it->is_number_integer())
            {
                material.SetParameter(member.name, it->get<int32>());
            }
            break;
        case ShaderBufferMember::Category::Vector:
            if (member.baseType != ShaderBufferMember::BaseType::Float)
            {
                break;
            }

            if (member.rowCount == 2)
            {
                glm::vec2 value{};
                if (deserializeVec2(*it, value))
                {
                    material.SetParameter(member.name, value);
                }
            }
            else if (member.rowCount == 3)
            {
                glm::vec3 value{};
                if (deserializeVec3(*it, value))
                {
                    material.SetParameter(member.name, value);
                }
            }
            else if (member.rowCount == 4)
            {
                glm::vec4 value{};
                if (deserializeVec4(*it, value))
                {
                    material.SetParameter(member.name, value);
                }
            }
            break;
        default:
            break;
        }
    }
}
}

bool MaterialSerializer::SaveToFile(const std::string& absolutePath, const std::string& assetRootPath, const Material& material)
{
    json root;
    root["version"] = "1.0";
    root["name"] = material.GetDisplayName().empty() ? (material.name ? material.name : "Material") : material.GetDisplayName();

    const Shader* shader = material.GetShader();
    const std::string shaderName = shader ? shader->GetShaderName() : material.GetShaderNameHint();
    if (!shaderName.empty())
    {
        root["shader"] = shaderName;
    }

    root["properties"]["diffuseColor"] = serializeVec4(material.GetDiffuseColor());
    root["properties"]["specularColor"] = serializeVec4(material.GetSpecularColor());
    root["properties"]["emissionColor"] = serializeVec4(material.GetEmissionColor());
    root["properties"]["ambientColor"] = serializeVec4(material.GetAmbientColor());
    root["properties"]["shininess"] = material.GetShininess();
    root["properties"]["opacity"] = material.GetOpacity();
    root["properties"]["roughness"] = material.GetRoughness();
    root["properties"]["metallic"] = material.GetMetallic();
    root["properties"]["twoSided"] = material.IsTwoSided();

    for (int textureIndex = 0; textureIndex < static_cast<int>(EMaterialTextureType::MaxTextureTypes); ++textureIndex)
    {
        const EMaterialTextureType textureType = static_cast<EMaterialTextureType>(textureIndex);
        if (!material.HasTextureAssetPath(textureType))
        {
            continue;
        }

        root["textures"][getTextureTypeKey(textureType)] = material.GetTextureAssetPath(textureType);
    }

    const std::vector<std::string>& defines = material.GetShaderDefines();
    if (!defines.empty())
    {
        root["shaderDefines"] = defines;
    }

    json parameterNode = json::object();
    serializeParameterBlock(parameterNode, material);
    if (!parameterNode.empty())
    {
        root["parameters"] = parameterNode;
    }

    std::ofstream file(absolutePath);
    if (!file.is_open())
    {
        HS_LOG(error, "[MaterialSerializer] Failed to open material file for writing: %s", absolutePath.c_str());
        return false;
    }

    file << root.dump(4);
    file.close();
    return true;
}

Scoped<Material> MaterialSerializer::LoadFromFile(const std::string& absolutePath, const std::string& assetRootPath)
{
    std::ifstream file(absolutePath);
    if (!file.is_open())
    {
        HS_LOG(error, "[MaterialSerializer] Failed to open material file: %s", absolutePath.c_str());
        return nullptr;
    }

    json root;
    file >> root;

    Scoped<Material> material = MakeScoped<Material>();
    material->SetSourceAssetPath(absolutePath);

    if (root.contains("name") && root["name"].is_string())
    {
        material->SetDisplayName(root["name"].get<std::string>());
    }
    else
    {
        material->SetDisplayName(FileSystem::GetFileNameWithoutExtension(absolutePath));
    }

    if (root.contains("shader") && root["shader"].is_string())
    {
        const std::string shaderName = root["shader"].get<std::string>();
        material->SetShaderNameHint(shaderName);
        Scoped<Shader> shader = loadOwnedShaderFromName(shaderName);
        if (shader)
        {
            material->SetOwnedShader(std::move(shader));
        }
    }

    const json properties = root.value("properties", json::object());
    glm::vec4 color4{};
    if (deserializeVec4(properties.value("diffuseColor", json::array()), color4)) material->SetDiffuseColor(color4);
    if (deserializeVec4(properties.value("specularColor", json::array()), color4)) material->SetSpecularColor(color4);
    if (deserializeVec4(properties.value("emissionColor", json::array()), color4)) material->SetEmissionColor(color4);
    if (deserializeVec4(properties.value("ambientColor", json::array()), color4)) material->SetAmbientColor(color4);
    if (properties.contains("shininess")) material->SetShininess(properties["shininess"].get<float>());
    if (properties.contains("opacity")) material->SetOpacity(properties["opacity"].get<float>());
    if (properties.contains("roughness")) material->SetRoughness(properties["roughness"].get<float>());
    if (properties.contains("metallic")) material->SetMetallic(properties["metallic"].get<float>());
    if (properties.contains("twoSided")) material->SetTwoSided(properties["twoSided"].get<bool>());

    const json textures = root.value("textures", json::object());
    for (int textureIndex = 0; textureIndex < static_cast<int>(EMaterialTextureType::MaxTextureTypes); ++textureIndex)
    {
        const EMaterialTextureType textureType = static_cast<EMaterialTextureType>(textureIndex);
        const std::string textureKey = getTextureTypeKey(textureType);
        if (!textures.contains(textureKey) || !textures[textureKey].is_string())
        {
            continue;
        }

        const std::string relativeTexturePath = textures[textureKey].get<std::string>();
        material->SetTextureAssetPath(textureType, relativeTexturePath);

        const std::string absoluteTexturePath = FileSystem::IsAbsolutePath(relativeTexturePath)
            ? relativeTexturePath
            : (assetRootPath + relativeTexturePath);

        Scoped<Image> image = ObjectManager::LoadImageFromFile(absoluteTexturePath, true);
        if (image)
        {
            image->SetDisplayName(FileSystem::GetFileName(absoluteTexturePath));
            image->SetSourceAssetPath(relativeTexturePath);
            material->SetOwnedTexture(textureType, std::move(image), relativeTexturePath);
        }
    }

    if (root.contains("shaderDefines") && root["shaderDefines"].is_array())
    {
        std::vector<std::string> defines;
        for (const json& defineNode : root["shaderDefines"])
        {
            if (defineNode.is_string())
            {
                defines.push_back(defineNode.get<std::string>());
            }
        }
        material->SetShaderDefines(defines);
    }

    if (root.contains("parameters"))
    {
        deserializeParameterBlock(root["parameters"], *material);
    }

    return material;
}

HS_NS_END
