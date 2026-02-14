#include "Resource/Shader.h"
#include "Resource/ObjectManager.h"

#include "Core/Log.h"
#include "Core/HAL/FileSystem.h"

#include "ShaderSystem/ShaderCompiler.h"

#include <fstream>
#include <sstream>

HS_NS_BEGIN

// Legacy constructor
Shader::Shader(const std::string& source, EShaderStage stage, const std::string& entryPointName)
	: Object(Object::EType::Shader), _source(source), _shaderType(stage), _entryPointName(entryPointName)
{
    _compileOptions.stage = stage;
    _compileOptions.entryPoint = entryPointName;
    _compileOptions.targetLanguage = EShaderLanguage::Spirv;

    _useSimpleMode = true;
}

// New constructor: multi-stage shader for runtime compilation
Shader::Shader(const std::string& shaderName, const std::string& sourceCode, EShaderStage requestedStages,
               const std::vector<std::string>& includePaths)
    : Object(Object::EType::Shader)
    , _shaderName(shaderName)
    , _source(sourceCode)
    , _shaderType(requestedStages)
    , _requestedStages(requestedStages)
    , _includePaths(includePaths)
    , _useSimpleMode(false)
{
    _compileOptions.stage = requestedStages;
#ifdef __APPLE__
    _compileOptions.targetLanguage = EShaderLanguage::Msl;
#else
    _compileOptions.targetLanguage = EShaderLanguage::Spirv;
#endif
}

Shader::~Shader()
{
}

// =======================
// SIMPLIFIED INTERFACE IMPLEMENTATION
// =======================

const ShaderCompileOutput* Shader::GetCompiledData() const
{
    if (_useSimpleMode && _simpleCompiledData.isValid)
    {
        return &_simpleCompiledData;
    }
    return nullptr;
}

bool Shader::CompileVariants()
{
    return false;
}

bool Shader::CompileVariant(const std::string& variantName)
{
    return false;
}

bool Shader::IsCompiled() const
{
    if (_compileOutputEx.isValid) return true;
    if (_useSimpleMode) return _simpleCompiledData.isValid;
    return _isCompiled;
}

// =======================
// NEW: Runtime compilation via ShaderSystem
// =======================

bool Shader::Compile()
{
    if (_compileOutputEx.isValid)
    {
        HS_LOG(info, "[Shader] '%s' already compiled", _shaderName.c_str());
        return true;
    }

    ShaderCompiler compiler;
    if (!compiler.Initialize())
    {
        HS_LOG(error, "[Shader] Failed to initialize ShaderCompiler for '%s'", _shaderName.c_str());
        return false;
    }

    ShaderCompileRequest request;
    request.shaderName = _shaderName;
    request.sourceCode = _source;
    request.includePaths = _includePaths;
    request.requestedStages = _requestedStages;

    _compileOutputEx = compiler.Compile(request);

    compiler.Finalize();

    if (_compileOutputEx.isValid)
    {
        HS_LOG(info, "[Shader] '%s' compiled successfully", _shaderName.c_str());
    }
    else
    {
        HS_LOG(error, "[Shader] '%s' compilation failed: %s",
               _shaderName.c_str(), _compileOutputEx.diagnostics.c_str());
    }

    return _compileOutputEx.isValid;
}

const std::vector<uint8>* Shader::GetBytecode(EShaderStage stage) const
{
    const ShaderStageOutput* stageOutput = _compileOutputEx.GetStageOutput(stage);
    if (stageOutput && stageOutput->isValid)
    {
        return &stageOutput->bytecode;
    }
    return nullptr;
}

const char* Shader::GetEntryPoint(EShaderStage stage) const
{
    const ShaderStageOutput* stageOutput = _compileOutputEx.GetStageOutput(stage);
    if (stageOutput && stageOutput->isValid)
    {
        return stageOutput->entryPoint.c_str();
    }
    return nullptr;
}

HS_NS_END
