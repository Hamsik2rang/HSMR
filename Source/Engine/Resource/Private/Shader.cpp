#include "Resource/Shader.h"
#include "Resource/ObjectManager.h"

#include "Core/Log.h"
#include "Core/HAL/FileSystem.h"
#include <fstream>
#include <sstream>

HS_NS_BEGIN

Shader::Shader(const std::string& source, EShaderStage stage, const std::string& entryPointName)
	: Object(Object::EType::SHADER), _source(source), _shaderType(stage), _entryPointName(entryPointName)
{
    // Initialize simple mode compilation options
    _compileOptions.stage = stage;
    _compileOptions.entryPoint = entryPointName;
    _compileOptions.targetLanguage = EShaderLanguage::SPIRV; // Default to SPIRV
    
    // Default to simple mode for new shaders
    _useSimpleMode = true;
}

Shader::~Shader()
{
    // Cleanup will be handled by destructors
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
    if (_useSimpleMode)
    {
        return _simpleCompiledData.isValid;
    }
    
    return _isCompiled;
}


HS_NS_END
