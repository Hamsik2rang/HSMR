#include "ShaderSystem/ShaderCompiler.h"

#include "Core/Log.h"
#include "Core/Hash.h"
#include "Core/HAL/FileSystem.h"

#include <slang.h>
#include <slang-com-ptr.h>

#include <algorithm>
#include <fstream>

HS_NS_BEGIN

static EShaderStage slangStageToHSStage(SlangStage stage)
{
    switch (stage)
    {
    case SLANG_STAGE_VERTEX:   return EShaderStage::Vertex;
    case SLANG_STAGE_FRAGMENT: return EShaderStage::Fragment;
    case SLANG_STAGE_COMPUTE:  return EShaderStage::Compute;
    case SLANG_STAGE_HULL:     return EShaderStage::Hull;
    case SLANG_STAGE_DOMAIN:   return EShaderStage::Domain;
    case SLANG_STAGE_GEOMETRY: return EShaderStage::Geometry;
    default:                   return EShaderStage::None;
    }
}

static const char* getEntryPointNameForStage(EShaderStage stage)
{
    switch (stage)
    {
    case EShaderStage::Vertex:   return "VertexMain";
    case EShaderStage::Fragment: return "FragmentMain";
    case EShaderStage::Compute:  return "ComputeMain";
    default:                     return nullptr;
    }
}

static const char* getStageFileName(EShaderStage stage)
{
    switch (stage)
    {
    case EShaderStage::Vertex:   return "vertex";
    case EShaderStage::Fragment: return "fragment";
    case EShaderStage::Compute:  return "compute";
    default:                     return "unknown";
    }
}

static ShaderBufferMember::BaseType getShaderMemberBaseType(slang::TypeReflection::ScalarType scalarType)
{
    switch (scalarType)
    {
    case slang::TypeReflection::ScalarType::Float16:
    case slang::TypeReflection::ScalarType::Float32:
    case slang::TypeReflection::ScalarType::Float64:
        return ShaderBufferMember::BaseType::Float;
    case slang::TypeReflection::ScalarType::Int8:
    case slang::TypeReflection::ScalarType::Int16:
    case slang::TypeReflection::ScalarType::Int32:
    case slang::TypeReflection::ScalarType::Int64:
        return ShaderBufferMember::BaseType::Int;
    case slang::TypeReflection::ScalarType::UInt8:
    case slang::TypeReflection::ScalarType::UInt16:
    case slang::TypeReflection::ScalarType::UInt32:
    case slang::TypeReflection::ScalarType::UInt64:
        return ShaderBufferMember::BaseType::UInt;
    case slang::TypeReflection::ScalarType::Bool:
        return ShaderBufferMember::BaseType::Bool;
    default:
        return ShaderBufferMember::BaseType::Unknown;
    }
}

static void populateShaderMemberTypeMetadata(ShaderBufferMember& member, slang::TypeLayoutReflection* typeLayout)
{
    if (!typeLayout)
    {
        return;
    }

    slang::TypeReflection* type = typeLayout->getType();
    if (!type)
    {
        return;
    }

    member.rowCount = type->getRowCount();
    member.columnCount = type->getColumnCount();
    member.baseType = getShaderMemberBaseType(type->getScalarType());

    if (type->getKind() == slang::TypeReflection::Kind::Struct)
    {
        member.category = ShaderBufferMember::Category::Struct;
        return;
    }

    if (member.columnCount > 1)
    {
        member.category = ShaderBufferMember::Category::Matrix;
    }
    else if (member.rowCount > 1)
    {
        member.category = ShaderBufferMember::Category::Vector;
    }
    else
    {
        member.category = ShaderBufferMember::Category::Scalar;
    }
}

#if defined(HSMR_SHADER_TRANSPILE_DEBUG) && defined(HSMR_SHADER_TRANSPILE_DEBUG_ROOT)
static bool dumpTranspiledShaderBlob(const ShaderCompileRequest& request,
                                     EShaderStage stage,
                                     const char* extension,
                                     const slang::IBlob* blob)
{
    if (!extension || !blob || blob->getBufferSize() == 0)
    {
        return false;
    }

    const std::string shaderDir = std::string(HSMR_SHADER_TRANSPILE_DEBUG_ROOT) +
        HS_DIR_SEPERATOR + request.shaderName;
    if (!FileSystem::CreateDirectoryRecursive(shaderDir))
    {
        HS_LOG(warning, "[ShaderCompiler] Failed to create transpile dump directory: %s", shaderDir.c_str());
        return false;
    }

    const std::string filePath = shaderDir + HS_DIR_SEPERATOR + getStageFileName(stage) + extension;
    std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        HS_LOG(warning, "[ShaderCompiler] Failed to open transpile dump file: %s", filePath.c_str());
        return false;
    }

    file.write(static_cast<const char*>(blob->getBufferPointer()),
               static_cast<std::streamsize>(blob->getBufferSize()));
    if (!file.good())
    {
        HS_LOG(warning, "[ShaderCompiler] Failed to write transpile dump file: %s", filePath.c_str());
        return false;
    }

    HS_LOG(info, "[ShaderCompiler] Dumped %s %s output: %s",
           request.shaderName.c_str(),
           getStageFileName(stage),
           filePath.c_str());
    return true;
}
#endif

ShaderCompiler::ShaderCompiler()
{
}

ShaderCompiler::~ShaderCompiler()
{
    Finalize();
}

bool ShaderCompiler::Initialize()
{
    if (_initialized) return true;

    SlangResult result = slang::createGlobalSession(&_globalSession);
    if (SLANG_FAILED(result) || !_globalSession)
    {
        HS_LOG(error, "[ShaderCompiler] Failed to create Slang global session");
        return false;
    }

    _initialized = true;
    HS_LOG(info, "[ShaderCompiler] Initialized successfully");
    return true;
}

void ShaderCompiler::Finalize()
{
    if (_globalSession)
    {
        _globalSession->release();
        _globalSession = nullptr;
    }
    _initialized = false;
}

EShaderLanguage ShaderCompiler::GetTargetLanguage() const
{
#ifdef __APPLE__
    return EShaderLanguage::Msl;
#elif __WINDOWS__
    return EShaderLanguage::Spirv;
#else
    return EShaderLanguage::Invalid;
#endif
}

ShaderCompileOutputEx ShaderCompiler::Compile(const ShaderCompileRequest& request)
{
    ShaderCompileOutputEx output;
    output.isValid = false;

    if (!_initialized)
    {
        output.diagnostics = "ShaderCompiler not initialized";
        HS_LOG(error, "[ShaderCompiler] Not initialized");
        return output;
    }

    // 1. Configure target
    std::vector<slang::TargetDesc> targetDescs;
    targetDescs.reserve(2);

    slang::TargetDesc runtimeTargetDesc = {};
    runtimeTargetDesc.structureSize     = sizeof(slang::TargetDesc);
    const char* targetName              = "unknown";
    const char* profileName             = "unknown";

#ifdef __APPLE__
    runtimeTargetDesc.format  = SLANG_METAL;
    runtimeTargetDesc.profile = _globalSession->findProfile("metallib_2_4");
    targetName                = "metal";
    profileName               = "metallib_2_4";
#elif __WINDOWS__
    runtimeTargetDesc.format = SLANG_SPIRV;
    targetName               = "spirv";
    profileName              = "default";
#endif
    targetDescs.push_back(runtimeTargetDesc);

#if defined(HSMR_SHADER_TRANSPILE_DEBUG) && defined(__WINDOWS__)
    slang::TargetDesc debugTargetDesc = {};
    debugTargetDesc.structureSize     = sizeof(slang::TargetDesc);
    debugTargetDesc.format            = SLANG_SPIRV_ASM;
    targetDescs.push_back(debugTargetDesc);
#endif

    // 2. Configure session
    slang::SessionDesc sessionDesc      = {};
    sessionDesc.structureSize           = sizeof(slang::SessionDesc);
    sessionDesc.targets                 = targetDescs.data();
    sessionDesc.targetCount             = static_cast<SlangInt>(targetDescs.size());
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

    // Set up search paths
    std::vector<const char*> searchPaths;
    for (const auto& path : request.includePaths)
    {
        searchPaths.push_back(path.c_str());
    }
    sessionDesc.searchPaths     = searchPaths.data();
    sessionDesc.searchPathCount = static_cast<SlangInt>(searchPaths.size());

    // Set up preprocessor macros
    std::vector<slang::PreprocessorMacroDesc> macros;
    macros.reserve(request.defines.size() + 1);
    for (const auto& def : request.defines)
    {
        slang::PreprocessorMacroDesc macro;
        macro.name  = def.first.c_str();
        macro.value = def.second.c_str();
        macros.push_back(macro);
    }

#ifdef __APPLE__
    macros.push_back({"HS_TARGET_METAL", "1"});
#elif __WINDOWS__
    macros.push_back({"HS_TARGET_SPIRV", "1"});
#endif

    sessionDesc.preprocessorMacros     = macros.data();
    sessionDesc.preprocessorMacroCount = static_cast<SlangInt>(macros.size());

    output.diagnostics = "[ShaderCompiler] target=" + std::string(targetName) + " profile=" + profileName;

    // 3. Create session
    Slang::ComPtr<slang::ISession> session;
    SlangResult result = _globalSession->createSession(sessionDesc, session.writeRef());
    if (SLANG_FAILED(result) || !session)
    {
        output.diagnostics += "\nFailed to create Slang session";
        HS_LOG(error, "[ShaderCompiler] Failed to create session for '%s'", request.shaderName.c_str());
        return output;
    }

    // 4. Load module from source
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = session->loadModuleFromSourceString(
        request.shaderName.c_str(),
        request.sourceFilePath.empty() ? request.shaderName.c_str() : request.sourceFilePath.c_str(),
        request.sourceCode.c_str(),
        diagnosticsBlob.writeRef());

    if (diagnosticsBlob)
    {
        output.diagnostics += "\n";
        output.diagnostics += static_cast<const char*>(diagnosticsBlob->getBufferPointer());
    }

    if (!module)
    {
        HS_LOG(error, "[ShaderCompiler] Failed to load module '%s': %s",
            request.shaderName.c_str(), output.diagnostics.c_str());
        return output;
    }

    // 5. Find entry points for requested stages
    struct StageInfo
    {
        EShaderStage stage;
        slang::IEntryPoint* entryPoint;
    };
    std::vector<StageInfo> foundStages;

    EShaderStage stagesToCheck[] = {EShaderStage::Vertex, EShaderStage::Fragment, EShaderStage::Compute};
    for (auto checkStage : stagesToCheck)
    {
        if ((request.requestedStages & checkStage) == EShaderStage::None) continue;

        const char* epName = getEntryPointNameForStage(checkStage);
        if (!epName) continue;

        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        result = module->findEntryPointByName(epName, entryPoint.writeRef());
        if (SLANG_SUCCEEDED(result) && entryPoint)
        {
            foundStages.push_back({checkStage, entryPoint.detach()});
        }
        else
        {
            HS_LOG(warning, "[ShaderCompiler] Entry point '%s' not found in '%s'",
                epName, request.shaderName.c_str());
        }
    }

    if (foundStages.empty())
    {
        output.diagnostics += "\nNo entry points found";
        HS_LOG(error, "[ShaderCompiler] No entry points found in '%s'", request.shaderName.c_str());
        return output;
    }

    // 6. Create composite component type (module + entry points)
    std::vector<slang::IComponentType*> componentTypes;
    componentTypes.push_back(module);
    for (auto& si : foundStages)
    {
        componentTypes.push_back(si.entryPoint);
    }

    Slang::ComPtr<slang::IComponentType> compositeProgram;
    diagnosticsBlob = nullptr;
    result          = session->createCompositeComponentType(
        componentTypes.data(),
        static_cast<SlangInt>(componentTypes.size()),
        compositeProgram.writeRef(),
        diagnosticsBlob.writeRef());

    if (diagnosticsBlob)
    {
        output.diagnostics += static_cast<const char*>(diagnosticsBlob->getBufferPointer());
    }

    if (SLANG_FAILED(result) || !compositeProgram)
    {
        HS_LOG(error, "[ShaderCompiler] Failed to create composite for '%s'", request.shaderName.c_str());
        for (auto& si : foundStages) si.entryPoint->release();
        return output;
    }

    // 7. Link
    Slang::ComPtr<slang::IComponentType> linkedProgram;
    diagnosticsBlob = nullptr;
    result          = compositeProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());

    if (diagnosticsBlob)
    {
        output.diagnostics += static_cast<const char*>(diagnosticsBlob->getBufferPointer());
    }

    if (SLANG_FAILED(result) || !linkedProgram)
    {
        HS_LOG(error, "[ShaderCompiler] Link failed for '%s': %s",
            request.shaderName.c_str(), output.diagnostics.c_str());
        for (auto& si : foundStages) si.entryPoint->release();
        return output;
    }

    // 8. Get bytecode for each entry point
    const int runtimeTargetIndex = 0;
    for (size_t i = 0; i < foundStages.size(); ++i)
    {
        ShaderStageOutput stageOutput;
        stageOutput.stage      = foundStages[i].stage;
        stageOutput.language   = GetTargetLanguage();
        stageOutput.entryPoint = getEntryPointNameForStage(foundStages[i].stage);

        Slang::ComPtr<slang::IBlob> codeBlob;
        diagnosticsBlob = nullptr;
        result          = linkedProgram->getEntryPointCode(
            static_cast<SlangInt>(i), runtimeTargetIndex,
            codeBlob.writeRef(), diagnosticsBlob.writeRef());

        if (diagnosticsBlob)
        {
            output.diagnostics += static_cast<const char*>(diagnosticsBlob->getBufferPointer());
        }

        if (SLANG_SUCCEEDED(result) && codeBlob && codeBlob->getBufferSize() > 0)
        {
            const uint8* data = static_cast<const uint8*>(codeBlob->getBufferPointer());
            size_t size       = codeBlob->getBufferSize();
            stageOutput.bytecode.assign(data, data + size);

#ifdef __APPLE__
            // Metal: keep original entry point name
#elif __WINDOWS__
            // SPIRV: entry point is always "main"
            stageOutput.entryPoint = "main";
#endif
            stageOutput.isValid = true;
            HS_LOG(info, "[ShaderCompiler] Stage %d compiled: %zu bytes",
                static_cast<int>(stageOutput.stage), size);
        }
        else
        {
            HS_LOG(error, "[ShaderCompiler] Failed to get code for stage %d of '%s'",
                static_cast<int>(foundStages[i].stage), request.shaderName.c_str());
        }

#if defined(HSMR_SHADER_TRANSPILE_DEBUG) && defined(HSMR_SHADER_TRANSPILE_DEBUG_ROOT)
#ifdef __APPLE__
        if (codeBlob && codeBlob->getBufferSize() > 0)
        {
            dumpTranspiledShaderBlob(request, foundStages[i].stage, ".metal", codeBlob.get());
        }
#elif __WINDOWS__
        if (codeBlob && codeBlob->getBufferSize() > 0)
        {
            dumpTranspiledShaderBlob(request, foundStages[i].stage, ".spv", codeBlob.get());
        }

        if (targetDescs.size() > 1)
        {
            Slang::ComPtr<slang::IBlob> asmBlob;
            Slang::ComPtr<slang::IBlob> asmDiagnosticsBlob;
            SlangResult asmResult = linkedProgram->getEntryPointCode(
                static_cast<SlangInt>(i), 1,
                asmBlob.writeRef(), asmDiagnosticsBlob.writeRef());

            if (asmDiagnosticsBlob)
            {
                output.diagnostics += static_cast<const char*>(asmDiagnosticsBlob->getBufferPointer());
            }

            if (SLANG_SUCCEEDED(asmResult) && asmBlob && asmBlob->getBufferSize() > 0)
            {
                dumpTranspiledShaderBlob(request, foundStages[i].stage, ".spv.asm", asmBlob.get());
            }
            else
            {
                HS_LOG(warning, "[ShaderCompiler] Failed to get SPIR-V assembly for stage %d of '%s'",
                       static_cast<int>(foundStages[i].stage), request.shaderName.c_str());
            }
        }
#endif
#endif

        output.stages.push_back(std::move(stageOutput));
    }

    // 9. Extract reflection
    extractReflection(linkedProgram.get(), runtimeTargetIndex, request, output.reflection);

    // Check if all requested stages produced valid output
    bool allValid = true;
    for (const auto& s : output.stages)
    {
        if (!s.isValid)
        {
            allValid = false;
            break;
        }
    }
    output.isValid = allValid && output.reflection.isValid;

    // Cleanup
    for (auto& si : foundStages)
    {
        si.entryPoint->release();
    }

    if (output.isValid)
    {
        HS_LOG(info, "[ShaderCompiler] '%s' compiled successfully (%zu stages)",
            request.shaderName.c_str(), output.stages.size());
    }

    return output;
}

ShaderCompileOutputEx ShaderCompiler::CompileFromFile(const std::string& filePath,
    EShaderStage requestedStages)
{
    ShaderCompileOutputEx output;

    FileHandle fileHandle;
    if (!FileSystem::Open(filePath, EFileAccess::ReadOnly, fileHandle))
    {
        output.diagnostics = "Failed to open file: " + filePath;
        HS_LOG(error, "[ShaderCompiler] %s", output.diagnostics.c_str());
        return output;
    }

    size_t fileSize = FileSystem::GetSize(fileHandle);
    std::string sourceCode;
    sourceCode.resize(fileSize);
    size_t bytesRead = FileSystem::Read(fileHandle, sourceCode.data(), fileSize);
    FileSystem::Close(fileHandle);

    if (bytesRead == 0)
    {
        output.diagnostics = "Failed to read file: " + filePath;
        HS_LOG(error, "[ShaderCompiler] %s", output.diagnostics.c_str());
        return output;
    }
    sourceCode.resize(bytesRead);

    // Extract shader name from file path
    std::string shaderName = filePath;
    auto lastSlash         = shaderName.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        shaderName = shaderName.substr(lastSlash + 1);
    }

    // Extract directory as include path
    std::string includeDir = filePath;
    if (lastSlash != std::string::npos)
    {
        includeDir = filePath.substr(0, lastSlash);
    }

    ShaderCompileRequest request;
    request.shaderName     = shaderName;
    request.sourceCode     = sourceCode;
    request.sourceFilePath = filePath;
    request.includePaths.push_back(includeDir);
    request.requestedStages = requestedStages;

    return Compile(request);
}

// ============================
// Reflection extraction
// ============================

enum class EResourceCategory
{
    Buffer,
    Texture,
    Sampler
};

static const char* getTypeName(slang::TypeReflection* type)
{
    const char* typeName = type ? type->getName() : nullptr;
    return typeName ? typeName : "";
}

static bool isSamplerType(slang::TypeReflection* type,
    slang::TypeReflection::Kind kind,
    slang::ParameterCategory category)
{
    if (kind == slang::TypeReflection::Kind::SamplerState ||
        category == slang::ParameterCategory::SamplerState)
    {
        return true;
    }

    std::string typeName = getTypeName(type);
    return typeName.find("SamplerState") != std::string::npos ||
           typeName.find("SamplerComparisonState") != std::string::npos;
}

static bool isTextureType(slang::TypeReflection* type,
    slang::ParameterCategory category)
{
    if (category == slang::ParameterCategory::ShaderResource)
    {
        return true;
    }

    std::string typeName = getTypeName(type);
    return typeName.find("Texture") != std::string::npos;
}

static uint32 getResourceBinding(slang::VariableLayoutReflection* param,
    EResourceCategory category,
    EShaderLanguage targetLanguage)
{
    if (targetLanguage == EShaderLanguage::Msl)
    {
        switch (category)
        {
        case EResourceCategory::Buffer:
            return static_cast<uint32>(param->getOffset(slang::ParameterCategory::MetalBuffer));
        case EResourceCategory::Texture:
            return static_cast<uint32>(param->getOffset(slang::ParameterCategory::MetalTexture));
        case EResourceCategory::Sampler:
            return static_cast<uint32>(param->getOffset(slang::ParameterCategory::SamplerState));
        }
    }
    return static_cast<uint32>(param->getBindingIndex());
}

static void setNativeBindingSlot(NativeShaderBindingSlots& slots, EShaderStage stage, uint32 binding)
{
    switch (stage)
    {
    case EShaderStage::Vertex:
        slots.vertexBinding = binding;
        break;
    case EShaderStage::Fragment:
        slots.fragmentBinding = binding;
        break;
    case EShaderStage::Compute:
        slots.computeBinding = binding;
        break;
    default:
        HS_LOG(crash, "Unsupported Stage");
        break;
    }
}

static NativeShaderBindingSlots makeNativeBindingSlots(uint32 binding,
    EShaderStage stages,
    EShaderLanguage targetLanguage)
{
    NativeShaderBindingSlots slots;
    if (targetLanguage != EShaderLanguage::Msl)
    {
        return slots;
    }

    bool hasBound = false;
    if ((stages & EShaderStage::Vertex) != EShaderStage::None)
    {
        slots.vertexBinding = binding;
        hasBound            = true;
    }
    if ((stages & EShaderStage::Fragment) != EShaderStage::None)
    {
        slots.fragmentBinding = binding;
        hasBound              = true;
    }
    if ((stages & EShaderStage::Compute) != EShaderStage::None)
    {
        slots.computeBinding = binding;
        hasBound             = true;
    }

    if (!hasBound)
    {
        HS_LOG(crash, "Unsupported Stage");
    }

    return slots;
}

static void mergeNativeBindingSlots(NativeShaderBindingSlots& dst, const NativeShaderBindingSlots& src)
{
    if (src.vertexBinding != NativeShaderBindingSlots::Invalid)
    {
        dst.vertexBinding = src.vertexBinding;
    }
    if (src.fragmentBinding != NativeShaderBindingSlots::Invalid)
    {
        dst.fragmentBinding = src.fragmentBinding;
    }
    if (src.computeBinding != NativeShaderBindingSlots::Invalid)
    {
        dst.computeBinding = src.computeBinding;
    }
}

static void mergeDuplicateBufferBindings(std::vector<ShaderBufferBindingInfo>& bindings)
{
    std::vector<ShaderBufferBindingInfo> merged;
    merged.reserve(bindings.size());

    for (ShaderBufferBindingInfo& binding : bindings)
    {
        auto it = std::find_if(merged.begin(), merged.end(),
            [&binding](const ShaderBufferBindingInfo& existing)
            {
                return existing.name == binding.name &&
                       existing.set == binding.set &&
                       existing.resourceType == binding.resourceType;
            });

        if (it == merged.end())
        {
            merged.push_back(std::move(binding));
            continue;
        }

        it->stages |= binding.stages;
        mergeNativeBindingSlots(it->nativeBindingSlots, binding.nativeBindingSlots);

        if (it->totalSize == 0)
        {
            it->totalSize = binding.totalSize;
        }
        if (it->members.empty())
        {
            it->members = std::move(binding.members);
        }
    }

    bindings = std::move(merged);
}

void ShaderCompiler::extractReflection(void* linkedProgramPtr, int targetIndex,
    const ShaderCompileRequest& request,
    ShaderReflectionDataEx& outReflection)
{
    auto* linkedProgram          = static_cast<slang::IComponentType*>(linkedProgramPtr);
    slang::ProgramLayout* layout = linkedProgram->getLayout(targetIndex, nullptr);
    if (!layout)
    {
        HS_LOG(error, "[ShaderCompiler] Failed to get program layout for reflection");
        return;
    }

    EShaderLanguage targetLang = GetTargetLanguage();

    // 글로벌 파라미터 추출 (ConstantBuffers, textures, samplers)
    unsigned paramCount = layout->getParameterCount();
    for (unsigned i = 0; i < paramCount; ++i)
    {
        slang::VariableLayoutReflection* param = layout->getParameterByIndex(i);
        if (!param) continue;

        slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
        slang::TypeReflection* type             = typeLayout->getType();
        auto kind                               = type->getKind();
        auto category                           = typeLayout->getParameterCategory();

        // Debug: log all parameters
        HS_LOG(debug, "[ShaderCompiler] Param '%s': type='%s', kind=%d, category=%d, binding=%d",
            param->getName() ? param->getName() : "(null)",
            getTypeName(type),
            static_cast<int>(kind),
            static_cast<int>(category),
            static_cast<int>(param->getBindingIndex()));

        if (kind == slang::TypeReflection::Kind::ConstantBuffer ||
            kind == slang::TypeReflection::Kind::ParameterBlock)
        {
            ShaderBufferBindingInfo bufInfo;
            bufInfo.name         = param->getName() ? param->getName() : "";
            bufInfo.binding      = getResourceBinding(param, EResourceCategory::Buffer, targetLang);
            bufInfo.set          = static_cast<uint32>(param->getBindingSpace());
            bufInfo.resourceType = EResourceType::UniformBuffer;

            // Determine which stages use this buffer
            // For now, check all requested stages
            bufInfo.stages             = request.requestedStages;
            bufInfo.nativeBindingSlots = makeNativeBindingSlots(bufInfo.binding, bufInfo.stages, targetLang);

            // Get element type layout for the constant buffer contents
            slang::TypeLayoutReflection* elementLayout = typeLayout->getElementTypeLayout();
            if (elementLayout)
            {
                bufInfo.totalSize = static_cast<uint32>(elementLayout->getSize(slang::ParameterCategory::Uniform));

                unsigned fieldCount = elementLayout->getFieldCount();
                for (unsigned f = 0; f < fieldCount; ++f)
                {
                    slang::VariableLayoutReflection* field = elementLayout->getFieldByIndex(f);
                    if (!field) continue;

                    ShaderBufferMember member;
                    member.name     = field->getName() ? field->getName() : "";
                    member.offset   = static_cast<uint32>(field->getOffset(slang::ParameterCategory::Uniform));
                    member.size     = static_cast<uint32>(field->getTypeLayout()->getSize(slang::ParameterCategory::Uniform));
                    member.nameHash = StringHash(member.name);
                    populateShaderMemberTypeMetadata(member, field->getTypeLayout());
                    bufInfo.members.push_back(std::move(member));
                }
            }

            outReflection.bufferBindings.push_back(std::move(bufInfo));
        }
        else if (kind == slang::TypeReflection::Kind::Resource ||
                 kind == slang::TypeReflection::Kind::SamplerState)
        {
            // Could be texture or sampler. Some Slang targets report SamplerState
            // as a resource/descriptor-table slot, so classify by actual type too.
            auto bindingType = typeLayout->getParameterCategory();

            if (isSamplerType(type, kind, bindingType))
            {
                ShaderSamplerBindingInfo sampInfo;
                sampInfo.name               = param->getName() ? param->getName() : "";
                sampInfo.binding            = getResourceBinding(param, EResourceCategory::Sampler, targetLang);
                sampInfo.set                = static_cast<uint32>(param->getBindingSpace());
                sampInfo.stages             = request.requestedStages;
                sampInfo.nativeBindingSlots = makeNativeBindingSlots(sampInfo.binding, sampInfo.stages, targetLang);
                outReflection.samplerBindings.push_back(std::move(sampInfo));
            }
            else if (isTextureType(type, bindingType))
            {
                ShaderTextureBindingInfo texInfo;
                texInfo.name               = param->getName() ? param->getName() : "";
                texInfo.binding            = getResourceBinding(param, EResourceCategory::Texture, targetLang);
                texInfo.set                = static_cast<uint32>(param->getBindingSpace());
                texInfo.stages             = request.requestedStages;
                texInfo.nativeBindingSlots = makeNativeBindingSlots(texInfo.binding, texInfo.stages, targetLang);
                texInfo.dimension          = 2; // Default to 2D
                outReflection.textureBindings.push_back(std::move(texInfo));
            }
        }
    }

    // Extract entry point info and vertex inputs
    SlangUInt entryPointCount = layout->getEntryPointCount();
    for (SlangUInt ep = 0; ep < entryPointCount; ++ep)
    {
        slang::EntryPointReflection* epReflection = layout->getEntryPointByIndex(ep);
        if (!epReflection) continue;

        SlangStage slangStage = epReflection->getStage();
        EShaderStage hsStage  = slangStageToHSStage(slangStage);

        const char* epName = epReflection->getName();
        if (hsStage == EShaderStage::Vertex)
        {
            outReflection.vertexEntryPoint = epName ? epName : "VertexMain";
            extractVertexInputFromEntryPoint(epReflection, outReflection.vertexInput);
        }
        else if (hsStage == EShaderStage::Fragment)
        {
            outReflection.fragmentEntryPoint = epName ? epName : "FragmentMain";
        }
        else if (hsStage == EShaderStage::Compute)
        {
            outReflection.computeEntryPoint = epName ? epName : "ComputeMain";
        }

        // Also check entry point parameters for per-entry-point bindings
        unsigned epParamCount = epReflection->getParameterCount();
        for (unsigned p = 0; p < epParamCount; ++p)
        {
            slang::VariableLayoutReflection* param = epReflection->getParameterByIndex(p);
            if (!param) continue;

            auto cat = param->getCategory();
            if (cat == slang::ParameterCategory::ConstantBuffer)
            {
                // Entry-point specific constant buffer
                slang::TypeLayoutReflection* typeLayout    = param->getTypeLayout();
                slang::TypeLayoutReflection* elementLayout = typeLayout->getElementTypeLayout();

                ShaderBufferBindingInfo bufInfo;
                bufInfo.name    = param->getName() ? param->getName() : "";
                bufInfo.binding = getResourceBinding(param, EResourceCategory::Buffer, targetLang);
                bufInfo.set     = static_cast<uint32>(param->getBindingSpace());
                bufInfo.stages  = hsStage;
                setNativeBindingSlot(bufInfo.nativeBindingSlots, hsStage, bufInfo.binding);
                bufInfo.resourceType = EResourceType::UniformBuffer;

                if (elementLayout)
                {
                    bufInfo.totalSize   = static_cast<uint32>(elementLayout->getSize(slang::ParameterCategory::Uniform));
                    unsigned fieldCount = elementLayout->getFieldCount();
                    for (unsigned f = 0; f < fieldCount; ++f)
                    {
                        slang::VariableLayoutReflection* field = elementLayout->getFieldByIndex(f);
                        if (!field) continue;

                        ShaderBufferMember member;
                        member.name     = field->getName() ? field->getName() : "";
                        member.offset   = static_cast<uint32>(field->getOffset(slang::ParameterCategory::Uniform));
                        member.size     = static_cast<uint32>(field->getTypeLayout()->getSize(slang::ParameterCategory::Uniform));
                        member.nameHash = StringHash(member.name);
                        populateShaderMemberTypeMetadata(member, field->getTypeLayout());
                        bufInfo.members.push_back(std::move(member));
                    }
                }

                outReflection.bufferBindings.push_back(std::move(bufInfo));
            }
        }
    }

    mergeDuplicateBufferBindings(outReflection.bufferBindings);

    // Build lookup tables
    outReflection.BuildLookup();
    outReflection.isValid = true;

    // Log reflection summary
    HS_LOG(info, "[ShaderCompiler] Reflection: %zu buffers, %zu textures, %zu samplers, %zu vertex attrs",
        outReflection.bufferBindings.size(),
        outReflection.textureBindings.size(),
        outReflection.samplerBindings.size(),
        outReflection.vertexInput.attributes.size());

    for (const auto& buf : outReflection.bufferBindings)
    {
        HS_LOG(info, "[ShaderCompiler]   Buffer '%s' @ binding %u (v=%u f=%u c=%u), %u bytes, %zu members",
            buf.name.c_str(),
            buf.binding,
            buf.nativeBindingSlots.GetStageBindingOr(buf.binding, EShaderStage::Vertex),
            buf.nativeBindingSlots.GetStageBindingOr(buf.binding, EShaderStage::Fragment),
            buf.nativeBindingSlots.GetStageBindingOr(buf.binding, EShaderStage::Compute),
            buf.totalSize,
            buf.members.size());
    }
    for (const auto& tex : outReflection.textureBindings)
    {
        HS_LOG(info, "[ShaderCompiler]   Texture '%s' @ binding %u (v=%u f=%u c=%u)",
            tex.name.c_str(),
            tex.binding,
            tex.nativeBindingSlots.GetStageBindingOr(tex.binding, EShaderStage::Vertex),
            tex.nativeBindingSlots.GetStageBindingOr(tex.binding, EShaderStage::Fragment),
            tex.nativeBindingSlots.GetStageBindingOr(tex.binding, EShaderStage::Compute));
    }
    for (const auto& samp : outReflection.samplerBindings)
    {
        HS_LOG(info, "[ShaderCompiler]   Sampler '%s' @ binding %u (v=%u f=%u c=%u)",
            samp.name.c_str(),
            samp.binding,
            samp.nativeBindingSlots.GetStageBindingOr(samp.binding, EShaderStage::Vertex),
            samp.nativeBindingSlots.GetStageBindingOr(samp.binding, EShaderStage::Fragment),
            samp.nativeBindingSlots.GetStageBindingOr(samp.binding, EShaderStage::Compute));
    }
}

void ShaderCompiler::extractVertexInputFromEntryPoint(void* entryPointReflectionPtr,
    ShaderVertexInputLayout& outLayout)
{
    auto* epReflection  = static_cast<slang::EntryPointReflection*>(entryPointReflectionPtr);
    unsigned paramCount = epReflection->getParameterCount();

    uint32 currentOffset = 0;

    for (unsigned i = 0; i < paramCount; ++i)
    {
        slang::VariableLayoutReflection* param = epReflection->getParameterByIndex(i);
        if (!param) continue;

        auto cat = param->getCategory();

        // Check for VaryingInput (vertex input)
        if (cat != slang::ParameterCategory::VaryingInput &&
            cat != slang::ParameterCategory::Mixed)
        {
            continue;
        }

        slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
        slang::TypeReflection* type             = typeLayout->getType();
        auto kind                               = type->getKind();

        if (kind == slang::TypeReflection::Kind::Struct)
        {
            // VSInput struct: iterate its fields
            unsigned fieldCount = typeLayout->getFieldCount();
            for (unsigned f = 0; f < fieldCount; ++f)
            {
                slang::VariableLayoutReflection* field = typeLayout->getFieldByIndex(f);
                if (!field) continue;

                const char* semantic = field->getSemanticName();
                size_t semanticIdx   = field->getSemanticIndex();

                if (!semantic) continue;

                slang::TypeLayoutReflection* fieldTypeLayout = field->getTypeLayout();
                slang::TypeReflection* fieldType             = fieldTypeLayout->getType();

                ShaderVertexAttribute attr;
                attr.semantic = std::string(semantic) + std::to_string(semanticIdx);
                attr.location = static_cast<uint32>(field->getOffset(slang::ParameterCategory::VaryingInput));

                unsigned rows   = fieldType->getRowCount();
                unsigned cols   = fieldType->getColumnCount();
                auto scalarType = fieldType->getScalarType();

                attr.format = scalarTypeToVertexFormat(static_cast<int>(scalarType), rows, cols);
                attr.offset = currentOffset;

                // Calculate size based on format
                switch (attr.format)
                {
                case EVertexFormat::Float:  attr.size = 4; break;
                case EVertexFormat::Float2: attr.size = 8; break;
                case EVertexFormat::Float3: attr.size = 12; break;
                case EVertexFormat::Float4: attr.size = 16; break;
                case EVertexFormat::Half:   attr.size = 2; break;
                case EVertexFormat::Half2:  attr.size = 4; break;
                case EVertexFormat::Half3:  attr.size = 6; break;
                case EVertexFormat::Half4:  attr.size = 8; break;
                default:                    attr.size = 0; break;
                }

                currentOffset += attr.size;
                outLayout.attributes.push_back(std::move(attr));
            }
        }
        else
        {
            // Scalar/Vector type directly as vertex input
            const char* semantic = param->getSemanticName();
            size_t semanticIdx   = param->getSemanticIndex();

            ShaderVertexAttribute attr;
            attr.semantic = semantic ? (std::string(semantic) + std::to_string(semanticIdx)) : "";
            attr.location = static_cast<uint32>(param->getOffset(slang::ParameterCategory::VaryingInput));

            unsigned rows   = type->getRowCount();
            unsigned cols   = type->getColumnCount();
            auto scalarType = type->getScalarType();

            attr.format = scalarTypeToVertexFormat(static_cast<int>(scalarType), rows, cols);
            attr.offset = currentOffset;

            switch (attr.format)
            {
            case EVertexFormat::Float:  attr.size = 4; break;
            case EVertexFormat::Float2: attr.size = 8; break;
            case EVertexFormat::Float3: attr.size = 12; break;
            case EVertexFormat::Float4: attr.size = 16; break;
            case EVertexFormat::Half:   attr.size = 2; break;
            case EVertexFormat::Half2:  attr.size = 4; break;
            case EVertexFormat::Half3:  attr.size = 6; break;
            case EVertexFormat::Half4:  attr.size = 8; break;
            default:                    attr.size = 0; break;
            }

            currentOffset += attr.size;
            outLayout.attributes.push_back(std::move(attr));
        }
    }

    outLayout.stride = currentOffset;

    HS_LOG(info, "[ShaderCompiler] Vertex layout: stride=%u, %zu attributes",
        outLayout.stride, outLayout.attributes.size());
    for (const auto& attr : outLayout.attributes)
    {
        HS_LOG(info, "[ShaderCompiler]   Attr '%s' loc=%u offset=%u size=%u",
            attr.semantic.c_str(), attr.location, attr.offset, attr.size);
    }
}

EVertexFormat ShaderCompiler::scalarTypeToVertexFormat(int scalarType, uint32 rows, uint32 cols) const
{
    // For vectors: rows = element count, cols = 1
    // For scalars: rows = 1, cols = 1
    // For matrices: rows = row count, cols = col count
    uint32 componentCount = (cols > 1) ? rows * cols : rows;
    if (componentCount == 0) componentCount = 1;

    auto slangScalar = static_cast<slang::TypeReflection::ScalarType>(scalarType);

    switch (slangScalar)
    {
    case slang::TypeReflection::ScalarType::Float32:
        switch (componentCount)
        {
        case 1:  return EVertexFormat::Float;
        case 2:  return EVertexFormat::Float2;
        case 3:  return EVertexFormat::Float3;
        case 4:  return EVertexFormat::Float4;
        default: return EVertexFormat::Invalid;
        }
    case slang::TypeReflection::ScalarType::Float16:
        switch (componentCount)
        {
        case 1:  return EVertexFormat::Half;
        case 2:  return EVertexFormat::Half2;
        case 3:  return EVertexFormat::Half3;
        case 4:  return EVertexFormat::Half4;
        default: return EVertexFormat::Invalid;
        }
    default:
        return EVertexFormat::Invalid;
    }
}

HS_NS_END
