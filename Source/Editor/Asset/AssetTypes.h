//
//  AssetTypes.h
//  Editor
//
//  Asset type definitions for the editor
//

#pragma once

#include "Precompile.h"
#include "Editor/GUI/EditorIcons.h"

#include <string>

HS_NS_EDITOR_BEGIN

/**
 * @brief Asset type enumeration
 */
enum class EAssetType : uint8
{
    Unknown = 0,
    Texture,        // .png, .jpg, .jpeg, .tga, .bmp, .hdr
    Model,          // .gltf, .glb, .fbx, .obj
    Material,       // .mat (custom format)
    Shader,         // .slang, .hlsl, .glsl
    Scene,          // .scene (custom format)
    Audio,          // .wav, .mp3, .ogg
    Font,           // .ttf, .otf
    Prefab,         // .prefab (custom format)
    Count
};

/**
 * @brief Get asset type from file extension
 */
inline EAssetType GetAssetTypeFromExtension(const std::string& extension)
{
    std::string ext = extension;
    // Convert to lowercase
    for (auto& c : ext) c = static_cast<char>(std::tolower(c));

    // Remove leading dot if present
    if (!ext.empty() && ext[0] == '.')
        ext = ext.substr(1);

    // Textures
    if (ext == "png" || ext == "jpg" || ext == "jpeg" ||
        ext == "tga" || ext == "bmp" || ext == "hdr")
        return EAssetType::Texture;

    // Models
    if (ext == "gltf" || ext == "glb" || ext == "fbx" || ext == "obj")
        return EAssetType::Model;

    // Materials
    if (ext == "mat")
        return EAssetType::Material;

    // Shaders
    if (ext == "slang" || ext == "hlsl" || ext == "glsl" ||
        ext == "vert" || ext == "frag" || ext == "comp")
        return EAssetType::Shader;

    // Scenes
    if (ext == "scene" || ext == "json")
        return EAssetType::Scene;

    // Audio
    if (ext == "wav" || ext == "mp3" || ext == "ogg")
        return EAssetType::Audio;

    // Fonts
    if (ext == "ttf" || ext == "otf")
        return EAssetType::Font;

    // Prefabs
    if (ext == "prefab")
        return EAssetType::Prefab;

    return EAssetType::Unknown;
}

/**
 * @brief Get display name for asset type
 */
inline const char* GetAssetTypeName(EAssetType type)
{
    switch (type)
    {
        case EAssetType::Texture:  return "Texture";
        case EAssetType::Model:    return "Model";
        case EAssetType::Material: return "Material";
        case EAssetType::Shader:   return "Shader";
        case EAssetType::Scene:    return "Scene";
        case EAssetType::Audio:    return "Audio";
        case EAssetType::Font:     return "Font";
        case EAssetType::Prefab:   return "Prefab";
        default:                   return "Unknown";
    }
}

/**
 * @brief Get Material Symbols icon for asset type.
 */
inline const char* GetAssetTypeIcon(EAssetType type)
{
    switch (type)
    {
        case EAssetType::Texture:  return EditorIcons::Image;
        case EAssetType::Model:    return EditorIcons::ViewInAr;
        case EAssetType::Material: return EditorIcons::Palette;
        case EAssetType::Shader:   return EditorIcons::Code;
        case EAssetType::Scene:    return EditorIcons::Description;
        case EAssetType::Audio:    return EditorIcons::Settings;
        case EAssetType::Font:     return EditorIcons::Article;
        case EAssetType::Prefab:   return EditorIcons::DeployedCode;
        default:                   return EditorIcons::Draft;
    }
}

HS_NS_EDITOR_END
