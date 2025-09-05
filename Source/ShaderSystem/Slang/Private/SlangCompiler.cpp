#include "ShaderSystem/Slang/SlangCompiler.h"
#include "Core/Log.h"

#include <slang/slang-com-helper.h>
#include <fstream>
#include <sstream>

HS_NS_BEGIN

SlangCompiler::SlangCompiler()
{
}

SlangCompiler::~SlangCompiler()
{
    Shutdown();
}

bool SlangCompiler::Initialize()
{
    if (_session)
    {
        return true;
    }

    SlangResult result = slang::createGlobalSession(_globalSession.writeRef());
    if (SLANG_FAILED(result))
    {
        HS_LOG(crash, "Failed to create Slang global session: {}", result);
        return false;
    }

    return InitializeSession();
}

void SlangCompiler::Shutdown()
{
    _session.setNull();
    _globalSession.setNull();
}

bool SlangCompiler::InitializeSession()
{
    slang::SessionDesc sessionDesc = {};
    
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = _globalSession->findProfile("spirv_1_5");
    
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    SlangResult result = _globalSession->createSession(sessionDesc, _session.writeRef());
    if (SLANG_FAILED(result))
    {
        HS_LOG(crash, "Failed to create Slang session: {}", result);
        return false;
    }

    return true;
}

SlangCompileResult SlangCompiler::CompileToSpirv(const SlangCompileRequest& request)
{
    SlangCompileResult result;
    
    if (!_session)
    {
        result.diagnostics = "Slang session not initialized";
        return result;
    }

    Slang::ComPtr<slang::IModule> module;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        module = _session->loadModuleFromSourceString(
            "", 
            request.filename.c_str(),
            request.sourceCode.c_str(),
            diagnosticsBlob.writeRef());

        if (diagnosticsBlob)
        {
            result.diagnostics = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
        }

        if (!module)
        {
            HS_LOG(error, "Failed to load Slang module from source: {}", result.diagnostics);
            return result;
        }
    }

    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        module->findEntryPointByName(request.entryPoint.c_str(), entryPoint.writeRef());
        
        if (diagnosticsBlob)
        {
            if (!result.diagnostics.empty())
                result.diagnostics += "\n";
            result.diagnostics += static_cast<const char*>(diagnosticsBlob->getBufferPointer());
        }

        if (!entryPoint)
        {
            HS_LOG(error, "Failed to find entry point '{}': {}", request.entryPoint, result.diagnostics);
            return result;
        }
    }

    slang::IComponentType* components[] = { module, entryPoint };
    Slang::ComPtr<slang::IComponentType> program;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult linkResult = _session->createCompositeComponentType(
            components, 2, program.writeRef(), diagnosticsBlob.writeRef()
        );

        if (diagnosticsBlob)
        {
            if (!result.diagnostics.empty())
                result.diagnostics += "\n";
            result.diagnostics += static_cast<const char*>(diagnosticsBlob->getBufferPointer());
        }

        if (SLANG_FAILED(linkResult))
        {
            HS_LOG(error, "Failed to link Slang program: {}", result.diagnostics);
            return result;
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
            if (!result.diagnostics.empty())
                result.diagnostics += "\n";
            result.diagnostics += static_cast<const char*>(diagnosticsBlob->getBufferPointer());
        }

        if (SLANG_FAILED(compileResult))
        {
            HS_LOG(error, "Failed to compile Slang program to SPIR-V: {}", result.diagnostics);
            return result;
        }
    }

    if (spirvBlob)
    {
        const uint32* spirvData = static_cast<const uint32*>(spirvBlob->getBufferPointer());
        size_t spirvSize = spirvBlob->getBufferSize();
        
        result.spirvBytecode.resize(spirvSize / sizeof(uint32));
        std::memcpy(result.spirvBytecode.data(), spirvData, spirvSize);
        result.success = true;
    }

    return result;
}

SlangStage SlangCompiler::ConvertShaderStage(uint32 stage)
{
    return static_cast<SlangStage>(stage);
}

HS_NS_END