#pragma once

#include "Precompile.h"

#include <string>

HS_NS_BEGIN

class Material;

class HS_RESOURCE_API MaterialSerializer
{
public:
    static bool SaveToFile(const std::string& absolutePath, const std::string& assetRootPath, const Material& material);
    static Scoped<Material> LoadFromFile(const std::string& absolutePath, const std::string& assetRootPath);
};

HS_NS_END
