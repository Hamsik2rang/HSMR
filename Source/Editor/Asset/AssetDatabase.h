//
//  AssetDatabase.h
//  Editor
//
//  Asset database for managing project resources
//

#pragma once

#include "Precompile.h"
#include "Editor/Asset/AssetTypes.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

HS_NS_BEGIN
class Mesh;
class Material;
class Image;
class Model;
class Shader;
HS_NS_END

HS_NS_EDITOR_BEGIN

/**
 * @brief Entry representing a single asset file
 */
struct AssetEntry
{
    std::string relativePath;       // Relative path from Assets folder
    std::string absolutePath;       // Full absolute path
    std::string name;               // File name without path
    std::string extension;          // File extension
    EAssetType type = EAssetType::Unknown;
    uint64 lastModified = 0;        // File modification time
    uint64 fileSize = 0;            // File size in bytes
    bool isDirectory = false;       // Is this a folder?

    // Cached resource (lazy loaded)
    mutable void* cachedResource = nullptr;
    mutable bool isLoaded = false;
};

/**
 * @brief Folder entry for tree view
 */
struct FolderEntry
{
    std::string name;
    std::string relativePath;
    std::vector<FolderEntry> subFolders;
    std::vector<AssetEntry*> assets;
};

/**
 * @brief Asset database singleton for managing project assets
 */
class HS_EDITOR_API AssetDatabase
{
public:
    static AssetDatabase& Get();

    /**
     * @brief Initialize and scan the asset directory
     * @param rootPath Root path to scan (usually Assets folder)
     */
    void Initialize(const std::string& rootPath);

    /**
     * @brief Set root path without scanning
     */
    void SetRootPath(const std::string& rootPath) { _rootPath = rootPath; }

    /**
     * @brief Scan the current root path
     */
    void Scan();

    /**
     * @brief Shutdown and clear all cached resources
     */
    void Shutdown();
    void Finalize() { Shutdown(); }

    /**
     * @brief Refresh the database (rescan for changes)
     */
    void Refresh();

    /**
     * @brief Get root path
     */
    const std::string& GetRootPath() const { return _rootPath; }

    /**
     * @brief Get all assets
     */
    const std::unordered_map<std::string, AssetEntry>& GetAllAssets() const { return _assets; }

    /**
     * @brief Get assets by type
     */
    std::vector<const AssetEntry*> GetAssetsByType(EAssetType type) const;

    /**
     * @brief Get assets in a specific folder
     */
    std::vector<const AssetEntry*> GetAssetsInFolder(const std::string& folderPath) const;

    /**
     * @brief Get subfolders in a specific folder
     */
    std::vector<std::string> GetSubFolders(const std::string& folderPath) const;

    /**
     * @brief Find asset by relative path
     */
    const AssetEntry* FindAsset(const std::string& relativePath) const;

    /**
     * @brief Get folder tree structure
     */
    const FolderEntry& GetFolderTree() const { return _folderTree; }

    // Resource loading (with caching)
    hs::Mesh* LoadMesh(const std::string& relativePath);
    hs::Material* LoadMaterial(const std::string& relativePath);
    bool SaveMaterial(const std::string& relativePath, hs::Material* material);
    hs::Image* LoadTexture(const std::string& relativePath);
    hs::Model* LoadModel(const std::string& relativePath);

    /**
     * @brief Check if a path is a valid mesh file
     */
    bool IsMeshFile(const std::string& path) const;

    /**
     * @brief Check if a path is a valid texture file
     */
    bool IsTextureFile(const std::string& path) const;

private:
    AssetDatabase() = default;
    ~AssetDatabase() = default;
    AssetDatabase(const AssetDatabase&) = delete;
    AssetDatabase& operator=(const AssetDatabase&) = delete;

    void scanDirectory(const std::filesystem::path& path, const std::string& relativePath);
    void buildFolderTree();
    void buildFolderTreeRecursive(FolderEntry& folder, const std::string& path);

    std::string _rootPath;
    std::unordered_map<std::string, AssetEntry> _assets;
    std::unordered_map<std::string, std::vector<std::string>> _folderContents; // folder -> subfolders
    FolderEntry _folderTree;

    // Cached loaded resources (owned by ObjectManager, we just store pointers)
    std::unordered_map<std::string, hs::Scoped<hs::Model>> _loadedModels;
    std::unordered_map<std::string, hs::Scoped<hs::Material>> _loadedMaterials;
    std::unordered_map<std::string, hs::Scoped<hs::Image>> _loadedTextures;
};

HS_NS_EDITOR_END
