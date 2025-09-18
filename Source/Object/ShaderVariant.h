#ifndef __HS_SHADER_VARIANT_H__
#define __HS_SHADER_VARIANT_H__

#include "Precompile.h"

#include "Core/Hash.h"
#include "Object/Object.h"

#include "ShaderSystem/ShaderSystemDefinition.h"

HS_NS_BEGIN

struct HS_OBJECT_API ShaderVariant
{
	EShaderLanguage language;
	std::unordered_map<std::string> macros;
	std::vector<std::string> includePaths;
};

template<>
struct Hasher<ShaderVariant>
{
	static uint32 Get(const ShaderVariant& key)
	{
		uint32 hash = static_cast<uint32>(key.language);

		std::sort(key.macros.begin(), key.macros.end());
		for (const auto& macro : key.macros)
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