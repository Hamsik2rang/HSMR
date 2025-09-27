#include "ShaderSystem/ShaderCrossCompiler.h"

#include "ShaderSystem/Slang/SlangCompiler.h"
#include "ShaderSystem/Spirv/SpirvCrossHelper.h"

#include "Core/Log.h"
#include "RHI/RHIDefinition.h"

#include <fstream>
#include <sstream>

HS_NS_BEGIN

ShaderCrossCompiler::ShaderCrossCompiler()
{
	_slangCompiler = MakeScoped<SlangCompiler>();
}

ShaderCrossCompiler::~ShaderCrossCompiler()
{
	Shutdown();
}

void ShaderCrossCompiler::Shutdown()
{

}

bool ShaderCrossCompiler::CompileShader(
	const ShaderCompileInput& input,
	ShaderCompileOutput& output)
{
	if (!_initialized)
	{
		HS_LOG(error, "ShaderCrossCompiler not initialized");
		return false;
	}

	HS_LOG(info, "Compiling shader: %s", input.shaderName.c_str());

	ShaderCompileOutput spvOutput{};
	if (compileHLSLToSPV(input, spvOutput))
	{
		HS_LOG(crash, "Fail to compile HLSL to SPIR-V: %s", input.shaderName.c_str());
		return false;
	}

	std::stringstream ss;
	ss << input.shaderName << "_" << static_cast<uint32>(input.option.stage) << "_" << static_cast<uint32>(input.option.targetLanguage);

	HS_LOG(info, "Successfully compiled shader: %s", input.shaderName.c_str());
	return true;
}

bool ShaderCrossCompiler::compileHLSLToSPV(
	const ShaderCompileInput& input,
	ShaderCompileOutput& output)
{
	SlangCompiler slangCompiler;
	if (!slangCompiler.Initialize())
	{
		HS_LOG(crash, "Failed to initialize Slang compiler");
		return false;
	}

	if (slangCompiler.CompileToSpirv(input, output))
	{
		HS_LOG(crash, "Slang compilation to SPIR-V failed: %s", input.shaderName.c_str());
		return false;
	}

	return true;
}

bool ShaderCrossCompiler::crossCompileSpirv(const std::vector<uint32>& spirvBytecode, EShaderLanguage targetLanguage, ShaderCompileOutput& output)
{
	if (targetLanguage == EShaderLanguage::SPIRV)
	{
		return true;
	}
	std::string outputCode{};
	switch (targetLanguage)
	{
	case EShaderLanguage::MSL:
		outputCode = SpirvCrossHelper::CrossCompileToMSL(spirvBytecode);
		break;

	default:
		HS_LOG(error, "Unsupported target shader language: %s", ShaderSystemUtil::GetShaderLanguageString(targetLanguage).c_str());
		return false;
	}

	if (0 == output.sourceCodeLen)
	{
		HS_LOG(error, "Cross-compilation resulted in empty source code");
		return false;
	}

	return true;
}

ShaderCompileOption ShaderCrossCompiler::CreateDefaultOptions(EShaderStage stage)
{
	ShaderCompileOption options;
	options.stage = stage;
	options.targetLanguage = EShaderLanguage::SPIRV;
	options.entryPoint = "main";
	options.debugInfoLevel = ShaderDebugInfoLevel::NONE;
	options.optimizationLevel = ShaderOptimizationLevel::MAXIMAL;
	return options;
}

ShaderCompileOption ShaderCrossCompiler::CreateVulkanOptions(EShaderStage stage)
{
	ShaderCompileOption options = CreateDefaultOptions(stage);
	options.targetLanguage = EShaderLanguage::SPIRV;
	return options;
}

ShaderCompileOption ShaderCrossCompiler::CreateMetalOptions(EShaderStage stage)
{
	ShaderCompileOption options = CreateDefaultOptions(stage);
	options.targetLanguage = EShaderLanguage::MSL;
	return options;
}

HS_NS_END