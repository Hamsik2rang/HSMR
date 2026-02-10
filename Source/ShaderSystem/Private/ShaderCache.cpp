#include "ShaderSystem/ShaderCache.h"

#include "Core/Log.h"
#include "Core/Hash.h"
#include "Core/HAL/FileSystem.h"

#include <fstream>

HS_NS_BEGIN

ShaderCache::ShaderCache()
{
}

ShaderCache::~ShaderCache()
{
    Finalize();
}

bool ShaderCache::Initialize(const std::string& cacheDirectory)
{
    _cacheDirectory = cacheDirectory;
    // Ensure trailing separator
    if (!_cacheDirectory.empty() && _cacheDirectory.back() != HS_DIR_SEPERATOR)
    {
        _cacheDirectory += HS_DIR_SEPERATOR;
    }
    _initialized = true;
    HS_LOG(info, "[ShaderCache] Initialized at: %s", _cacheDirectory.c_str());
    return true;
}

void ShaderCache::Finalize()
{
    _initialized = false;
}

std::string ShaderCache::getCachePath(const std::string& shaderName) const
{
    return _cacheDirectory + shaderName + ".hssc";
}

bool ShaderCache::HasValidCache(const std::string& shaderName, uint64 sourceHash) const
{
    if (!_initialized) return false;

    std::string path = getCachePath(shaderName);
    if (!FileSystem::Exist(path)) return false;

    // Quick check: read just the header to verify magic, version, and hash
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    uint32 magic, version;
    uint64 storedHash;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    file.read(reinterpret_cast<char*>(&storedHash), sizeof(storedHash));

    return magic == CACHE_MAGIC && version == CACHE_VERSION && storedHash == sourceHash;
}

bool ShaderCache::Store(const std::string& shaderName, uint64 sourceHash, const ShaderCompileOutputEx& output)
{
    if (!_initialized || !output.isValid) return false;
    return serializeOutput(getCachePath(shaderName), sourceHash, output);
}

bool ShaderCache::Load(const std::string& shaderName, uint64 sourceHash, ShaderCompileOutputEx& outOutput)
{
    if (!_initialized) return false;
    return deserializeOutput(getCachePath(shaderName), sourceHash, outOutput);
}

// Helper: write POD type to stream
template <typename T>
static void writeVal(std::ofstream& f, const T& val)
{
    f.write(reinterpret_cast<const char*>(&val), sizeof(T));
}

// Helper: write string to stream (length-prefixed)
static void writeStr(std::ofstream& f, const std::string& s)
{
    uint32 len = static_cast<uint32>(s.size());
    f.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) f.write(s.data(), len);
}

// Helper: write vector<uint8> to stream
static void writeBytes(std::ofstream& f, const std::vector<uint8>& data)
{
    uint32 len = static_cast<uint32>(data.size());
    f.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) f.write(reinterpret_cast<const char*>(data.data()), len);
}

// Helper: read POD type
template <typename T>
static bool readVal(std::ifstream& f, T& val)
{
    f.read(reinterpret_cast<char*>(&val), sizeof(T));
    return f.good();
}

// Helper: read string
static bool readStr(std::ifstream& f, std::string& s)
{
    uint32 len;
    if (!readVal(f, len)) return false;
    s.resize(len);
    if (len > 0) f.read(s.data(), len);
    return f.good() || f.eof();
}

// Helper: read vector<uint8>
static bool readBytes(std::ifstream& f, std::vector<uint8>& data)
{
    uint32 len;
    if (!readVal(f, len)) return false;
    data.resize(len);
    if (len > 0) f.read(reinterpret_cast<char*>(data.data()), len);
    return f.good() || f.eof();
}

bool ShaderCache::serializeOutput(const std::string& path, uint64 sourceHash, const ShaderCompileOutputEx& output)
{
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        HS_LOG(error, "[ShaderCache] Failed to open for writing: %s", path.c_str());
        return false;
    }

    // Header
    writeVal(file, CACHE_MAGIC);
    writeVal(file, CACHE_VERSION);
    writeVal(file, sourceHash);

    // Stage outputs
    uint32 stageCount = static_cast<uint32>(output.stages.size());
    writeVal(file, stageCount);
    for (const auto& stage : output.stages)
    {
        writeVal(file, static_cast<uint32>(stage.stage));
        writeBytes(file, stage.bytecode);
        writeStr(file, stage.entryPoint);
        writeVal(file, static_cast<uint32>(stage.language));
        writeVal(file, stage.isValid);
    }

    // Reflection data
    const auto& refl = output.reflection;

    // Buffer bindings
    uint32 bufCount = static_cast<uint32>(refl.bufferBindings.size());
    writeVal(file, bufCount);
    for (const auto& buf : refl.bufferBindings)
    {
        writeStr(file, buf.name);
        writeVal(file, buf.nameHash);
        writeVal(file, buf.set);
        writeVal(file, buf.binding);
        writeVal(file, buf.totalSize);
        writeVal(file, static_cast<uint32>(buf.stages));
        writeVal(file, static_cast<uint8>(buf.resourceType));

        uint32 memberCount = static_cast<uint32>(buf.members.size());
        writeVal(file, memberCount);
        for (const auto& member : buf.members)
        {
            writeStr(file, member.name);
            writeVal(file, member.offset);
            writeVal(file, member.size);
            writeVal(file, member.nameHash);
        }
    }

    // Texture bindings
    uint32 texCount = static_cast<uint32>(refl.textureBindings.size());
    writeVal(file, texCount);
    for (const auto& tex : refl.textureBindings)
    {
        writeStr(file, tex.name);
        writeVal(file, tex.nameHash);
        writeVal(file, tex.set);
        writeVal(file, tex.binding);
        writeVal(file, static_cast<uint32>(tex.stages));
        writeVal(file, tex.dimension);
    }

    // Sampler bindings
    uint32 sampCount = static_cast<uint32>(refl.samplerBindings.size());
    writeVal(file, sampCount);
    for (const auto& samp : refl.samplerBindings)
    {
        writeStr(file, samp.name);
        writeVal(file, samp.nameHash);
        writeVal(file, samp.set);
        writeVal(file, samp.binding);
        writeVal(file, static_cast<uint32>(samp.stages));
    }

    // Vertex input layout
    uint32 attrCount = static_cast<uint32>(refl.vertexInput.attributes.size());
    writeVal(file, attrCount);
    for (const auto& attr : refl.vertexInput.attributes)
    {
        writeStr(file, attr.semantic);
        writeVal(file, attr.location);
        writeVal(file, static_cast<uint32>(attr.format));
        writeVal(file, attr.offset);
        writeVal(file, attr.size);
    }
    writeVal(file, refl.vertexInput.stride);

    // Entry points
    writeStr(file, refl.vertexEntryPoint);
    writeStr(file, refl.fragmentEntryPoint);
    writeStr(file, refl.computeEntryPoint);

    HS_LOG(info, "[ShaderCache] Stored cache: %s", path.c_str());
    return true;
}

bool ShaderCache::deserializeOutput(const std::string& path, uint64 sourceHash, ShaderCompileOutputEx& outOutput)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    // Header
    uint32 magic, version;
    uint64 storedHash;
    if (!readVal(file, magic) || magic != CACHE_MAGIC) return false;
    if (!readVal(file, version) || version != CACHE_VERSION) return false;
    if (!readVal(file, storedHash) || storedHash != sourceHash) return false;

    // Stage outputs
    uint32 stageCount;
    if (!readVal(file, stageCount)) return false;
    outOutput.stages.resize(stageCount);
    for (uint32 i = 0; i < stageCount; ++i)
    {
        auto& stage = outOutput.stages[i];
        uint32 stageVal;
        if (!readVal(file, stageVal)) return false;
        stage.stage = static_cast<EShaderStage>(stageVal);
        if (!readBytes(file, stage.bytecode)) return false;
        if (!readStr(file, stage.entryPoint)) return false;
        uint32 langVal;
        if (!readVal(file, langVal)) return false;
        stage.language = static_cast<EShaderLanguage>(langVal);
        if (!readVal(file, stage.isValid)) return false;
    }

    // Reflection
    auto& refl = outOutput.reflection;

    // Buffer bindings
    uint32 bufCount;
    if (!readVal(file, bufCount)) return false;
    refl.bufferBindings.resize(bufCount);
    for (uint32 i = 0; i < bufCount; ++i)
    {
        auto& buf = refl.bufferBindings[i];
        if (!readStr(file, buf.name)) return false;
        if (!readVal(file, buf.nameHash)) return false;
        if (!readVal(file, buf.set)) return false;
        if (!readVal(file, buf.binding)) return false;
        if (!readVal(file, buf.totalSize)) return false;
        uint32 stagesVal;
        if (!readVal(file, stagesVal)) return false;
        buf.stages = static_cast<EShaderStage>(stagesVal);
        uint8 resTypeVal;
        if (!readVal(file, resTypeVal)) return false;
        buf.resourceType = static_cast<EResourceType>(resTypeVal);

        uint32 memberCount;
        if (!readVal(file, memberCount)) return false;
        buf.members.resize(memberCount);
        for (uint32 m = 0; m < memberCount; ++m)
        {
            auto& member = buf.members[m];
            if (!readStr(file, member.name)) return false;
            if (!readVal(file, member.offset)) return false;
            if (!readVal(file, member.size)) return false;
            if (!readVal(file, member.nameHash)) return false;
        }
    }

    // Texture bindings
    uint32 texCount;
    if (!readVal(file, texCount)) return false;
    refl.textureBindings.resize(texCount);
    for (uint32 i = 0; i < texCount; ++i)
    {
        auto& tex = refl.textureBindings[i];
        if (!readStr(file, tex.name)) return false;
        if (!readVal(file, tex.nameHash)) return false;
        if (!readVal(file, tex.set)) return false;
        if (!readVal(file, tex.binding)) return false;
        uint32 stagesVal;
        if (!readVal(file, stagesVal)) return false;
        tex.stages = static_cast<EShaderStage>(stagesVal);
        if (!readVal(file, tex.dimension)) return false;
    }

    // Sampler bindings
    uint32 sampCount;
    if (!readVal(file, sampCount)) return false;
    refl.samplerBindings.resize(sampCount);
    for (uint32 i = 0; i < sampCount; ++i)
    {
        auto& samp = refl.samplerBindings[i];
        if (!readStr(file, samp.name)) return false;
        if (!readVal(file, samp.nameHash)) return false;
        if (!readVal(file, samp.set)) return false;
        if (!readVal(file, samp.binding)) return false;
        uint32 stagesVal;
        if (!readVal(file, stagesVal)) return false;
        samp.stages = static_cast<EShaderStage>(stagesVal);
    }

    // Vertex input layout
    uint32 attrCount;
    if (!readVal(file, attrCount)) return false;
    refl.vertexInput.attributes.resize(attrCount);
    for (uint32 i = 0; i < attrCount; ++i)
    {
        auto& attr = refl.vertexInput.attributes[i];
        if (!readStr(file, attr.semantic)) return false;
        if (!readVal(file, attr.location)) return false;
        uint32 formatVal;
        if (!readVal(file, formatVal)) return false;
        attr.format = static_cast<EVertexFormat>(formatVal);
        if (!readVal(file, attr.offset)) return false;
        if (!readVal(file, attr.size)) return false;
    }
    if (!readVal(file, refl.vertexInput.stride)) return false;

    // Entry points
    if (!readStr(file, refl.vertexEntryPoint)) return false;
    if (!readStr(file, refl.fragmentEntryPoint)) return false;
    if (!readStr(file, refl.computeEntryPoint)) return false;

    // Rebuild lookup
    refl.BuildLookup();
    refl.isValid = true;

    outOutput.isValid = true;
    HS_LOG(info, "[ShaderCache] Loaded cache: %s", path.c_str());
    return true;
}

HS_NS_END
