#ifndef __HS_SLANG_COMPILER_H__
#define __HS_SLANG_COMPILER_H__

#include "Precompile.h"

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <vector>
#include <string>

HS_NS_BEGIN

enum class SlangStage : uint32
{
    Vertex = SLANG_STAGE_VERTEX,
    Fragment = SLANG_STAGE_FRAGMENT,
    Geometry = SLANG_STAGE_GEOMETRY,
    Hull = SLANG_STAGE_HULL,
    Domain = SLANG_STAGE_DOMAIN,
    Compute = SLANG_STAGE_COMPUTE
};

struct SlangCompileRequest
{
    std::string sourceCode;
    std::string filename;
    std::string entryPoint = "main";
    SlangStage stage = SlangStage::Vertex;
    std::vector<std::string> defines;
    std::vector<std::string> includePaths;
    bool enableDebugInfo = false;
    bool enableOptimization = true;
};

struct SlangCompileResult
{
    std::vector<uint32> spirvBytecode;
    std::string diagnostics;
    bool success = false;
};

class HS_SHADERSYSTEM_API SlangCompiler
{
public:
    SlangCompiler();
    ~SlangCompiler();

    bool Initialize();
    void Shutdown();

    SlangCompileResult CompileToSpirv(const SlangCompileRequest& request);

    bool IsInitialized() const { return _session != nullptr; }

private:
    Slang::ComPtr<slang::IGlobalSession> _globalSession;
    Slang::ComPtr<slang::ISession> _session;

    bool InitializeSession();
    SlangStage ConvertShaderStage(uint32 stage);
};

HS_NS_END

#endif