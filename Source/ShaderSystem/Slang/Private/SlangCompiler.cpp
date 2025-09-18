#include "ShaderSystem/Slang/SlangCompiler.h"
#include "Core/Log.h"

#ifdef HS_SLANG_AVAILABLE
#include <slang/slang-com-helper.h>
#endif

#include <fstream>
#include <sstream>

HS_NS_BEGIN

static SlangDebugInfoLevel ToDebugInfoLevel(ShaderDebugInfoLevel level)
{
	switch (level)
	{
	case ShaderDebugInfoLevel::NONE:
		return SLANG_DEBUG_INFO_LEVEL_NONE;
	case ShaderDebugInfoLevel::MINIMAL:
		return SLANG_DEBUG_INFO_LEVEL_MINIMAL;
	case ShaderDebugInfoLevel::STANDARD:
		return SLANG_DEBUG_INFO_LEVEL_STANDARD;
	case ShaderDebugInfoLevel::MAXIMAL:
		return SLANG_DEBUG_INFO_LEVEL_MAXIMAL;
	default:
		return SLANG_DEBUG_INFO_LEVEL_NONE;
	}
}

static SlangOptimizationLevel ToOptimizationLevel(ShaderOptimizationLevel level)
{
	switch (level)
	{
	case ShaderOptimizationLevel::NONE:
		return SLANG_OPTIMIZATION_LEVEL_NONE;
	case ShaderOptimizationLevel::STANDARD:
		return SLANG_OPTIMIZATION_LEVEL_DEFAULT;
	case ShaderOptimizationLevel::HIGH:
		return SLANG_OPTIMIZATION_LEVEL_HIGH;
	case ShaderOptimizationLevel::MAXIMAL:
		return SLANG_OPTIMIZATION_LEVEL_MAXIMAL;
	default:
		return SLANG_OPTIMIZATION_LEVEL_NONE;
	}
}

SlangCompiler::SlangCompiler()
{
	Initialize();
}

SlangCompiler::~SlangCompiler()
{
	Shutdown();
}

bool SlangCompiler::Initialize()
{
	if (_globalSession)
	{
		return true;
	}

	SlangResult result = slang::createGlobalSession(_globalSession.writeRef());
	if (SLANG_FAILED(result))
	{
		HS_LOG(crash, "Failed to create Slang global session: %d", static_cast<int>(result));
		return false;
	}
}

void SlangCompiler::Shutdown()
{
	_globalSession.setNull();
}

bool SlangCompiler::CompileToSpirv(const ShaderCompileInput& input, ShaderCompileOutput& output)
{
	slang::SessionDesc sessionDesc{};
	slang::TargetDesc targetDesc{};

	uint32 sessionHash = 0;

	targetDesc.format = SLANG_SPIRV;
	targetDesc.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_NONE;
	targetDesc.floatingPointMode = SLANG_FLOATING_POINT_MODE_DEFAULT;
	targetDesc.profile = _globalSession->findProfile("spirv_1_6");

	slang::CompilerOptionEntry emitViaGLSL{};
	emitViaGLSL.name = slang::CompilerOptionName::EmitSpirvViaGLSL;
	emitViaGLSL.value.intValue0 = 1;

	sessionHash = HashCombine(static_cast<uint32>(emitViaGLSL.name), static_cast<uint32>(emitViaGLSL.value.intValue0), sessionHash);

	slang::CompilerOptionEntry debugInfoLevel{};
	debugInfoLevel.name = slang::CompilerOptionName::DebugInformation;
	debugInfoLevel.value.intValue0 = ToDebugInfoLevel(input.option.debugInfoLevel);
	
	sessionHash = HashCombine(static_cast<uint32>(debugInfoLevel.name), static_cast<uint32>(debugInfoLevel.value.intValue0), sessionHash);

	slang::CompilerOptionEntry optimizationLevel{};
	optimizationLevel.name = slang::CompilerOptionName::Optimization;
	optimizationLevel.value.intValue0 = ToOptimizationLevel(input.option.optimizationLevel);

	sessionHash = HashCombine(static_cast<uint32>(optimizationLevel.name), static_cast<uint32>(optimizationLevel.value.intValue0), sessionHash);

	slang::CompilerOptionEntry forceDXLayout{};
	forceDXLayout.name = slang::CompilerOptionName::ForceDXLayout;
	forceDXLayout.value.intValue0 = 1;

	sessionHash = HashCombine(static_cast<uint32>(forceDXLayout.name), static_cast<uint32>(forceDXLayout.value.intValue0), sessionHash);

	slang::CompilerOptionEntry optionEntries[] = {
		emitViaGLSL,
		debugInfoLevel,
		optimizationLevel,
		forceDXLayout,
	};

	targetDesc.compilerOptionEntries = optionEntries;
	targetDesc.compilerOptionEntryCount = sizeof(optionEntries) / sizeof(optionEntries[0]);

	sessionDesc.targetCount = 1;
	sessionDesc.targets = &targetDesc;

	std::vector<slang::PreprocessorMacroDesc> macros(input.option.macros.size());
	size_t predefineCount = input.option.macros.size();
	for (size_t i = 0; i < predefineCount; i++)
	{
		slang::PreprocessorMacroDesc desc{};
		desc.name = input.option.macros[i].name;
		desc.value = input.option.macros[i].value;
		macros.push_back(desc);

		sessionHash = HashCombine(StringHash(desc.name), StringHash(desc.value), sessionHash);
	}

	std::vector<const char*> includePaths;
	size_t includePathCount = input.option.includePaths.size();
	for (size_t i = 0; i < includePathCount; i++)
	{
		includePaths.push_back(input.option.includePaths[i].c_str());
		sessionHash = HashCombine(StringHash(includePaths.back()), sessionHash);
	}

	sessionDesc.preprocessorMacroCount = static_cast<uint32_t>(predefineCount);
	sessionDesc.preprocessorMacros = macros.data();
	sessionDesc.searchPathCount = static_cast<uint32_t>(includePathCount);
	sessionDesc.searchPaths = includePaths.data();

	Slang::ComPtr<slang::ISession> session;

	if(_sessionTable.find(sessionHash) != _sessionTable.end())
	{
		session = _sessionTable[sessionHash];
	}
	else
	{
		SlangResult result = _globalSession->createSession(sessionDesc, session.writeRef());
		if (SLANG_FAILED(result))
		{
			HS_LOG(error, "Failed to create Slang session: %d", static_cast<int>(result));
			return false;
		}
		_sessionTable.insert(std::make_pair(sessionHash, session));
	}

	Slang::ComPtr<slang::IModule> slangModule;
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		slangModule = session->loadModuleFromSourceString(
			"",
			input.option.entryPoint.c_str(),
			input.sourceCode.c_str(),
			diagnosticsBlob.writeRef());

		if (diagnosticsBlob)
		{
			output.diagnostics = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
		}

		if (!slangModule)
		{
			HS_LOG(error, "Failed to load Slang module from source: %s", output.diagnostics);
			return false;
		}
	}

	Slang::ComPtr<slang::IEntryPoint> entryPoint;
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		slangModule->findEntryPointByName(input.option.entryPoint.c_str(), entryPoint.writeRef());

		if (diagnosticsBlob)
		{
			if (output.diagnostics.empty())
			{
				output.diagnostics += "\n";
			}
			output.diagnostics += static_cast<const char*>(diagnosticsBlob->getBufferPointer());
		}

		if (!entryPoint)
		{
			HS_LOG(error, "Failed to find entry point '%s': %s", input.option.entryPoint.c_str(), output.diagnostics.c_str());
			return false;
		}
	}

	slang::IComponentType* components[] = { slangModule, entryPoint };
	Slang::ComPtr<slang::IComponentType> program;
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		SlangResult linkResult = session->createCompositeComponentType(
			components, 2, program.writeRef(), diagnosticsBlob.writeRef()
		);

		if (diagnosticsBlob)
		{
			if (!output.diagnostics.empty())
			{
				output.diagnostics += "\n";
			}
			output.diagnostics += static_cast<const char*>(diagnosticsBlob->getBufferPointer());
		}

		if (SLANG_FAILED(linkResult))
		{
			HS_LOG(error, "Failed to link Slang program: %as", output.diagnostics.c_str());
			return false;
		}
	}

	Slang::ComPtr<slang::IBlob> spirvBlob;
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		SlangResult compileResult = program->getEntryPointCode(
			0, 0, spirvBlob.writeRef(), diagnosticsBlob.writeRef()
		);

		if (diagnosticsBlob)
		{
			if (!output.diagnostics.empty())
			{
				output.diagnostics += "\n";
			}
			output.diagnostics += static_cast<const char*>(diagnosticsBlob->getBufferPointer());
		}

		if (SLANG_FAILED(compileResult))
		{
			HS_LOG(error, "Failed to compile Slang program to SPIR-V: %s", output.diagnostics.c_str());
			return false;
		}
	}

	if (spirvBlob)
	{
		const uint32* spirvData = static_cast<const uint32*>(spirvBlob->getBufferPointer());
		size_t spirvSize = spirvBlob->getBufferSize();

		output.code = MakeScoped<char[]>(spirvSize);

		std::memcpy(output.code.get(), spirvData, spirvSize);

		output.diagnostics = "";
	}

	return true;
}

SlangStage SlangCompiler::ConvertShaderStage(uint32 stage)
{
	return static_cast<SlangStage>(stage);
}

HS_NS_END
