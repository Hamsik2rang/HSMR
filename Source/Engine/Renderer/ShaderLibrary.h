//
//  ShaderLibrary.h
//  HSMR
//
//  Runtime shader cache with lazy compilation.
//
#ifndef __HS_SHADER_LIBRARY_H__
#define __HS_SHADER_LIBRARY_H__

#include "Precompile.h"

#include "ShaderSystem/ShaderCompiler.h"

#include <string>
#include <unordered_map>

namespace hs { class Shader; }

HS_NS_BEGIN

class HS_API ShaderLibrary
{
public:
    ShaderLibrary();
    ~ShaderLibrary();

    ShaderLibrary(const ShaderLibrary&) = delete;
    ShaderLibrary& operator=(const ShaderLibrary&) = delete;

    bool Initialize(const std::string& shaderSourceDir);
    void Shutdown();

    // Core API: request shader by name, auto-compiles if not yet compiled
    Shader* GetOrCompile(const std::string& shaderName,
                         EShaderStage stages = EShaderStage::Vertex | EShaderStage::Fragment);

    bool HasShader(const std::string& shaderName) const;

private:
    Shader* compileFromSource(const std::string& shaderName, EShaderStage stages);
    std::string readShaderSource(const std::string& shaderName);

    ShaderCompiler _compiler;
    std::string _sourceDir;

    std::unordered_map<std::string, Scoped<Shader>> _shaders;
    bool _initialized = false;
};

HS_NS_END

#endif
