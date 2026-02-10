#include "ShaderSystem/ShaderCompiler.h"

#include "Core/Log.h"
#include "Core/Hash.h"
#include "Core/HAL/FileSystem.h"

#include <slang.h>
#include <slang-com-ptr.h>

HS_NS_BEGIN

static EShaderStage slangStageToHSStage(SlangStage stage)
{
    switch (stage)
    {
    case SLANG_STAGE_VERTEX:   return EShaderStage::VERTEX;
    case SLANG_STAGE_FRAGMENT: return EShaderStage::FRAGMENT;
    case SLANG_STAGE_COMPUTE:  return EShaderStage::COMPUTE;
    case SLANG_STAGE_HULL:     return EShaderStage::HULL;
    case SLANG_STAGE_DOMAIN:   return EShaderStage::DOMAIN;
    case SLANG_STAGE_GEOMETRY: return EShaderStage::GEOMETRY;
    default: return EShaderStage::NONE;
    }
}

static const char* getEntryPointNameForStage(EShaderStage stage)
{
    switch (stage)
    {
    case EShaderStage::VERTEX:   return "VertexMain";
    case EShaderStage::FRAGMENT: return "FragmentMain";
    case EShaderStage::COMPUTE:  return "ComputeMain";
    default: return nullptr;
    }
}

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
    return EShaderLanguage::MSL;
#elif __WINDOWS__
    return EShaderLanguage::SPIRV;
#else
    return EShaderLanguage::INVALID;
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
    slang::TargetDesc targetDesc = {};
    targetDesc.structureSize = sizeof(slang::TargetDesc);
#ifdef __APPLE__
    targetDesc.format = SLANG_METAL;
#elif __WINDOWS__
    targetDesc.format = SLANG_SPIRV;
#endif

    // 2. Configure session
    slang::SessionDesc sessionDesc = {};
    sessionDesc.structureSize = sizeof(slang::SessionDesc);
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

    // Set up search paths
    std::vector<const char*> searchPaths;
    for (const auto& path : request.includePaths)
    {
        searchPaths.push_back(path.c_str());
    }
    sessionDesc.searchPaths = searchPaths.data();
    sessionDesc.searchPathCount = static_cast<SlangInt>(searchPaths.size());

    // Set up preprocessor macros
    std::vector<slang::PreprocessorMacroDesc> macros;
    for (const auto& def : request.defines)
    {
        slang::PreprocessorMacroDesc macro;
        macro.name = def.first.c_str();
        macro.value = def.second.c_str();
        macros.push_back(macro);
    }
    sessionDesc.preprocessorMacros = macros.data();
    sessionDesc.preprocessorMacroCount = static_cast<SlangInt>(macros.size());

    // 3. Create session
    Slang::ComPtr<slang::ISession> session;
    SlangResult result = _globalSession->createSession(sessionDesc, session.writeRef());
    if (SLANG_FAILED(result) || !session)
    {
        output.diagnostics = "Failed to create Slang session";
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
        output.diagnostics = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
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

    EShaderStage stagesToCheck[] = { EShaderStage::VERTEX, EShaderStage::FRAGMENT, EShaderStage::COMPUTE };
    for (auto checkStage : stagesToCheck)
    {
        if ((request.requestedStages & checkStage) == EShaderStage::NONE) continue;

        const char* epName = getEntryPointNameForStage(checkStage);
        if (!epName) continue;

        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        result = module->findEntryPointByName(epName, entryPoint.writeRef());
        if (SLANG_SUCCEEDED(result) && entryPoint)
        {
            foundStages.push_back({ checkStage, entryPoint.detach() });
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
    result = session->createCompositeComponentType(
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
    result = compositeProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());

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
    const int targetIndex = 0;
    for (size_t i = 0; i < foundStages.size(); ++i)
    {
        ShaderStageOutput stageOutput;
        stageOutput.stage = foundStages[i].stage;
        stageOutput.language = GetTargetLanguage();
        stageOutput.entryPoint = getEntryPointNameForStage(foundStages[i].stage);

        Slang::ComPtr<slang::IBlob> codeBlob;
        diagnosticsBlob = nullptr;
        result = linkedProgram->getEntryPointCode(
            static_cast<SlangInt>(i), targetIndex,
            codeBlob.writeRef(), diagnosticsBlob.writeRef());

        if (diagnosticsBlob)
        {
            output.diagnostics += static_cast<const char*>(diagnosticsBlob->getBufferPointer());
        }

        if (SLANG_SUCCEEDED(result) && codeBlob && codeBlob->getBufferSize() > 0)
        {
            const uint8* data = static_cast<const uint8*>(codeBlob->getBufferPointer());
            size_t size = codeBlob->getBufferSize();
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

        output.stages.push_back(std::move(stageOutput));
    }

    // 9. Extract reflection
    extractReflection(linkedProgram.get(), targetIndex, request, output.reflection);

    // Check if all requested stages produced valid output
    bool allValid = true;
    for (const auto& s : output.stages)
    {
        if (!s.isValid) { allValid = false; break; }
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
    if (!FileSystem::Open(filePath, EFileAccess::READ_ONLY, fileHandle))
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
    auto lastSlash = shaderName.find_last_of("/\\");
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
    request.shaderName = shaderName;
    request.sourceCode = sourceCode;
    request.sourceFilePath = filePath;
    request.includePaths.push_back(includeDir);
    request.requestedStages = requestedStages;

    return Compile(request);
}

// ============================
// Reflection extraction
// ============================

void ShaderCompiler::extractReflection(void* linkedProgramPtr, int targetIndex,
                                       const ShaderCompileRequest& request,
                                       ShaderReflectionDataEx& outReflection)
{
    auto* linkedProgram = static_cast<slang::IComponentType*>(linkedProgramPtr);
    slang::ProgramLayout* layout = linkedProgram->getLayout(targetIndex, nullptr);
    if (!layout)
    {
        HS_LOG(error, "[ShaderCompiler] Failed to get program layout for reflection");
        return;
    }

    // Extract global parameters (ConstantBuffers, textures, samplers)
    unsigned paramCount = layout->getParameterCount();
    for (unsigned i = 0; i < paramCount; ++i)
    {
        slang::VariableLayoutReflection* param = layout->getParameterByIndex(i);
        if (!param) continue;

        slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
        slang::TypeReflection* type = typeLayout->getType();
        auto kind = type->getKind();

        if (kind == slang::TypeReflection::Kind::ConstantBuffer ||
            kind == slang::TypeReflection::Kind::ParameterBlock)
        {
            ShaderBufferBindingInfo bufInfo;
            bufInfo.name = param->getName() ? param->getName() : "";
            bufInfo.binding = static_cast<uint32>(param->getBindingIndex());
            bufInfo.set = static_cast<uint32>(param->getBindingSpace());
            bufInfo.resourceType = EResourceType::UNIFORM_BUFFER;

            // Determine which stages use this buffer
            // For now, check all requested stages
            bufInfo.stages = request.requestedStages;

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
                    member.name = field->getName() ? field->getName() : "";
                    member.offset = static_cast<uint32>(field->getOffset(slang::ParameterCategory::Uniform));
                    member.size = static_cast<uint32>(field->getTypeLayout()->getSize(slang::ParameterCategory::Uniform));
                    member.nameHash = StringHash(member.name);
                    bufInfo.members.push_back(std::move(member));
                }
            }

            outReflection.bufferBindings.push_back(std::move(bufInfo));
        }
        else if (kind == slang::TypeReflection::Kind::Resource)
        {
            // Could be texture or sampler
            auto bindingType = typeLayout->getParameterCategory();

            if (bindingType == slang::ParameterCategory::ShaderResource)
            {
                ShaderTextureBindingInfo texInfo;
                texInfo.name = param->getName() ? param->getName() : "";
                texInfo.binding = static_cast<uint32>(param->getBindingIndex());
                texInfo.set = static_cast<uint32>(param->getBindingSpace());
                texInfo.stages = request.requestedStages;
                texInfo.dimension = 2; // Default to 2D
                outReflection.textureBindings.push_back(std::move(texInfo));
            }
            else if (bindingType == slang::ParameterCategory::SamplerState)
            {
                ShaderSamplerBindingInfo sampInfo;
                sampInfo.name = param->getName() ? param->getName() : "";
                sampInfo.binding = static_cast<uint32>(param->getBindingIndex());
                sampInfo.set = static_cast<uint32>(param->getBindingSpace());
                sampInfo.stages = request.requestedStages;
                outReflection.samplerBindings.push_back(std::move(sampInfo));
            }
        }
        else if (kind == slang::TypeReflection::Kind::SamplerState)
        {
            ShaderSamplerBindingInfo sampInfo;
            sampInfo.name = param->getName() ? param->getName() : "";
            sampInfo.binding = static_cast<uint32>(param->getBindingIndex());
            sampInfo.set = static_cast<uint32>(param->getBindingSpace());
            sampInfo.stages = request.requestedStages;
            outReflection.samplerBindings.push_back(std::move(sampInfo));
        }
    }

    // Extract entry point info and vertex inputs
    SlangUInt entryPointCount = layout->getEntryPointCount();
    for (SlangUInt ep = 0; ep < entryPointCount; ++ep)
    {
        slang::EntryPointReflection* epReflection = layout->getEntryPointByIndex(ep);
        if (!epReflection) continue;

        SlangStage slangStage = epReflection->getStage();
        EShaderStage hsStage = slangStageToHSStage(slangStage);

        const char* epName = epReflection->getName();
        if (hsStage == EShaderStage::VERTEX)
        {
            outReflection.vertexEntryPoint = epName ? epName : "VertexMain";
            extractVertexInputFromEntryPoint(epReflection, outReflection.vertexInput);
        }
        else if (hsStage == EShaderStage::FRAGMENT)
        {
            outReflection.fragmentEntryPoint = epName ? epName : "FragmentMain";
        }
        else if (hsStage == EShaderStage::COMPUTE)
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
                slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
                slang::TypeLayoutReflection* elementLayout = typeLayout->getElementTypeLayout();

                ShaderBufferBindingInfo bufInfo;
                bufInfo.name = param->getName() ? param->getName() : "";
                bufInfo.binding = static_cast<uint32>(param->getBindingIndex());
                bufInfo.set = static_cast<uint32>(param->getBindingSpace());
                bufInfo.stages = hsStage;
                bufInfo.resourceType = EResourceType::UNIFORM_BUFFER;

                if (elementLayout)
                {
                    bufInfo.totalSize = static_cast<uint32>(elementLayout->getSize(slang::ParameterCategory::Uniform));
                    unsigned fieldCount = elementLayout->getFieldCount();
                    for (unsigned f = 0; f < fieldCount; ++f)
                    {
                        slang::VariableLayoutReflection* field = elementLayout->getFieldByIndex(f);
                        if (!field) continue;

                        ShaderBufferMember member;
                        member.name = field->getName() ? field->getName() : "";
                        member.offset = static_cast<uint32>(field->getOffset(slang::ParameterCategory::Uniform));
                        member.size = static_cast<uint32>(field->getTypeLayout()->getSize(slang::ParameterCategory::Uniform));
                        member.nameHash = StringHash(member.name);
                        bufInfo.members.push_back(std::move(member));
                    }
                }

                outReflection.bufferBindings.push_back(std::move(bufInfo));
            }
        }
    }

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
        HS_LOG(info, "[ShaderCompiler]   Buffer '%s' @ binding %u, %u bytes, %zu members",
               buf.name.c_str(), buf.binding, buf.totalSize, buf.members.size());
    }
}

void ShaderCompiler::extractVertexInputFromEntryPoint(void* entryPointReflectionPtr,
                                                       ShaderVertexInputLayout& outLayout)
{
    auto* epReflection = static_cast<slang::EntryPointReflection*>(entryPointReflectionPtr);
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
        slang::TypeReflection* type = typeLayout->getType();
        auto kind = type->getKind();

        if (kind == slang::TypeReflection::Kind::Struct)
        {
            // VSInput struct: iterate its fields
            unsigned fieldCount = typeLayout->getFieldCount();
            for (unsigned f = 0; f < fieldCount; ++f)
            {
                slang::VariableLayoutReflection* field = typeLayout->getFieldByIndex(f);
                if (!field) continue;

                const char* semantic = field->getSemanticName();
                size_t semanticIdx = field->getSemanticIndex();

                if (!semantic) continue;

                slang::TypeLayoutReflection* fieldTypeLayout = field->getTypeLayout();
                slang::TypeReflection* fieldType = fieldTypeLayout->getType();

                ShaderVertexAttribute attr;
                attr.semantic = std::string(semantic) + std::to_string(semanticIdx);
                attr.location = static_cast<uint32>(field->getOffset(slang::ParameterCategory::VaryingInput));

                unsigned rows = fieldType->getRowCount();
                unsigned cols = fieldType->getColumnCount();
                auto scalarType = fieldType->getScalarType();

                attr.format = scalarTypeToVertexFormat(static_cast<int>(scalarType), rows, cols);
                attr.offset = currentOffset;

                // Calculate size based on format
                switch (attr.format)
                {
                case EVertexFormat::FLOAT:  attr.size = 4;  break;
                case EVertexFormat::FLOAT2: attr.size = 8;  break;
                case EVertexFormat::FLOAT3: attr.size = 12; break;
                case EVertexFormat::FLOAT4: attr.size = 16; break;
                case EVertexFormat::HALF:   attr.size = 2;  break;
                case EVertexFormat::HALF2:  attr.size = 4;  break;
                case EVertexFormat::HALF3:  attr.size = 6;  break;
                case EVertexFormat::HALF4:  attr.size = 8;  break;
                default: attr.size = 0; break;
                }

                currentOffset += attr.size;
                outLayout.attributes.push_back(std::move(attr));
            }
        }
        else
        {
            // Scalar/Vector type directly as vertex input
            const char* semantic = param->getSemanticName();
            size_t semanticIdx = param->getSemanticIndex();

            ShaderVertexAttribute attr;
            attr.semantic = semantic ? (std::string(semantic) + std::to_string(semanticIdx)) : "";
            attr.location = static_cast<uint32>(param->getOffset(slang::ParameterCategory::VaryingInput));

            unsigned rows = type->getRowCount();
            unsigned cols = type->getColumnCount();
            auto scalarType = type->getScalarType();

            attr.format = scalarTypeToVertexFormat(static_cast<int>(scalarType), rows, cols);
            attr.offset = currentOffset;

            switch (attr.format)
            {
            case EVertexFormat::FLOAT:  attr.size = 4;  break;
            case EVertexFormat::FLOAT2: attr.size = 8;  break;
            case EVertexFormat::FLOAT3: attr.size = 12; break;
            case EVertexFormat::FLOAT4: attr.size = 16; break;
            case EVertexFormat::HALF:   attr.size = 2;  break;
            case EVertexFormat::HALF2:  attr.size = 4;  break;
            case EVertexFormat::HALF3:  attr.size = 6;  break;
            case EVertexFormat::HALF4:  attr.size = 8;  break;
            default: attr.size = 0; break;
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
        case 1: return EVertexFormat::FLOAT;
        case 2: return EVertexFormat::FLOAT2;
        case 3: return EVertexFormat::FLOAT3;
        case 4: return EVertexFormat::FLOAT4;
        default: return EVertexFormat::INVALID;
        }
    case slang::TypeReflection::ScalarType::Float16:
        switch (componentCount)
        {
        case 1: return EVertexFormat::HALF;
        case 2: return EVertexFormat::HALF2;
        case 3: return EVertexFormat::HALF3;
        case 4: return EVertexFormat::HALF4;
        default: return EVertexFormat::INVALID;
        }
    default:
        return EVertexFormat::INVALID;
    }
}

HS_NS_END
