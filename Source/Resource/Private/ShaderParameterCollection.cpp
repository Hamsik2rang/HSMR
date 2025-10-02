#include "Resource/ShaderParameterCollection.h"
#include "Core/Log.h"
#include <cstring>

HS_NS_BEGIN

ShaderParameterCollection::ShaderParameterCollection()
{
}

ShaderParameterCollection::~ShaderParameterCollection()
{
}

void ShaderParameterCollection::SetParameter(const std::string& name, float value)
{
    ValidateParameter(name, ShaderParameterType::Float);
    _parameters[name] = value;
    MarkHashDirty();
}

void ShaderParameterCollection::SetParameter(const std::string& name, const glm::vec2& value)
{
    ValidateParameter(name, ShaderParameterType::Vec2);
    _parameters[name] = value;
    MarkHashDirty();
}

void ShaderParameterCollection::SetParameter(const std::string& name, const glm::vec3& value)
{
    ValidateParameter(name, ShaderParameterType::Vec3);
    _parameters[name] = value;
    MarkHashDirty();
}

void ShaderParameterCollection::SetParameter(const std::string& name, const glm::vec4& value)
{
    ValidateParameter(name, ShaderParameterType::Vec4);
    _parameters[name] = value;
    MarkHashDirty();
}

void ShaderParameterCollection::SetParameter(const std::string& name, int32 value)
{
    ValidateParameter(name, ShaderParameterType::Int);
    _parameters[name] = value;
    MarkHashDirty();
}

void ShaderParameterCollection::SetParameter(const std::string& name, const glm::ivec2& value)
{
    ValidateParameter(name, ShaderParameterType::IVec2);
    _parameters[name] = value;
    MarkHashDirty();
}

void ShaderParameterCollection::SetParameter(const std::string& name, const glm::ivec3& value)
{
    ValidateParameter(name, ShaderParameterType::IVec3);
    _parameters[name] = value;
    MarkHashDirty();
}

void ShaderParameterCollection::SetParameter(const std::string& name, const glm::ivec4& value)
{
    ValidateParameter(name, ShaderParameterType::IVec4);
    _parameters[name] = value;
    MarkHashDirty();
}

void ShaderParameterCollection::SetParameter(const std::string& name, const glm::mat3& value)
{
    ValidateParameter(name, ShaderParameterType::Mat3);
    _parameters[name] = value;
    MarkHashDirty();
}

void ShaderParameterCollection::SetParameter(const std::string& name, const glm::mat4& value)
{
    ValidateParameter(name, ShaderParameterType::Mat4);
    _parameters[name] = value;
    MarkHashDirty();
}

void ShaderParameterCollection::SetParameter(const std::string& name, bool value)
{
    ValidateParameter(name, ShaderParameterType::Bool);
    _parameters[name] = value;
    MarkHashDirty();
}

void ShaderParameterCollection::SetParameter(const std::string& name, uint64 textureHandle)
{
    // For texture parameters, we accept both Texture2D and TextureCube types
    auto infoIt = _parameterInfo.find(name);
    if (infoIt != _parameterInfo.end())
    {
        if (infoIt->second.type != ShaderParameterType::Texture2D && 
            infoIt->second.type != ShaderParameterType::TextureCube &&
            infoIt->second.type != ShaderParameterType::Buffer)
        {
            HS_LOG(warning, "Parameter '%s' type mismatch for texture/buffer handle", name.c_str());
        }
    }
    
    _parameters[name] = textureHandle;
    MarkHashDirty();
}

bool ShaderParameterCollection::HasParameter(const std::string& name) const
{
    return _parameters.find(name) != _parameters.end();
}

ShaderParameterType ShaderParameterCollection::GetParameterType(const std::string& name) const
{
    auto it = _parameterInfo.find(name);
    if (it != _parameterInfo.end())
    {
        return it->second.type;
    }
    return ShaderParameterType::Float; // Default
}

const std::unordered_map<std::string, ShaderParameterInfo>& ShaderParameterCollection::GetParameterInfo() const
{
    return _parameterInfo;
}

const std::unordered_map<std::string, MaterialParameterValue>& ShaderParameterCollection::GetParameters() const
{
    return _parameters;
}

uint64 ShaderParameterCollection::GetParametersHash() const
{
    if (!_hashDirty)
    {
        return _cachedHash;
    }

    uint64 hash = 14695981039346656037ULL; // FNV-1a offset basis
    
    // Hash parameter values
    for (const auto& pair : _parameters)
    {
        // Hash parameter name
        for (char c : pair.first)
        {
            hash ^= static_cast<uint64>(c);
            hash *= 1099511628211ULL; // FNV-1a prime
        }

        // Hash parameter value based on type
        std::visit([&hash](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            
            if constexpr (std::is_same_v<T, float>)
            {
                uint32 intVal = *reinterpret_cast<const uint32*>(&value);
                hash ^= static_cast<uint64>(intVal);
            }
            else if constexpr (std::is_same_v<T, int32> || std::is_same_v<T, bool>)
            {
                hash ^= static_cast<uint64>(value);
            }
            else if constexpr (std::is_same_v<T, uint64>)
            {
                hash ^= value;
            }
            else if constexpr (std::is_same_v<T, glm::vec2>)
            {
                hash ^= *reinterpret_cast<const uint64*>(&value);
            }
            else if constexpr (std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::ivec3>)
            {
                const uint32* ptr = reinterpret_cast<const uint32*>(&value);
                hash ^= static_cast<uint64>(ptr[0]) | (static_cast<uint64>(ptr[1]) << 32);
                hash ^= static_cast<uint64>(ptr[2]);
            }
            else if constexpr (std::is_same_v<T, glm::vec4> || std::is_same_v<T, glm::ivec4>)
            {
                const uint64* ptr = reinterpret_cast<const uint64*>(&value);
                hash ^= ptr[0] ^ ptr[1];
            }
            else if constexpr (std::is_same_v<T, glm::mat3>)
            {
                const uint32* ptr = reinterpret_cast<const uint32*>(&value);
                for (int i = 0; i < 9; ++i)
                {
                    hash ^= static_cast<uint64>(ptr[i]);
                    hash *= 1099511628211ULL;
                }
            }
            else if constexpr (std::is_same_v<T, glm::mat4>)
            {
                const uint64* ptr = reinterpret_cast<const uint64*>(&value);
                for (int i = 0; i < 8; ++i)
                {
                    hash ^= ptr[i];
                    hash *= 1099511628211ULL;
                }
            }
            
            hash *= 1099511628211ULL;
        }, pair.second);
    }

    _cachedHash = hash;
    _hashDirty = false;
    return _cachedHash;
}

void ShaderParameterCollection::RegisterParameter(const std::string& name, ShaderParameterType type, 
                                                 uint32 offset, uint32 size, uint32 arrayCount)
{
    ShaderParameterInfo info;
    info.name = name;
    info.type = type;
    info.offset = offset;
    info.size = size > 0 ? size : GetTypeSize(type);
    info.arrayCount = arrayCount;
    
    _parameterInfo[name] = info;
}

void ShaderParameterCollection::Clear()
{
    _parameters.clear();
    _parameterInfo.clear();
    MarkHashDirty();
}

void ShaderParameterCollection::CopyFrom(const ShaderParameterCollection& other)
{
    _parameters = other._parameters;
    _parameterInfo = other._parameterInfo;
    MarkHashDirty();
}

std::vector<uint8> ShaderParameterCollection::PackParametersForGPU() const
{
    if (_parameterInfo.empty())
    {
        return {};
    }

    uint32 totalSize = GetPackedSize();
    std::vector<uint8> packedData(totalSize, 0);

    for (const auto& paramInfo : _parameterInfo)
    {
        const std::string& name = paramInfo.first;
        const ShaderParameterInfo& info = paramInfo.second;
        
        auto paramIt = _parameters.find(name);
        if (paramIt == _parameters.end())
        {
            continue; // Skip unset parameters
        }

        uint8* dest = packedData.data() + info.offset;
        
        std::visit([dest, &info](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            
            if constexpr (std::is_same_v<T, uint64>)
            {
                // Skip texture handles in constant buffer packing
                return;
            }
            else
            {
                std::memcpy(dest, &value, sizeof(T));
            }
        }, paramIt->second);
    }

    return packedData;
}

uint32 ShaderParameterCollection::GetPackedSize() const
{
    uint32 maxOffset = 0;
    uint32 maxSize = 0;
    
    for (const auto& pair : _parameterInfo)
    {
        const ShaderParameterInfo& info = pair.second;
        
        // Skip texture handles
        if (info.type == ShaderParameterType::Texture2D ||
            info.type == ShaderParameterType::TextureCube ||
            info.type == ShaderParameterType::Buffer)
        {
            continue;
        }
        
        uint32 endOffset = info.offset + info.size;
        if (endOffset > maxOffset + maxSize)
        {
            maxOffset = info.offset;
            maxSize = info.size;
        }
    }
    
    return maxOffset + maxSize;
}

uint32 ShaderParameterCollection::GetTypeSize(ShaderParameterType type) const
{
    switch (type)
    {
    case ShaderParameterType::Float:
    case ShaderParameterType::Int:
    case ShaderParameterType::Bool:
        return sizeof(float); // Bool is treated as 4 bytes in GPU
        
    case ShaderParameterType::Vec2:
    case ShaderParameterType::IVec2:
        return sizeof(float) * 2;
        
    case ShaderParameterType::Vec3:
    case ShaderParameterType::IVec3:
        return sizeof(float) * 3;
        
    case ShaderParameterType::Vec4:
    case ShaderParameterType::IVec4:
        return sizeof(float) * 4;
        
    case ShaderParameterType::Mat3:
        return sizeof(float) * 9;
        
    case ShaderParameterType::Mat4:
        return sizeof(float) * 16;
        
    case ShaderParameterType::Texture2D:
    case ShaderParameterType::TextureCube:
    case ShaderParameterType::Buffer:
        return 0; // Textures/buffers don't take space in constant buffers
        
    default:
        return sizeof(float);
    }
}

void ShaderParameterCollection::ValidateParameter(const std::string& name, ShaderParameterType expectedType) const
{
    auto it = _parameterInfo.find(name);
    if (it != _parameterInfo.end() && it->second.type != expectedType)
    {
        HS_LOG(warning, "Parameter '%s' type mismatch. Expected: %u, Got: %u", 
               name.c_str(), static_cast<uint32>(it->second.type), static_cast<uint32>(expectedType));
    }
}

HS_NS_END