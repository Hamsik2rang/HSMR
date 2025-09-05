#ifndef __HS_SHADER_PARAMETER_COLLECTION_H__
#define __HS_SHADER_PARAMETER_COLLECTION_H__

#include "Precompile.h"
#include "Core/Math/Math.h"
#include "Core/Hash.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <variant>

HS_NS_BEGIN

enum class ShaderParameterType : uint8
{
    Float = 0,
    Vec2,
    Vec3,
    Vec4,
    Int,
    IVec2,
    IVec3,
    IVec4,
    Mat3,
    Mat4,
    Bool,
    Texture2D,
    TextureCube,
    Buffer
};

struct ShaderParameterInfo
{
    ShaderParameterType type;
    uint32 offset;
    uint32 size;
    uint32 arrayCount;
    std::string name;
};

using MaterialParameterValue = std::variant<
    float,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    int32,
    glm::ivec2,
    glm::ivec3,
    glm::ivec4,
    glm::mat3,
    glm::mat4,
    bool,
    uint64  // For texture/buffer handles
>;

class HS_OBJECT_API ShaderParameterCollection
{
public:
    ShaderParameterCollection();
    ~ShaderParameterCollection();

    // Parameter setters
    void SetParameter(const std::string& name, float value);
    void SetParameter(const std::string& name, const glm::vec2& value);
    void SetParameter(const std::string& name, const glm::vec3& value);
    void SetParameter(const std::string& name, const glm::vec4& value);
    void SetParameter(const std::string& name, int32 value);
    void SetParameter(const std::string& name, const glm::ivec2& value);
    void SetParameter(const std::string& name, const glm::ivec3& value);
    void SetParameter(const std::string& name, const glm::ivec4& value);
    void SetParameter(const std::string& name, const glm::mat3& value);
    void SetParameter(const std::string& name, const glm::mat4& value);
    void SetParameter(const std::string& name, bool value);
    void SetParameter(const std::string& name, uint64 textureHandle); // For textures

    // Parameter getters
    template<typename T>
    bool GetParameter(const std::string& name, T& outValue) const;

    bool HasParameter(const std::string& name) const;
    ShaderParameterType GetParameterType(const std::string& name) const;

    // Get parameter info for reflection
    const std::unordered_map<std::string, ShaderParameterInfo>& GetParameterInfo() const;
    
    // Get raw parameter data for constant buffer upload
    const std::unordered_map<std::string, MaterialParameterValue>& GetParameters() const;
    
    // Generate hash for change detection
    uint64 GetParametersHash() const;

    // Register parameter info (used by shaders to define their interface)
    void RegisterParameter(const std::string& name, ShaderParameterType type, 
                          uint32 offset = 0, uint32 size = 0, uint32 arrayCount = 1);

    // Clear all parameters
    void Clear();

    // Copy parameters from another collection
    void CopyFrom(const ShaderParameterCollection& other);

    // Pack parameters into a continuous buffer for GPU upload
    std::vector<uint8> PackParametersForGPU() const;
    uint32 GetPackedSize() const;

private:
    std::unordered_map<std::string, MaterialParameterValue> _parameters;
    std::unordered_map<std::string, ShaderParameterInfo> _parameterInfo;
    mutable uint64 _cachedHash = 0;
    mutable bool _hashDirty = true;

    void MarkHashDirty() { _hashDirty = true; }
    uint32 GetTypeSize(ShaderParameterType type) const;
    void ValidateParameter(const std::string& name, ShaderParameterType expectedType) const;
};

// Template implementation for getters
template<typename T>
bool ShaderParameterCollection::GetParameter(const std::string& name, T& outValue) const
{
    auto it = _parameters.find(name);
    if (it == _parameters.end())
    {
        return false;
    }

    if (const T* value = std::get_if<T>(&it->second))
    {
        outValue = *value;
        return true;
    }
    
    return false;
}

HS_NS_END

#endif