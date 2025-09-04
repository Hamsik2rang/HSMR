#ifndef __HS_SHADER_CACHE_MANAGER_H__
#define __HS_SHADER_CACHE_MANAGER_H__

#include "Precompile.h"
#include "ShaderCrossCompiler.h"
#include "Core/Hash.h"
#include <unordered_map>
#include <string>
#include <memory>

HS_NS_BEGIN

struct CachedShaderEntry
{
    CompiledShader shader;
    uint64 timestamp;
    std::string sourceFilePath;
    uint64 sourceHash;
    ShaderCompileOptions options;
};

class HS_SHADERSYSTEM_API ShaderCacheManager
{
public:
    ShaderCacheManager();
    ~ShaderCacheManager();

    bool Initialize(const std::string& cacheDirectory = "");
    void Shutdown();

    CompiledShader GetOrCompileShader(
        const std::string& sourceCode,
        const std::string& filename,
        const ShaderCompileOptions& options
    );

    CompiledShader GetOrCompileShaderFromFile(
        const std::string& filepath,
        const ShaderCompileOptions& options
    );

    void InvalidateCache();
    void InvalidateShader(const std::string& filename);

    bool SaveCacheToDisk();
    bool LoadCacheFromDisk();

    size_t GetCacheSize() const { return _cache.size(); }
    const std::string& GetCacheDirectory() const { return _cacheDirectory; }

private:
    std::unordered_map<uint64, CachedShaderEntry> _cache;
    std::string _cacheDirectory;
    std::unique_ptr<ShaderCrossCompiler> _compiler;
    bool _initialized = false;

    uint64 GenerateShaderHash(
        const std::string& sourceCode,
        const std::string& filename,
        const ShaderCompileOptions& options
    );

    std::string GetCacheFilePath() const;
    std::string SerializeCacheEntry(const CachedShaderEntry& entry);
    CachedShaderEntry DeserializeCacheEntry(const std::string& data);
    
    bool IsShaderCacheValid(const CachedShaderEntry& entry, const std::string& currentSourceCode);
    uint64 GetCurrentTimestamp();

    uint64 HashString(const std::string& str);
};

HS_NS_END

#endif