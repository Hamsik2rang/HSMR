//
//  AssetCache.cpp
//
#include "Resource/AssetCache.h"

#include "Resource/Image.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"

#include "Core/Hash.h"
#include "Core/Log.h"
#include "Core/HAL/FileSystem.h"

#include <fstream>
#include <filesystem>
#include <cstdio>

HS_NS_BEGIN

std::string AssetCache::s_cacheDir;
bool AssetCache::s_initialized = false;

// =====================================================================
// Init / Finalize
// =====================================================================

bool AssetCache::Initialize(const std::string& cacheRootDir)
{
    s_cacheDir = cacheRootDir;
    if (!s_cacheDir.empty() && s_cacheDir.back() != HS_DIR_SEPERATOR)
    {
        s_cacheDir += HS_DIR_SEPERATOR;
    }

    const std::string imageDir = s_cacheDir + "image" + HS_DIR_SEPERATOR;
    const std::string modelDir = s_cacheDir + "model" + HS_DIR_SEPERATOR;

    if (!FileSystem::CreateDirectoryRecursive(imageDir) ||
        !FileSystem::CreateDirectoryRecursive(modelDir))
    {
        HS_LOG(warning, "[AssetCache] Failed to create cache directories under %s", s_cacheDir.c_str());
        return false;
    }

    s_initialized = true;
    HS_LOG(info, "[AssetCache] Initialized at: %s", s_cacheDir.c_str());
    return true;
}

void AssetCache::Finalize()
{
    std::string().swap(s_cacheDir);
    s_initialized = false;
}

uint64 AssetCache::computeSourceHash(const std::string& sourceAbsPath)
{
    uint64 hash = FNV1A64OffsetBasis;
    auto mix = [&](const void* data, size_t len)
    {
        hash = HashBytes64(data, len, hash);
    };

    mix(sourceAbsPath.data(), sourceAbsPath.size());

    std::error_code ec;
    namespace fs = std::filesystem;
    auto p = fs::path(sourceAbsPath);
    if (fs::exists(p, ec) && !ec)
    {
        auto sz = fs::file_size(p, ec);
        if (!ec) mix(&sz, sizeof(sz));
        auto t = fs::last_write_time(p, ec);
        if (!ec)
        {
            auto rep = t.time_since_epoch().count();
            mix(&rep, sizeof(rep));
        }
    }
    return hash;
}

std::string AssetCache::getImagePath(uint64 hash)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%016llx.hstex", static_cast<unsigned long long>(hash));
    return s_cacheDir + "image" + HS_DIR_SEPERATOR + buf;
}

std::string AssetCache::getModelPath(uint64 hash)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%016llx.hsmodel", static_cast<unsigned long long>(hash));
    return s_cacheDir + "model" + HS_DIR_SEPERATOR + buf;
}

// =====================================================================
// IO helpers
// =====================================================================

template <typename T>
static void writeVal(std::ofstream& f, const T& v)
{
    f.write(reinterpret_cast<const char*>(&v), sizeof(T));
}

template <typename T>
static bool readVal(std::ifstream& f, T& v)
{
    f.read(reinterpret_cast<char*>(&v), sizeof(T));
    return f.good();
}

static void writeStr(std::ofstream& f, const std::string& s)
{
    uint32 n = static_cast<uint32>(s.size());
    writeVal(f, n);
    if (n > 0) f.write(s.data(), n);
}

static bool readStr(std::ifstream& f, std::string& s)
{
    uint32 n;
    if (!readVal(f, n)) return false;
    s.resize(n);
    if (n > 0) f.read(s.data(), n);
    return f.good() || f.eof();
}

template <typename T>
static void writeVec(std::ofstream& f, const std::vector<T>& v)
{
    uint32 n = static_cast<uint32>(v.size());
    writeVal(f, n);
    if (n > 0) f.write(reinterpret_cast<const char*>(v.data()), n * sizeof(T));
}

template <typename T>
static bool readVec(std::ifstream& f, std::vector<T>& v)
{
    uint32 n;
    if (!readVal(f, n)) return false;
    v.resize(n);
    if (n > 0) f.read(reinterpret_cast<char*>(v.data()), n * sizeof(T));
    return f.good() || f.eof();
}

// =====================================================================
// Image cache
// =====================================================================

bool AssetCache::TryLoadImage(const std::string& sourceAbsPath, Scoped<Image>& outImage)
{
    if (!s_initialized) return false;

    const uint64 hash = computeSourceHash(sourceAbsPath);
    const std::string path = getImagePath(hash);

    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return false;

    uint32 magic = 0, version = 0;
    uint64 storedHash = 0;
    if (!readVal(f, magic) || magic != IMAGE_MAGIC) return false;
    if (!readVal(f, version) || version != IMAGE_VERSION) return false;
    if (!readVal(f, storedHash) || storedHash != hash) return false;

    uint16 width = 0, height = 0;
    uint8 channel = 0;
    if (!readVal(f, width) || !readVal(f, height) || !readVal(f, channel)) return false;

    std::vector<uint8> rawData;
    if (!readVec(f, rawData)) return false;

    outImage = MakeScoped<Image>(std::move(rawData), width, height, channel);
    outImage->SetDisplayName(FileSystem::GetFileName(sourceAbsPath));
    outImage->SetSourceAssetPath(sourceAbsPath);
    return true;
}

bool AssetCache::StoreImage(const std::string& sourceAbsPath, const Image& image)
{
    if (!s_initialized) return false;

    const uint64 hash = computeSourceHash(sourceAbsPath);
    const std::string path = getImagePath(hash);

    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f.is_open())
    {
        HS_LOG(warning, "[AssetCache] Failed to open for image cache write: %s", path.c_str());
        return false;
    }

    writeVal(f, IMAGE_MAGIC);
    writeVal(f, IMAGE_VERSION);
    writeVal(f, hash);
    writeVal(f, image.GetWidth());
    writeVal(f, image.GetHeight());
    writeVal(f, image.GetChannel());
    writeVec(f, image.GetRawDataVector());
    return true;
}

// =====================================================================
// Model (mesh-array) cache
// =====================================================================

// Serialize a single Material's PBR factors and texture asset paths. Texture
// pixel data is NOT inlined; on cache hit the paths flow through the Image
// cache (which is itself a fast fopen/read on warm runs).
static void writeMaterial(std::ofstream& f, const Material& m)
{
    writeStr(f, m.GetDisplayName());
    writeStr(f, m.GetShaderNameHint());

    auto writeVec4 = [&](const glm::vec4& v) {
        writeVal(f, v.x); writeVal(f, v.y); writeVal(f, v.z); writeVal(f, v.w);
    };
    writeVec4(m.GetDiffuseColor());
    writeVec4(m.GetSpecularColor());
    writeVec4(m.GetEmissionColor());
    writeVec4(m.GetAmbientColor());
    writeVal(f, m.GetShininess());
    writeVal(f, m.GetOpacity());
    writeVal(f, m.GetRoughness());
    writeVal(f, m.GetMetallic());
    writeVal(f, static_cast<uint8>(m.IsTwoSided() ? 1 : 0));

    // Texture asset path entries (type, path).
    constexpr uint8 typeCount = static_cast<uint8>(EMaterialTextureType::MaxTextureTypes);
    uint32 entryCount = 0;
    for (uint8 t = 0; t < typeCount; ++t)
    {
        auto type = static_cast<EMaterialTextureType>(t);
        if (m.HasTextureAssetPath(type)) ++entryCount;
    }
    writeVal(f, entryCount);
    for (uint8 t = 0; t < typeCount; ++t)
    {
        auto type = static_cast<EMaterialTextureType>(t);
        if (!m.HasTextureAssetPath(type)) continue;
        writeVal(f, t);
        writeStr(f, m.GetTextureAssetPath(type));
    }
}

static bool readMaterial(std::ifstream& f, Material& m)
{
    std::string name;
    if (!readStr(f, name)) return false;
    if (!name.empty()) m.SetDisplayName(name);

    std::string hint;
    if (!readStr(f, hint)) return false;
    m.SetShaderNameHint(hint);

    glm::vec4 v;
    auto readVec4 = [&](glm::vec4& out) {
        if (!readVal(f, out.x)) return false;
        if (!readVal(f, out.y)) return false;
        if (!readVal(f, out.z)) return false;
        if (!readVal(f, out.w)) return false;
        return true;
    };

    if (!readVec4(v)) return false; m.SetDiffuseColor(v);
    if (!readVec4(v)) return false; m.SetSpecularColor(v);
    if (!readVec4(v)) return false; m.SetEmissionColor(v);
    if (!readVec4(v)) return false; m.SetAmbientColor(v);

    float fv;
    if (!readVal(f, fv)) return false; m.SetShininess(fv);
    if (!readVal(f, fv)) return false; m.SetOpacity(fv);
    if (!readVal(f, fv)) return false; m.SetRoughness(fv);
    if (!readVal(f, fv)) return false; m.SetMetallic(fv);

    uint8 twoSided = 0;
    if (!readVal(f, twoSided)) return false;
    m.SetTwoSided(twoSided != 0);

    uint32 entryCount = 0;
    if (!readVal(f, entryCount)) return false;
    for (uint32 i = 0; i < entryCount; ++i)
    {
        uint8 typeIdx;
        std::string p;
        if (!readVal(f, typeIdx)) return false;
        if (!readStr(f, p))       return false;
        m.SetTextureAssetPath(static_cast<EMaterialTextureType>(typeIdx), p);
    }
    return true;
}

bool AssetCache::TryLoadModel(const std::string& sourceAbsPath,
                              std::vector<Scoped<Mesh>>& outMeshes,
                              std::vector<Scoped<Material>>& outMaterials)
{
    if (!s_initialized) return false;

    const uint64 hash = computeSourceHash(sourceAbsPath);
    const std::string path = getModelPath(hash);

    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return false;

    uint32 magic = 0, version = 0;
    uint64 storedHash = 0;
    if (!readVal(f, magic) || magic != MODEL_MAGIC) return false;
    if (!readVal(f, version) || version != MODEL_VERSION) return false;
    if (!readVal(f, storedHash) || storedHash != hash) return false;

    // Materials
    uint32 matCount = 0;
    if (!readVal(f, matCount)) return false;
    outMaterials.clear();
    outMaterials.reserve(matCount);
    for (uint32 i = 0; i < matCount; ++i)
    {
        Scoped<Material> mat = MakeScoped<Material>();
        if (!readMaterial(f, *mat)) return false;
        outMaterials.push_back(std::move(mat));
    }

    // Meshes
    uint32 meshCount = 0;
    if (!readVal(f, meshCount)) return false;
    outMeshes.clear();
    outMeshes.reserve(meshCount);

    for (uint32 i = 0; i < meshCount; ++i)
    {
        Scoped<Mesh> mesh = MakeScoped<Mesh>();

        std::string displayName;
        if (!readStr(f, displayName)) return false;
        if (!displayName.empty()) mesh->SetDisplayName(displayName);

        int32 materialIndex = -1;
        if (!readVal(f, materialIndex)) return false;
        mesh->SetMaterialIndex(materialIndex);

        std::vector<float> position, normal, tangent, bitangent, texcoord0, color;
        std::vector<uint32> indices;

        if (!readVec(f, position))  return false;
        if (!readVec(f, normal))    return false;
        if (!readVec(f, tangent))   return false;
        if (!readVec(f, bitangent)) return false;
        if (!readVec(f, texcoord0)) return false;
        if (!readVec(f, color))     return false;
        if (!readVec(f, indices))   return false;

        mesh->SetPosition(std::move(position));
        if (!normal.empty())    mesh->SetNormal(std::move(normal));
        if (!tangent.empty())   mesh->SetTangent(std::move(tangent));
        if (!bitangent.empty()) mesh->SetBitangent(std::move(bitangent));
        if (!texcoord0.empty()) mesh->SetTexCoord(std::move(texcoord0), 0);
        if (!color.empty())     mesh->SetColor(std::move(color));
        if (!indices.empty())   mesh->SetIndices(std::move(indices));

        outMeshes.push_back(std::move(mesh));
    }

    HS_LOG(info, "[AssetCache] Model cache hit (%u meshes / %u materials): %s",
           meshCount, matCount, sourceAbsPath.c_str());
    return true;
}

bool AssetCache::StoreModel(const std::string& sourceAbsPath,
                            const std::vector<Scoped<Mesh>>& meshes,
                            const std::vector<Scoped<Material>>& materials)
{
    if (!s_initialized) return false;

    const uint64 hash = computeSourceHash(sourceAbsPath);
    const std::string path = getModelPath(hash);

    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f.is_open())
    {
        HS_LOG(warning, "[AssetCache] Failed to open for model cache write: %s", path.c_str());
        return false;
    }

    writeVal(f, MODEL_MAGIC);
    writeVal(f, MODEL_VERSION);
    writeVal(f, hash);

    uint32 matCount = static_cast<uint32>(materials.size());
    writeVal(f, matCount);
    for (const auto& matPtr : materials)
    {
        if (matPtr) writeMaterial(f, *matPtr);
        else        { Material empty; writeMaterial(f, empty); }
    }

    uint32 meshCount = static_cast<uint32>(meshes.size());
    writeVal(f, meshCount);
    for (const auto& meshPtr : meshes)
    {
        Mesh* mesh = meshPtr.get();
        writeStr(f, mesh ? mesh->GetDisplayName() : std::string{});
        writeVal(f, mesh ? mesh->GetMaterialIndex() : -1);
        writeVec(f, mesh ? mesh->GetPosition()  : std::vector<float>{});
        writeVec(f, mesh ? mesh->GetNormal()    : std::vector<float>{});
        writeVec(f, mesh ? mesh->GetTangent()   : std::vector<float>{});
        writeVec(f, mesh ? mesh->GetBitangent() : std::vector<float>{});
        writeVec(f, mesh ? mesh->GetTexCoord(0) : std::vector<float>{});
        writeVec(f, mesh ? mesh->GetColor()     : std::vector<float>{});
        writeVec(f, mesh ? mesh->GetIndices()   : std::vector<uint32>{});
    }

    HS_LOG(info, "[AssetCache] Stored model cache (%u meshes / %u materials): %s",
           meshCount, matCount, path.c_str());
    return true;
}

HS_NS_END
