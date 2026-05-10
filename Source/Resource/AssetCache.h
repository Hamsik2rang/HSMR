//
//  AssetCache.h
//  HSMR
//
//  Disk caching for decoded image and parsed mesh data so repeated runs of
//  the same asset (e.g. Sponza GLTF) skip stb_image / Assimp on cache hit.
//
#ifndef __HS_ASSET_CACHE_H__
#define __HS_ASSET_CACHE_H__

#include "Precompile.h"

#include <string>
#include <vector>

HS_NS_BEGIN

class Image;
class Mesh;
class Material;

class HS_RESOURCE_API AssetCache
{
public:
    static bool Initialize(const std::string& cacheRootDir);
    static void Finalize();
    static bool IsInitialized() { return s_initialized; }

    // Image cache (per source file). Cache key = source absolute path + mtime + size.
    // TryLoadImage allocates and returns a fully-formed Image on hit, or false on miss.
    static bool TryLoadImage(const std::string& sourceAbsPath, Scoped<Image>& outImage);
    static bool StoreImage(const std::string& sourceAbsPath, const Image& image);

    // Model cache (mesh + material arrays per GLTF/FBX). Includes material PBR
    // factors and texture asset paths so a cache hit fully bypasses Assimp.
    // Texture pixel data still flows through the Image cache when paths are
    // re-decoded (typically a fast cache hit).
    static bool TryLoadModel(const std::string& sourceAbsPath,
                             std::vector<Scoped<Mesh>>& outMeshes,
                             std::vector<Scoped<Material>>& outMaterials);
    static bool StoreModel(const std::string& sourceAbsPath,
                           const std::vector<Scoped<Mesh>>& meshes,
                           const std::vector<Scoped<Material>>& materials);

private:
    static uint64 computeSourceHash(const std::string& sourceAbsPath);
    static std::string getImagePath(uint64 hash);
    static std::string getModelPath(uint64 hash);

    static std::string s_cacheDir;
    static bool s_initialized;

    static constexpr uint32 IMAGE_MAGIC   = 0x58455354;  // 'TSEX' little-endian
    static constexpr uint32 IMAGE_VERSION = 1;
    static constexpr uint32 MODEL_MAGIC   = 0x4C444D48;  // 'HMDL' little-endian
    static constexpr uint32 MODEL_VERSION = 2;
};

HS_NS_END

#endif
