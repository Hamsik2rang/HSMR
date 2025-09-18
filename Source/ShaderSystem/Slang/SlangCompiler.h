#ifndef __HS_SLANG_COMPILER_H__
#define __HS_SLANG_COMPILER_H__

#include "Precompile.h"

#include "ShaderSystem/ShaderSystemDefinition.h"

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <vector>
#include <string>

HS_NS_BEGIN

class HS_SHADERSYSTEM_API SlangCompiler
{
public:
    SlangCompiler();
    ~SlangCompiler();

    bool Initialize();
    void Shutdown();

    bool CompileToSpirv(const ShaderCompileInput& input, ShaderCompileOutput& output);

    bool IsInitialized() const { return _globalSession.readRef() != nullptr; }

private:
    Slang::ComPtr<slang::IGlobalSession> _globalSession;
	std::unordered_map<uint32, Slang::ComPtr<slang::ISession>> _sessionTable;

    SlangStage ConvertShaderStage(uint32 stage);
};

HS_NS_END

#endif