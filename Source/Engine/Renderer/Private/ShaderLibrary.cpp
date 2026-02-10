#include "Renderer/ShaderLibrary.h"

#include "Core/Log.h"
#include "Core/HAL/FileSystem.h"

#include "Resource/Shader.h"

#include <fstream>
#include <sstream>

HS_NS_BEGIN

ShaderLibrary::ShaderLibrary()
{
}

ShaderLibrary::~ShaderLibrary()
{
    Shutdown();
}

bool ShaderLibrary::Initialize(const std::string& shaderSourceDir)
{
    if (_initialized)
    {
        return true;
    }

    _sourceDir = shaderSourceDir;

    if (!_compiler.Initialize())
    {
        HS_LOG(error, "[ShaderLibrary] Failed to initialize ShaderCompiler");
        return false;
    }

    _initialized = true;
    HS_LOG(info, "[ShaderLibrary] Initialized (source: %s)", _sourceDir.c_str());
    return true;
}

void ShaderLibrary::Shutdown()
{
    _shaders.clear();
    _compiler.Finalize();
    _initialized = false;
}

Shader* ShaderLibrary::GetOrCompile(const std::string& shaderName, EShaderStage stages)
{
    if (!_initialized)
    {
        HS_LOG(error, "[ShaderLibrary] Not initialized");
        return nullptr;
    }

    // 1. In-memory cache hit
    auto it = _shaders.find(shaderName);
    if (it != _shaders.end())
    {
        return it->second.get();
    }

    // 2. Cache miss - compile from source
    return compileFromSource(shaderName, stages);
}

bool ShaderLibrary::HasShader(const std::string& shaderName) const
{
    return _shaders.find(shaderName) != _shaders.end();
}

Shader* ShaderLibrary::compileFromSource(const std::string& shaderName, EShaderStage stages)
{
    std::string source = readShaderSource(shaderName);
    if (source.empty())
    {
        return nullptr;
    }

    ShaderCompileRequest request;
    request.shaderName = shaderName;
    request.sourceCode = source;
    request.includePaths = { _sourceDir };
    request.requestedStages = stages;

    ShaderCompileOutputEx output = _compiler.Compile(request);
    if (!output.isValid)
    {
        HS_LOG(error, "[ShaderLibrary] Compilation failed for '%s': %s",
               shaderName.c_str(), output.diagnostics.c_str());
        return nullptr;
    }

    // Create Shader object and store in memory cache
    auto shader = MakeScoped<Shader>(shaderName, source, stages, std::vector<std::string>{_sourceDir});
    shader->SetCompileOutputEx(std::move(output));

    Shader* result = shader.get();
    _shaders[shaderName] = std::move(shader);

    HS_LOG(info, "[ShaderLibrary] '%s' compiled and cached", shaderName.c_str());
    return result;
}

std::string ShaderLibrary::readShaderSource(const std::string& shaderName)
{
    std::string filePath = _sourceDir + HS_DIR_SEPERATOR + shaderName + ".slang";

    std::ifstream file(filePath, std::ios::in);
    if (!file.is_open())
    {
        HS_LOG(error, "[ShaderLibrary] Cannot open shader source: %s", filePath.c_str());
        return "";
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

HS_NS_END
