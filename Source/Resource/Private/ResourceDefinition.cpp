#include "Resource/ResourceDefinition.h"

#include "Core/Log.h"
#include "Core/Hash.h"

#include <cstring>

HS_NS_BEGIN

void MaterialParameterBlock::Initialize(const ShaderBufferBindingInfo& bufferInfo)
{
    _bufferName = bufferInfo.name;
    _binding = bufferInfo.binding;
    _set = bufferInfo.set;
    _data.resize(bufferInfo.totalSize, 0);
    _memberMap.clear();

    for (const auto& member : bufferInfo.members)
    {
        _memberMap[member.nameHash] = { member.offset, member.size };
    }

    _dirty = true;
}

bool MaterialParameterBlock::setRaw(const std::string& name, const void* data, uint32 dataSize)
{
    uint32 hash = StringHash(name);
    auto it = _memberMap.find(hash);
    if (it == _memberMap.end())
    {
        HS_LOG(warning, "[MaterialParameterBlock] Member '%s' not found in buffer '%s'",
               name.c_str(), _bufferName.c_str());
        return false;
    }

    uint32 offset = it->second.first;
    uint32 size = it->second.second;

    if (dataSize > size)
    {
        HS_LOG(warning, "[MaterialParameterBlock] Data size %u exceeds member '%s' size %u",
               dataSize, name.c_str(), size);
        return false;
    }

    if (offset + dataSize > static_cast<uint32>(_data.size()))
    {
        HS_LOG(error, "[MaterialParameterBlock] Write would overflow buffer");
        return false;
    }

    std::memcpy(_data.data() + offset, data, dataSize);
    _dirty = true;
    return true;
}

bool MaterialParameterBlock::SetFloat(const std::string& name, float value)
{
    return setRaw(name, &value, sizeof(float));
}

bool MaterialParameterBlock::SetVec2(const std::string& name, const glm::vec2& value)
{
    return setRaw(name, &value, sizeof(glm::vec2));
}

bool MaterialParameterBlock::SetVec3(const std::string& name, const glm::vec3& value)
{
    return setRaw(name, &value, sizeof(glm::vec3));
}

bool MaterialParameterBlock::SetVec4(const std::string& name, const glm::vec4& value)
{
    return setRaw(name, &value, sizeof(glm::vec4));
}

bool MaterialParameterBlock::SetMat3(const std::string& name, const glm::mat3& value)
{
    return setRaw(name, &value, sizeof(glm::mat3));
}

bool MaterialParameterBlock::SetMat4(const std::string& name, const glm::mat4& value)
{
    return setRaw(name, &value, sizeof(glm::mat4));
}

bool MaterialParameterBlock::SetInt(const std::string& name, int32 value)
{
    return setRaw(name, &value, sizeof(int32));
}

HS_NS_END
