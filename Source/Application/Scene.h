//
//  Scene.h
//  HSMR
//
//  Created for lightweight prototyping framework
//
#ifndef __HS_APPLICATION_SCENE_H__
#define __HS_APPLICATION_SCENE_H__

#include "Precompile.h"
#include "SceneObject.h"
#include "Camera.h"
#include <string>
#include <vector>
#include <unordered_map>

// Forward declarations
struct RHIShader;
struct RHITexture;

HS_NS_BEGIN

// Model data structure (loaded from file)
struct HS_APPLICATION_API ModelData
{
    std::string name;
    std::string path;

    // Model settings
    bool generateNormals = false;
    bool generateTangents = false;
    float scale = 1.0f;

    // Computed bounds
    AABB bounds;

    // Model resource (can be extended to actual mesh data)
    void* modelResource = nullptr;
};

// Shader data structure
struct HS_APPLICATION_API ShaderData
{
    std::string name;
    std::string path;
    std::vector<std::string> stages;

    // Shader resources
    RHIShader* vertexShader = nullptr;
    RHIShader* fragmentShader = nullptr;
    RHIShader* computeShader = nullptr;
};

// Texture data structure
struct HS_APPLICATION_API TextureData
{
    std::string name;
    std::string path;

    // Texture resource
    RHITexture* texture = nullptr;
};

// Camera configuration from JSON
struct HS_APPLICATION_API CameraConfig
{
    glm::vec3 position = glm::vec3(0.0f, 2.0f, -5.0f);
    glm::vec3 target = glm::vec3(0.0f);
    float fov = 60.0f;
    float nearZ = 0.1f;
    float farZ = 1000.0f;
};

// Scene class - manages scene data loaded from JSON
class HS_APPLICATION_API Scene
{
public:
    Scene() = default;
    ~Scene();

    // JSON loading
    bool LoadFromJSON(const std::string& path);
    void Clear();

    // Scene name
    const std::string& GetName() const { return _name; }

    // Camera configuration
    const CameraConfig& GetCameraConfig() const { return _cameraConfig; }
    void ApplyCameraConfig(Camera* camera) const;

    // Resource access
    const std::vector<ModelData>& GetModels() const { return _models; }
    const std::vector<ShaderData>& GetShaders() const { return _shaders; }
    const std::vector<TextureData>& GetTextures() const { return _textures; }

    // Scene objects
    const std::vector<SceneObject>& GetObjects() const { return _objects; }
    std::vector<SceneObject>& GetObjects() { return _objects; }

    size_t GetObjectCount() const { return _objects.size(); }
    SceneObject* GetObject(size_t index);
    SceneObject* FindObjectByName(const std::string& name);

    // Selection
    SceneObject* GetSelectedObject() const { return _selectedObject; }
    void SetSelectedObject(SceneObject* obj) { _selectedObject = obj; }

    // Model lookup
    int32 FindModelIndex(const std::string& name) const;
    int32 FindShaderIndex(const std::string& name) const;
    int32 FindTextureIndex(const std::string& name) const;

    // Resource loading callbacks (set by Application)
    using ModelLoadCallback = std::function<void*(const ModelData&)>;
    using ShaderLoadCallback = std::function<void(ShaderData&)>;
    using TextureLoadCallback = std::function<RHITexture*(const TextureData&)>;

    void SetModelLoadCallback(ModelLoadCallback cb) { _modelLoadCallback = cb; }
    void SetShaderLoadCallback(ShaderLoadCallback cb) { _shaderLoadCallback = cb; }
    void SetTextureLoadCallback(TextureLoadCallback cb) { _textureLoadCallback = cb; }

private:
    bool parseCamera(const void* jsonCamera);
    bool parseShaders(const void* jsonShaders);
    bool parseTextures(const void* jsonTextures);
    bool parseModels(const void* jsonModels);
    bool parseObjects(const void* jsonObjects);

    std::string _name;
    std::string _basePath; // Directory containing the JSON file

    CameraConfig _cameraConfig;

    std::vector<ModelData> _models;
    std::vector<ShaderData> _shaders;
    std::vector<TextureData> _textures;
    std::vector<SceneObject> _objects;

    // Name to index mappings for fast lookup
    std::unordered_map<std::string, int32> _modelNameMap;
    std::unordered_map<std::string, int32> _shaderNameMap;
    std::unordered_map<std::string, int32> _textureNameMap;

    SceneObject* _selectedObject = nullptr;

    // Resource loading callbacks
    ModelLoadCallback _modelLoadCallback;
    ShaderLoadCallback _shaderLoadCallback;
    TextureLoadCallback _textureLoadCallback;
};

HS_NS_END

#endif // __HS_APPLICATION_SCENE_H__
