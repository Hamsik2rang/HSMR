//
//  ResourceDefinition.h
//  HSMR
//
//  Created by Yongsik Im on 10/26/25.
//
#ifndef __HS_RESOURCE_DEFINITION_H__
#define __HS_RESOURCE_DEFINITION_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"
#include "ShaderSystem/ShaderSystemDefinition.h"

#include "Core/Hash.h"
#include "Core/Math/Common.h"

#include <unordered_map>

HS_NS_BEGIN

// Reflection-based byte buffer for material parameters
class HS_RESOURCE_API MaterialParameterBlock
{
public:
    MaterialParameterBlock() = default;

    void Initialize(const ShaderBufferBindingInfo& bufferInfo);

    bool SetFloat(const std::string& name, float value);
    bool SetVec2(const std::string& name, const glm::vec2& value);
    bool SetVec3(const std::string& name, const glm::vec3& value);
    bool SetVec4(const std::string& name, const glm::vec4& value);
    bool SetMat3(const std::string& name, const glm::mat3& value);
    bool SetMat4(const std::string& name, const glm::mat4& value);
    bool SetInt(const std::string& name, int32 value);

    const void* GetData() const { return _data.empty() ? nullptr : _data.data(); }
    uint32 GetSize() const { return static_cast<uint32>(_data.size()); }
    bool IsDirty() const { return _dirty; }
    void ClearDirty() { _dirty = false; }
    bool IsInitialized() const { return !_data.empty(); }

    const std::string& GetBufferName() const { return _bufferName; }
    uint32 GetBinding() const { return _binding; }
    uint32 GetSet() const { return _set; }

private:
    bool setRaw(const std::string& name, const void* data, uint32 dataSize);

    std::vector<uint8> _data;
    std::unordered_map<uint32, std::pair<uint32, uint32>> _memberMap; // nameHash -> (offset, size)
    std::string _bufferName;
    uint32 _binding = 0;
    uint32 _set = 0;
    bool _dirty = false;
};


HS_NS_END

#endif
