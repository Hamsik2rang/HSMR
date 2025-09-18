//
//  ShaderCrossCompiler.h
//  ShaderSystem
//
//  Created by Yongsik Im on 9/11/25.
//
#ifndef __HS_SHADER_CROSS_COMPILER_H__
#define __HS_SHADER_CROSS_COMPILER_H__

#include "Precompile.h"

#include "ShaderSystem//ShaderSystemDefinition.h"

#include <vector>
#include <string>

namespace HS { class SlangCompiler; }

HS_NS_BEGIN

class HS_SHADERSYSTEM_API ShaderCrossCompiler
{
public:
	ShaderCrossCompiler();
	~ShaderCrossCompiler();

	void Shutdown();

	bool CompileShader(
		const ShaderCompileInput& input,
		ShaderCompileOutput& output
	);

	//CompiledShader CompileFromFile(
	//    const std::string& filepath,
	//    const ShaderCompileOption& options
	//);

	bool IsInitialized() const { return _initialized; }

	ShaderCompileOption CreateDefaultOptions(EShaderStage stage);
	ShaderCompileOption CreateVulkanOptions(EShaderStage stage);
	ShaderCompileOption CreateMetalOptions(EShaderStage stage);

private:
	bool compileHLSLToSPV(const ShaderCompileInput& input, ShaderCompileOutput& output);

	bool crossCompileSpirv(
		const std::vector<uint32>& spirvBytecode, EShaderLanguage targetLanguage, ShaderCompileOutput& output);

	bool _initialized = false;
	Scoped<SlangCompiler> _slangCompiler;
};

HS_NS_END

#endif