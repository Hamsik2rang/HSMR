//
//  ShaderWatcher.h
//  HSMR
//
//  File watcher for shader hot reload
//
#ifndef __HS_APPLICATION_SHADER_WATCHER_H__
#define __HS_APPLICATION_SHADER_WATCHER_H__

#include "Precompile.h"
#include <string>
#include <vector>
#include <functional>
#include <filesystem>

// Forward declarations
struct RHIShader;

HS_NS_BEGIN

// Shader reload callback
using ShaderReloadCallback = std::function<void(const std::string& path, RHIShader* newShader)>;

// Watch entry for tracking a shader file
struct HS_APPLICATION_API ShaderWatchEntry
{
    std::string path;
    std::string name;
    RHIShader* shader = nullptr;
    std::filesystem::file_time_type lastModified;
    bool needsReload = false;
};

// Shader watcher for hot reload
class HS_APPLICATION_API ShaderWatcher
{
public:
    ShaderWatcher();
    ~ShaderWatcher();

    // Add a shader to watch
    void Watch(const std::string& shaderPath, RHIShader* shader, const std::string& name = "");

    // Remove a shader from watch
    void Unwatch(const std::string& shaderPath);

    // Clear all watches
    void ClearAll();

    // Update - checks for file changes
    // Call once per frame with deltaTime in seconds
    void Update(float deltaTime);

    // Set reload callback
    void SetReloadCallback(ShaderReloadCallback callback) { _reloadCallback = callback; }

    // Settings
    void SetCheckInterval(float seconds) { _checkInterval = seconds; }
    float GetCheckInterval() const { return _checkInterval; }

    void SetEnabled(bool enabled) { _enabled = enabled; }
    bool IsEnabled() const { return _enabled; }

    // Manual reload
    void ReloadShader(const std::string& path);
    void ReloadAll();

    // Status
    size_t GetWatchCount() const { return _watchList.size(); }
    const std::vector<ShaderWatchEntry>& GetWatchList() const { return _watchList; }

    // Get list of shaders that need reload
    std::vector<std::string> GetPendingReloads() const;

private:
    bool checkFileModified(ShaderWatchEntry& entry);
    void recompileShader(ShaderWatchEntry& entry);

    std::vector<ShaderWatchEntry> _watchList;
    ShaderReloadCallback _reloadCallback;

    float _checkInterval = 0.5f;  // Check every 0.5 seconds
    float _timeSinceLastCheck = 0.0f;
    bool _enabled = true;
};

HS_NS_END

#endif // __HS_APPLICATION_SHADER_WATCHER_H__
