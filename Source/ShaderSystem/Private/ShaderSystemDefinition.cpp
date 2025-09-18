#include "ShaderSystem/ShaderSystemDefinition.h"

HS_NS_BEGIN





namespace ShaderSystemUtil
{
	std::string GetShaderStageString(EShaderStage stage)
	{
		switch (stage)
		{
		case EShaderStage::VERTEX:
			return "Vertex";
		case EShaderStage::FRAGMENT:
			return "Fragment";
		case EShaderStage::COMPUTE:
			return "Compute";
		case EShaderStage::GEOMETRY:
			return "Geometry";
		case EShaderStage::DOMAIN:
			return "Domain";
		case EShaderStage::HULL:
			return "HULL";
		default:
			return "Unknown";
		}
	}
}


std::string GetShaderStageLanguage(EShaderLanguage language)
{
	switch (language)
	{
	case EShaderLanguage::SPIRV:
		return "SPIR-V";
	case EShaderLanguage::MSL:
		return "MSL";
	case EShaderLanguage::HLSL:
		return "HLSL";
	default:
		return "Unknown";
	}
}

HS_NS_END