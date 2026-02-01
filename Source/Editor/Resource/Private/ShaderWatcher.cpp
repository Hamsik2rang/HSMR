//
//  ShaderWatcher.cpp
//  HSMR
//
//  File watcher for shader hot reload
//
#include "Editor/Resource/ShaderWatcher.h"
#include "Core/Log.h"

#include <fstream>

HS_NS_BEGIN

ShaderWatcher::ShaderWatcher()
{
}

ShaderWatcher::~ShaderWatcher()
{
    ClearAll();
}

void ShaderWatcher::Watch(const std::string& shaderPath, RHIShader* shader, const std::string& name)
{
    // Check if already watching
    for (auto& entry : _watchList)
    {
        if (entry.path == shaderPath)
        {
            // Update existing entry
            entry.shader = shader;
            entry.name = name.empty() ? shaderPath : name;
            return;
        }
    }

    // Add new entry
    ShaderWatchEntry entry;
    entry.path = shaderPath;
    entry.name = name.empty() ? shaderPath : name;
    entry.shader = shader;
    entry.needsReload = false;

    // Get initial modification time
    std::error_code ec;
    if (std::filesystem::exists(shaderPath, ec))
    {
        entry.lastModified = std::filesystem::last_write_time(shaderPath, ec);
    }

    _watchList.push_back(std::move(entry));
    HS_LOG(info, "Watching shader: %s", shaderPath.c_str());
}

void ShaderWatcher::Unwatch(const std::string& shaderPath)
{
    auto it = std::remove_if(_watchList.begin(), _watchList.end(),
        [&shaderPath](const ShaderWatchEntry& entry) {
            return entry.path == shaderPath;
        });

    if (it != _watchList.end())
    {
        _watchList.erase(it, _watchList.end());
        HS_LOG(info, "Stopped watching shader: %s", shaderPath.c_str());
    }
}

void ShaderWatcher::ClearAll()
{
    _watchList.clear();
}

void ShaderWatcher::Update(float deltaTime)
{
    if (!_enabled) return;

    _timeSinceLastCheck += deltaTime;

    if (_timeSinceLastCheck < _checkInterval)
    {
        return;
    }

    _timeSinceLastCheck = 0.0f;

    // Check all watched files
    for (auto& entry : _watchList)
    {
        if (checkFileModified(entry))
        {
            recompileShader(entry);
        }
    }
}

void ShaderWatcher::ReloadShader(const std::string& path)
{
    for (auto& entry : _watchList)
    {
        if (entry.path == path)
        {
            recompileShader(entry);
            return;
        }
    }

    HS_LOG(warning, "Shader not in watch list: %s", path.c_str());
}

void ShaderWatcher::ReloadAll()
{
    for (auto& entry : _watchList)
    {
        recompileShader(entry);
    }
}

std::vector<std::string> ShaderWatcher::GetPendingReloads() const
{
    std::vector<std::string> pending;

    for (const auto& entry : _watchList)
    {
        if (entry.needsReload)
        {
            pending.push_back(entry.path);
        }
    }

    return pending;
}

bool ShaderWatcher::checkFileModified(ShaderWatchEntry& entry)
{
    std::error_code ec;

    if (!std::filesystem::exists(entry.path, ec))
    {
        return false;
    }

    auto currentTime = std::filesystem::last_write_time(entry.path, ec);
    if (ec)
    {
        return false;
    }

    if (currentTime != entry.lastModified)
    {
        entry.lastModified = currentTime;
        entry.needsReload = true;
        HS_LOG(info, "Shader file changed: %s", entry.path.c_str());
        return true;
    }

    return false;
}

void ShaderWatcher::recompileShader(ShaderWatchEntry& entry)
{
    HS_LOG(info, "Recompiling shader: %s", entry.path.c_str());

    // Note: Actual shader recompilation requires:
    // 1. Reading the shader source file
    // 2. Compiling with Slang/SPIRV-Cross
    // 3. Creating new RHI shader object
    // 4. Notifying dependent pipelines to recreate

    // For now, just invoke the callback if set
    if (_reloadCallback)
    {
        // Read shader source (for verification)
        std::ifstream file(entry.path);
        if (!file.is_open())
        {
            HS_LOG(error, "Failed to open shader file for reload: %s", entry.path.c_str());
            return;
        }

        // TODO: Actual shader compilation
        // This would typically involve:
        // - Slang compilation for cross-platform shaders
        // - SPIRV-Cross for shader reflection
        // - RHI shader object creation

        // For now, pass nullptr to indicate reload needed
        // The application should handle actual recompilation
        _reloadCallback(entry.path, nullptr);
    }

    entry.needsReload = false;
    HS_LOG(info, "Shader reload triggered: %s", entry.name.c_str());
}

HS_NS_END
