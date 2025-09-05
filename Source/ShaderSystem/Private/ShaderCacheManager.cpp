#include "ShaderSystem/ShaderCacheManager.h"
#include "Core/Log.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>

HS_NS_BEGIN

ShaderCacheManager::ShaderCacheManager()
{
}

ShaderCacheManager::~ShaderCacheManager()
{
    Shutdown();
}

bool ShaderCacheManager::Initialize(const std::string& cacheDirectory)
{
    if (_initialized)
    {
        return true;
    }

    if (cacheDirectory.empty())
    {
        _cacheDirectory = std::filesystem::current_path().string() + "/ShaderCache";
    }
    else
    {
        _cacheDirectory = cacheDirectory;
    }

    std::error_code ec;
    if (!std::filesystem::create_directories(_cacheDirectory, ec))
    {
        if (ec)
        {
            HS_LOG(crash, "Failed to create cache directory {}: {}", _cacheDirectory, ec.message());
            return false;
        }
    }

    _compiler = std::make_unique<ShaderCrossCompiler>();
    if (!_compiler->Initialize())
    {
        HS_LOG(crash, "Failed to initialize ShaderCrossCompiler");
        return false;
    }

    LoadCacheFromDisk();

    _initialized = true;
    HS_LOG(info, "ShaderCacheManager initialized with cache directory: {}", _cacheDirectory);
    return true;
}

void ShaderCacheManager::Shutdown()
{
    if (!_initialized)
    {
        return;
    }

    SaveCacheToDisk();

    if (_compiler)
    {
        _compiler->Shutdown();
        _compiler.reset();
    }

    _cache.clear();
    _initialized = false;

    HS_LOG(info, "ShaderCacheManager shut down");
}

CompiledShader ShaderCacheManager::GetOrCompileShader(
    const std::string& sourceCode,
    const std::string& filename,
    const ShaderCompileOptions& options)
{
    if (!_initialized)
    {
        HS_LOG(error, "ShaderCacheManager not initialized");
        return {};
    }

    uint64 hash = GenerateShaderHash(sourceCode, filename, options);
    
    auto it = _cache.find(hash);
    if (it != _cache.end())
    {
        if (IsShaderCacheValid(it->second, sourceCode))
        {
            HS_LOG(info, "Using cached shader: {}", filename);
            return it->second.shader;
        }
        else
        {
            HS_LOG(info, "Cache invalid, recompiling shader: {}", filename);
            _cache.erase(it);
        }
    }

    CompiledShader shader = _compiler->CompileShader(sourceCode, filename, options);
    
    if (shader.isValid)
    {
        CachedShaderEntry entry;
        entry.shader = shader;
        entry.timestamp = GetCurrentTimestamp();
        entry.sourceFilePath = filename;
        entry.sourceHash = HashString(sourceCode);
        entry.options = options;
        
        _cache[hash] = entry;
        HS_LOG(info, "Cached compiled shader: {}", filename);
    }

    return shader;
}

CompiledShader ShaderCacheManager::GetOrCompileShaderFromFile(
    const std::string& filepath,
    const ShaderCompileOptions& options)
{
    if (!_initialized)
    {
        HS_LOG(error, "ShaderCacheManager not initialized");
        return {};
    }

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        HS_LOG(error, "Failed to open shader file: {}", filepath);
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string sourceCode = buffer.str();
    return GetOrCompileShader(sourceCode, filepath, options);
}

void ShaderCacheManager::InvalidateCache()
{
    _cache.clear();
    HS_LOG(info, "Shader cache invalidated");
}

void ShaderCacheManager::InvalidateShader(const std::string& filename)
{
    auto it = _cache.begin();
    while (it != _cache.end())
    {
        if (it->second.sourceFilePath == filename)
        {
            it = _cache.erase(it);
            HS_LOG(info, "Invalidated cached shader: {}", filename);
        }
        else
        {
            ++it;
        }
    }
}

bool ShaderCacheManager::SaveCacheToDisk()
{
    if (_cacheDirectory.empty())
    {
        return false;
    }

    std::string cacheFilePath = GetCacheFilePath();
    
    std::ofstream file(cacheFilePath, std::ios::binary);
    if (!file.is_open())
    {
        HS_LOG(error, "Failed to open cache file for writing: {}", cacheFilePath);
        return false;
    }

    uint64 cacheSize = _cache.size();
    file.write(reinterpret_cast<const char*>(&cacheSize), sizeof(cacheSize));
    if (file.fail())
    {
        HS_LOG(error, "Failed to write cache size to file: {}", cacheFilePath);
        return false;
    }

    for (const auto& pair : _cache)
    {
        uint64 hash = pair.first;
        file.write(reinterpret_cast<const char*>(&hash), sizeof(hash));
        if (file.fail())
        {
            HS_LOG(error, "Failed to write hash to cache file");
            return false;
        }
        
        std::string serializedEntry = SerializeCacheEntry(pair.second);
        uint64 entrySize = serializedEntry.size();
        file.write(reinterpret_cast<const char*>(&entrySize), sizeof(entrySize));
        file.write(serializedEntry.data(), entrySize);
        if (file.fail())
        {
            HS_LOG(error, "Failed to write entry to cache file");
            return false;
        }
    }

    file.close();
    HS_LOG(info, "Saved {} cached shaders to disk", cacheSize);
    return true;
}

bool ShaderCacheManager::LoadCacheFromDisk()
{
    std::string cacheFilePath = GetCacheFilePath();
    
    if (!std::filesystem::exists(cacheFilePath))
    {
        HS_LOG(info, "No cache file found, starting with empty cache");
        return true;
    }

    std::ifstream file(cacheFilePath, std::ios::binary);
    if (!file.is_open())
    {
        HS_LOG(error, "Failed to open cache file for reading: {}", cacheFilePath);
        return false;
    }

    uint64 cacheSize;
    file.read(reinterpret_cast<char*>(&cacheSize), sizeof(cacheSize));
    if (file.fail() || file.gcount() != sizeof(cacheSize))
    {
        HS_LOG(error, "Failed to read cache size from file: {}", cacheFilePath);
        _cache.clear();
        return false;
    }

    _cache.reserve(static_cast<size_t>(cacheSize));

    for (uint64 i = 0; i < cacheSize; ++i)
    {
        uint64 hash;
        file.read(reinterpret_cast<char*>(&hash), sizeof(hash));
        if (file.fail() || file.gcount() != sizeof(hash))
        {
            HS_LOG(error, "Failed to read hash from cache file at entry {}", i);
            _cache.clear();
            return false;
        }
        
        uint64 entrySize;
        file.read(reinterpret_cast<char*>(&entrySize), sizeof(entrySize));
        if (file.fail() || file.gcount() != sizeof(entrySize))
        {
            HS_LOG(error, "Failed to read entry size from cache file at entry {}", i);
            _cache.clear();
            return false;
        }
        
        std::string serializedData(entrySize, '\0');
        file.read(serializedData.data(), entrySize);
        if (file.fail() || static_cast<uint64>(file.gcount()) != entrySize)
        {
            HS_LOG(error, "Failed to read entry data from cache file at entry {}", i);
            _cache.clear();
            return false;
        }
        
        CachedShaderEntry entry = DeserializeCacheEntry(serializedData);
        _cache[hash] = entry;
    }

    file.close();
    HS_LOG(info, "Loaded {} cached shaders from disk", cacheSize);
    return true;
}

uint64 ShaderCacheManager::GenerateShaderHash(
    const std::string& sourceCode,
    const std::string& filename,
    const ShaderCompileOptions& options)
{
    std::stringstream ss;
    ss << filename;
    ss << static_cast<uint32>(options.stage);
    ss << static_cast<uint32>(options.targetLanguage);
    ss << options.entryPoint;
    ss << options.enableDebugInfo;
    ss << options.enableOptimization;
    
    for (const auto& define : options.defines)
    {
        ss << define;
    }
    
    for (const auto& includePath : options.includePaths)
    {
        ss << includePath;
    }
    
    ss << sourceCode;
    
    return HashString(ss.str());
}

std::string ShaderCacheManager::GetCacheFilePath() const
{
    return _cacheDirectory + "/shader_cache.bin";
}

std::string ShaderCacheManager::SerializeCacheEntry(const CachedShaderEntry& entry)
{
    std::stringstream ss;
    
    ss.write(reinterpret_cast<const char*>(&entry.timestamp), sizeof(entry.timestamp));
    ss.write(reinterpret_cast<const char*>(&entry.sourceHash), sizeof(entry.sourceHash));
    
    uint64 pathSize = entry.sourceFilePath.size();
    ss.write(reinterpret_cast<const char*>(&pathSize), sizeof(pathSize));
    ss.write(entry.sourceFilePath.data(), pathSize);
    
    ss.write(reinterpret_cast<const char*>(&entry.options.stage), sizeof(entry.options.stage));
    ss.write(reinterpret_cast<const char*>(&entry.options.targetLanguage), sizeof(entry.options.targetLanguage));
    
    uint64 entryPointSize = entry.options.entryPoint.size();
    ss.write(reinterpret_cast<const char*>(&entryPointSize), sizeof(entryPointSize));
    ss.write(entry.options.entryPoint.data(), entryPointSize);
    
    ss.write(reinterpret_cast<const char*>(&entry.options.enableDebugInfo), sizeof(entry.options.enableDebugInfo));
    ss.write(reinterpret_cast<const char*>(&entry.options.enableOptimization), sizeof(entry.options.enableOptimization));
    
    uint64 spirvSize = entry.shader.spirvBytecode.size();
    ss.write(reinterpret_cast<const char*>(&spirvSize), sizeof(spirvSize));
    ss.write(reinterpret_cast<const char*>(entry.shader.spirvBytecode.data()), spirvSize * sizeof(uint32));
    
    uint64 compiledSourceSize = entry.shader.compiledSource.size();
    ss.write(reinterpret_cast<const char*>(&compiledSourceSize), sizeof(compiledSourceSize));
    ss.write(entry.shader.compiledSource.data(), compiledSourceSize);
    
    ss.write(reinterpret_cast<const char*>(&entry.shader.hash), sizeof(entry.shader.hash));
    ss.write(reinterpret_cast<const char*>(&entry.shader.isValid), sizeof(entry.shader.isValid));
    
    return ss.str();
}

CachedShaderEntry ShaderCacheManager::DeserializeCacheEntry(const std::string& data)
{
    CachedShaderEntry entry;
    std::istringstream ss(data);
    
    ss.read(reinterpret_cast<char*>(&entry.timestamp), sizeof(entry.timestamp));
    ss.read(reinterpret_cast<char*>(&entry.sourceHash), sizeof(entry.sourceHash));
    
    uint64 pathSize;
    ss.read(reinterpret_cast<char*>(&pathSize), sizeof(pathSize));
    entry.sourceFilePath.resize(pathSize);
    ss.read(entry.sourceFilePath.data(), pathSize);
    
    ss.read(reinterpret_cast<char*>(&entry.options.stage), sizeof(entry.options.stage));
    ss.read(reinterpret_cast<char*>(&entry.options.targetLanguage), sizeof(entry.options.targetLanguage));
    
    uint64 entryPointSize;
    ss.read(reinterpret_cast<char*>(&entryPointSize), sizeof(entryPointSize));
    entry.options.entryPoint.resize(entryPointSize);
    ss.read(entry.options.entryPoint.data(), entryPointSize);
    
    ss.read(reinterpret_cast<char*>(&entry.options.enableDebugInfo), sizeof(entry.options.enableDebugInfo));
    ss.read(reinterpret_cast<char*>(&entry.options.enableOptimization), sizeof(entry.options.enableOptimization));
    
    uint64 spirvSize;
    ss.read(reinterpret_cast<char*>(&spirvSize), sizeof(spirvSize));
    entry.shader.spirvBytecode.resize(spirvSize);
    ss.read(reinterpret_cast<char*>(entry.shader.spirvBytecode.data()), spirvSize * sizeof(uint32));
    
    uint64 compiledSourceSize;
    ss.read(reinterpret_cast<char*>(&compiledSourceSize), sizeof(compiledSourceSize));
    entry.shader.compiledSource.resize(compiledSourceSize);
    ss.read(entry.shader.compiledSource.data(), compiledSourceSize);
    
    ss.read(reinterpret_cast<char*>(&entry.shader.hash), sizeof(entry.shader.hash));
    ss.read(reinterpret_cast<char*>(&entry.shader.isValid), sizeof(entry.shader.isValid));
    
    return entry;
}

bool ShaderCacheManager::IsShaderCacheValid(const CachedShaderEntry& entry, const std::string& currentSourceCode)
{
    uint64 currentHash = HashString(currentSourceCode);
    return entry.sourceHash == currentHash;
}

uint64 ShaderCacheManager::GetCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

uint64 ShaderCacheManager::HashString(const std::string& str)
{
    uint64 hash = 14695981039346656037ULL;
    for (char c : str)
    {
        hash ^= static_cast<uint64>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

HS_NS_END