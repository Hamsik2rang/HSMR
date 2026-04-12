//
//  AssetDatabase.cpp
//  Editor
//
//  Asset database implementation
//

#include "Editor/Asset/AssetDatabase.h"
#include "Core/Log.h"
#include "Resource/ObjectManager.h"
#include "Resource/MaterialSerializer.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"
#include "Resource/Image.h"

#include <algorithm>

HS_NS_EDITOR_BEGIN

AssetDatabase& AssetDatabase::Get()
{
    static AssetDatabase instance;
    return instance;
}

void AssetDatabase::Initialize(const std::string& rootPath)
{
    SetRootPath(rootPath);
    Scan();
}

void AssetDatabase::Scan()
{
    // Normalize path separators
    for (auto& c : _rootPath)
    {
        if (c == '\\') c = '/';
    }

    // Remove trailing slash
    if (!_rootPath.empty() && _rootPath.back() == '/')
    {
        _rootPath.pop_back();
    }

    HS_LOG(info, "[AssetDatabase] Scanning root: %s", _rootPath.c_str());

    Refresh();
}

void AssetDatabase::Shutdown()
{
    _assets.clear();
    _folderContents.clear();
    _loadedModels.clear();
    _loadedMaterials.clear();
    _loadedTextures.clear();
    _folderTree = FolderEntry{};
}

void AssetDatabase::Refresh()
{
    _assets.clear();
    _folderContents.clear();

    if (_rootPath.empty())
    {
        HS_LOG(warning, "[AssetDatabase] Root path is empty, cannot refresh");
        return;
    }

    std::filesystem::path rootFsPath(_rootPath);
    if (!std::filesystem::exists(rootFsPath))
    {
        HS_LOG(warning, "[AssetDatabase] Root path does not exist: %s", _rootPath.c_str());
        return;
    }

    scanDirectory(rootFsPath, "");
    buildFolderTree();

    HS_LOG(info, "[AssetDatabase] Scanned %zu assets", _assets.size());
}

void AssetDatabase::scanDirectory(const std::filesystem::path& path, const std::string& relativePath)
{
    try
    {
        std::vector<std::string> subFolders;

        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            std::string fileName = entry.path().filename().string();

            // Skip hidden files and folders
            if (!fileName.empty() && fileName[0] == '.')
                continue;

            std::string entryRelativePath = relativePath.empty()
                ? fileName
                : relativePath + "/" + fileName;

            if (entry.is_directory())
            {
                subFolders.push_back(entryRelativePath);
                scanDirectory(entry.path(), entryRelativePath);
            }
            else if (entry.is_regular_file())
            {
                AssetEntry asset;
                asset.relativePath = entryRelativePath;
                asset.absolutePath = entry.path().string();
                asset.name = fileName;
                asset.extension = entry.path().extension().string();
                asset.type = GetAssetTypeFromExtension(asset.extension);
                asset.isDirectory = false;

                // Get file info
                auto fileTime = std::filesystem::last_write_time(entry);
                asset.lastModified = std::chrono::duration_cast<std::chrono::seconds>(
                    fileTime.time_since_epoch()).count();
                asset.fileSize = entry.file_size();

                _assets[entryRelativePath] = std::move(asset);
            }
        }

        _folderContents[relativePath] = std::move(subFolders);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        HS_LOG(error, "[AssetDatabase] Error scanning directory: %s", e.what());
    }
}

void AssetDatabase::buildFolderTree()
{
    _folderTree = FolderEntry{};
    _folderTree.name = "Assets";
    _folderTree.relativePath = "";

    buildFolderTreeRecursive(_folderTree, "");
}

void AssetDatabase::buildFolderTreeRecursive(FolderEntry& folder, const std::string& path)
{
    // Add subfolders
    auto it = _folderContents.find(path);
    if (it != _folderContents.end())
    {
        for (const auto& subFolderPath : it->second)
        {
            FolderEntry subFolder;
            size_t lastSlash = subFolderPath.rfind('/');
            subFolder.name = (lastSlash != std::string::npos)
                ? subFolderPath.substr(lastSlash + 1)
                : subFolderPath;
            subFolder.relativePath = subFolderPath;

            buildFolderTreeRecursive(subFolder, subFolderPath);
            folder.subFolders.push_back(std::move(subFolder));
        }
    }

    // Add assets in this folder
    for (auto& [assetPath, asset] : _assets)
    {
        // Check if asset is directly in this folder
        size_t lastSlash = assetPath.rfind('/');
        std::string assetFolder = (lastSlash != std::string::npos)
            ? assetPath.substr(0, lastSlash)
            : "";

        if (assetFolder == path)
        {
            folder.assets.push_back(&asset);
        }
    }

    // Sort subfolders and assets by name
    std::sort(folder.subFolders.begin(), folder.subFolders.end(),
        [](const FolderEntry& a, const FolderEntry& b) { return a.name < b.name; });

    std::sort(folder.assets.begin(), folder.assets.end(),
        [](const AssetEntry* a, const AssetEntry* b) { return a->name < b->name; });
}

std::vector<const AssetEntry*> AssetDatabase::GetAssetsByType(EAssetType type) const
{
    std::vector<const AssetEntry*> result;
    for (const auto& [path, asset] : _assets)
    {
        if (asset.type == type)
        {
            result.push_back(&asset);
        }
    }
    return result;
}

std::vector<const AssetEntry*> AssetDatabase::GetAssetsInFolder(const std::string& folderPath) const
{
    std::vector<const AssetEntry*> result;
    for (const auto& [assetPath, asset] : _assets)
    {
        size_t lastSlash = assetPath.rfind('/');
        std::string assetFolder = (lastSlash != std::string::npos)
            ? assetPath.substr(0, lastSlash)
            : "";

        if (assetFolder == folderPath)
        {
            result.push_back(&asset);
        }
    }

    // Sort by name
    std::sort(result.begin(), result.end(),
        [](const AssetEntry* a, const AssetEntry* b) { return a->name < b->name; });

    return result;
}

std::vector<std::string> AssetDatabase::GetSubFolders(const std::string& folderPath) const
{
    auto it = _folderContents.find(folderPath);
    if (it != _folderContents.end())
    {
        return it->second;
    }
    return {};
}

const AssetEntry* AssetDatabase::FindAsset(const std::string& relativePath) const
{
    auto it = _assets.find(relativePath);
    if (it != _assets.end())
    {
        return &it->second;
    }
    return nullptr;
}

hs::Mesh* AssetDatabase::LoadMesh(const std::string& relativePath)
{
    // For mesh loading, we need to load the whole model first
    hs::Model* model = LoadModel(relativePath);
    if (model)
    {
        return model->GetMesh();
    }
    return nullptr;
}

hs::Material* AssetDatabase::LoadMaterial(const std::string& relativePath)
{
    auto it = _loadedMaterials.find(relativePath);
    if (it != _loadedMaterials.end())
    {
        return it->second.get();
    }

    const AssetEntry* asset = FindAsset(relativePath);
    if (!asset || asset->type != EAssetType::Material)
    {
        return nullptr;
    }

    hs::Scoped<hs::Material> material = hs::MaterialSerializer::LoadFromFile(asset->absolutePath, _rootPath + "/");
    if (material)
    {
        material->SetSourceAssetPath(relativePath);
        if (material->GetDisplayName().empty())
        {
            material->SetDisplayName(std::filesystem::path(relativePath).stem().string());
        }

        hs::Material* rawPtr = material.get();
        _loadedMaterials[relativePath] = std::move(material);
        return rawPtr;
    }

    return nullptr;
}

bool AssetDatabase::SaveMaterial(const std::string& relativePath, hs::Material* material)
{
    if (!material)
    {
        return false;
    }

    std::filesystem::path absolutePath = std::filesystem::path(_rootPath) / relativePath;
    std::filesystem::create_directories(absolutePath.parent_path());

    material->SetSourceAssetPath(relativePath);
    if (material->GetDisplayName().empty())
    {
        material->SetDisplayName(absolutePath.stem().string());
    }

    if (!hs::MaterialSerializer::SaveToFile(absolutePath.string(), _rootPath + "/", *material))
    {
        return false;
    }

    Refresh();
    return true;
}

hs::Image* AssetDatabase::LoadTexture(const std::string& relativePath)
{
    auto cachedIt = _loadedTextures.find(relativePath);
    if (cachedIt != _loadedTextures.end())
    {
        return cachedIt->second.get();
    }

    const AssetEntry* asset = FindAsset(relativePath);
    if (!asset || asset->type != EAssetType::Texture)
    {
        return nullptr;
    }

    // Use ObjectManager to load
    // Note: ObjectManager expects path relative to Assets folder
    hs::Scoped<hs::Image> image = hs::ObjectManager::LoadImageFromFile(asset->absolutePath, true);
    if (image)
    {
        image->SetSourceAssetPath(relativePath);
        image->SetDisplayName(asset->name);
        hs::Image* rawPtr = image.get();
        _loadedTextures[relativePath] = std::move(image);
        return rawPtr;
    }
    return nullptr;
}

hs::Model* AssetDatabase::LoadModel(const std::string& relativePath)
{
    // Check cache first
    auto it = _loadedModels.find(relativePath);
    if (it != _loadedModels.end())
    {
        return it->second.get();
    }

    const AssetEntry* asset = FindAsset(relativePath);
    if (!asset || asset->type != EAssetType::Model)
    {
        return nullptr;
    }

    // Load using ObjectManager
    hs::Scoped<hs::Model> model;
    if (hs::ObjectManager::LoadModel(relativePath, model))
    {
        if (model->GetMesh())
        {
            model->GetMesh()->SetSourceAssetPath(relativePath);
            if (model->GetMesh()->GetDisplayName().empty())
            {
                model->GetMesh()->SetDisplayName(std::filesystem::path(relativePath).stem().string());
            }
        }
        hs::Model* rawPtr = model.get();
        _loadedModels[relativePath] = std::move(model);
        return rawPtr;
    }

    return nullptr;
}

bool AssetDatabase::IsMeshFile(const std::string& path) const
{
    EAssetType type = GetAssetTypeFromExtension(
        std::filesystem::path(path).extension().string());
    return type == EAssetType::Model;
}

bool AssetDatabase::IsTextureFile(const std::string& path) const
{
    EAssetType type = GetAssetTypeFromExtension(
        std::filesystem::path(path).extension().string());
    return type == EAssetType::Texture;
}

HS_NS_EDITOR_END
