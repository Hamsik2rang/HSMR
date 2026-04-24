#ifndef __HS_MATERIAL_TEXTURE_BINDING_H__
#define __HS_MATERIAL_TEXTURE_BINDING_H__

#include "Precompile.h"
#include "Resource/Material.h"

#include <string>
#include <cctype>

HS_NS_BEGIN

inline std::string ToLowerTextureBindingName(const std::string& name)
{
    std::string lowerName = name;
    for (char& c : lowerName)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lowerName;
}

inline EMaterialTextureType MapTextureBindingNameToMaterialTextureType(const std::string& name)
{
    const std::string lowerName = ToLowerTextureBindingName(name);

    if (lowerName.find("albedo") != std::string::npos ||
        lowerName.find("diffuse") != std::string::npos ||
        lowerName.find("basecolor") != std::string::npos ||
        lowerName.find("base_color") != std::string::npos)
    {
        return EMaterialTextureType::Diffuse;
    }

    if (lowerName.find("normal") != std::string::npos)
    {
        return EMaterialTextureType::Normal;
    }

    if (lowerName.find("metallic") != std::string::npos ||
        lowerName.find("metalness") != std::string::npos)
    {
        return EMaterialTextureType::Metallic;
    }

    if (lowerName.find("roughness") != std::string::npos)
    {
        return EMaterialTextureType::Roughness;
    }

    if (lowerName.find("emission") != std::string::npos ||
        lowerName.find("emissive") != std::string::npos)
    {
        return EMaterialTextureType::Emission;
    }

    if (lowerName.find("ao") != std::string::npos ||
        lowerName.find("occlusion") != std::string::npos ||
        lowerName.find("ambient") != std::string::npos)
    {
        return EMaterialTextureType::AmbientOcclusion;
    }

    if (lowerName.find("specular") != std::string::npos)
    {
        return EMaterialTextureType::Specular;
    }

    return EMaterialTextureType::Diffuse;
}

HS_NS_END

#endif
