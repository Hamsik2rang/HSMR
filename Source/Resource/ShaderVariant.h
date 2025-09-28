#ifndef __HS_SHADER_VARIANT_H__
#define __HS_SHADER_VARIANT_H__

#include "Precompile.h"

#include "Core/Hash.h"
#include "Resource/Object.h"

#include "ShaderSystem/ShaderSystemDefinition.h"

HS_NS_BEGIN

struct HS_RESOURCE_API ShaderVariant
{
	EShaderLanguage language;
	std::unordered_map<std::string, std::string> macros;
	std::vector<std::string> includePaths;
};

template<>
struct Hasher<ShaderVariant>
{
	static uint32 Get(const ShaderVariant& key)
	{
		uint32 hash = static_cast<uint32>(key.language);

		// Create sorted vector of macro pairs for consistent hashing
		std::vector<std::pair<std::string, std::string>> sortedMacros(key.macros.begin(), key.macros.end());
		std::sort(sortedMacros.begin(), sortedMacros.end());
		
		for (const auto& macro : sortedMacros)
		{
			hash = HashCombine(hash, StringHash(macro.first), StringHash(macro.second));
		}
		
		for (const auto& path : key.includePaths)
		{
			hash = HashCombine(hash, StringHash(path));
		}

		return hash;
	}
};



HS_NS_END

#endif