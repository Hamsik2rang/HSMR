//
//  ShaderCache.h
//  HSMR
//
//  Bytecode + reflection disk caching.
//
#ifndef __HS_SHADER_CACHE_H__
#define __HS_SHADER_CACHE_H__

#include "Precompile.h"
#include "ShaderSystem/ShaderSystemDefinition.h"

#include <string>

HS_NS_BEGIN

class HS_SHADER_SYSTEM_API ShaderCache
{
public:
    ShaderCache();
    ~ShaderCache();

    bool Initialize(const std::string& cacheDirectory);
    void Finalize();

    bool HasValidCache(const std::string& shaderName, uint64 sourceHash) const;
    bool Store(const std::string& shaderName, uint64 sourceHash, const ShaderCompileOutputEx& output);
    bool Load(const std::string& shaderName, uint64 sourceHash, ShaderCompileOutputEx& outOutput);

    const std::string& GetCacheDirectory() const { return _cacheDirectory; }

private:
    std::string getCachePath(const std::string& shaderName) const;

    // Binary serialization helpers
    bool serializeOutput(const std::string& path, uint64 sourceHash, const ShaderCompileOutputEx& output);
    bool deserializeOutput(const std::string& path, uint64 sourceHash, ShaderCompileOutputEx& outOutput);

    std::string _cacheDirectory;
    bool _initialized = false;

    static constexpr uint32 CACHE_MAGIC = 0x48535343;   // "HSSC"
    static constexpr uint32 CACHE_VERSION = 1;
};

HS_NS_END

#endif
