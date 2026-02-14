//
//  ShaderSystemDefinition.h
//  HSMR
//
//  Reflection data types for the ShaderSystem module.
//
#ifndef __HS_SHADER_SYSTEM_DEFINITION_H__
#define __HS_SHADER_SYSTEM_DEFINITION_H__

#include "Precompile.h"
#include "Core/Hash.h"
#include "RHI/RHIDefinition.h"

#include <string>
#include <vector>
#include <unordered_map>

HS_NS_BEGIN

// UBO member layout
struct HS_SHADER_SYSTEM_API ShaderBufferMember
{
    std::string name;
    uint32 offset;      // Byte offset within buffer
    uint32 size;        // Byte size
    uint32 nameHash;    // FNV-1a hash for fast lookup
};

// Buffer binding info (UBO/SSBO)
struct HS_SHADER_SYSTEM_API ShaderBufferBindingInfo
{
    std::string name;
    uint32 nameHash = 0;
    uint32 set = 0;         // Descriptor set index
    uint32 binding = 0;     // Binding slot
    uint32 totalSize = 0;   // Total byte size
    EShaderStage stages = EShaderStage::None;   // Bitmask: which stages use this
    EResourceType resourceType = EResourceType::UniformBuffer;
    std::vector<ShaderBufferMember> members;
};

// Texture binding info
struct HS_SHADER_SYSTEM_API ShaderTextureBindingInfo
{
    std::string name;
    uint32 nameHash = 0;
    uint32 set = 0;
    uint32 binding = 0;
    EShaderStage stages = EShaderStage::None;
    uint32 dimension = 2;   // 1D, 2D, 3D, Cube
};

// Sampler binding info
struct HS_SHADER_SYSTEM_API ShaderSamplerBindingInfo
{
    std::string name;
    uint32 nameHash = 0;
    uint32 set = 0;
    uint32 binding = 0;
    EShaderStage stages = EShaderStage::None;
};

// Vertex input attribute
struct HS_SHADER_SYSTEM_API ShaderVertexAttribute
{
    std::string semantic;   // "POSITION0", "NORMAL0", etc.
    uint32 location = 0;
    EVertexFormat format = EVertexFormat::Invalid;
    uint32 offset = 0;
    uint32 size = 0;
};

// Vertex input layout
struct HS_SHADER_SYSTEM_API ShaderVertexInputLayout
{
    std::vector<ShaderVertexAttribute> attributes;
    uint32 stride = 0;
};

// Unified reflection data
struct HS_SHADER_SYSTEM_API ShaderReflectionDataEx
{
    std::vector<ShaderBufferBindingInfo> bufferBindings;
    std::vector<ShaderTextureBindingInfo> textureBindings;
    std::vector<ShaderSamplerBindingInfo> samplerBindings;
    ShaderVertexInputLayout vertexInput;

    std::string vertexEntryPoint;
    std::string fragmentEntryPoint;
    std::string computeEntryPoint;

    std::unordered_map<uint32, uint32> bufferLookup;  // nameHash -> index
    bool isValid = false;

    const ShaderBufferBindingInfo* FindBuffer(const std::string& name) const
    {
        uint32 hash = StringHash(name);
        auto it = bufferLookup.find(hash);
        if (it != bufferLookup.end() && it->second < bufferBindings.size())
        {
            return &bufferBindings[it->second];
        }
        return nullptr;
    }

    bool FindMemberOffset(const std::string& bufferName, const std::string& memberName,
                          uint32& outOffset, uint32& outSize) const
    {
        const ShaderBufferBindingInfo* buf = FindBuffer(bufferName);
        if (!buf) return false;

        uint32 memberHash = StringHash(memberName);
        for (const auto& member : buf->members)
        {
            if (member.nameHash == memberHash)
            {
                outOffset = member.offset;
                outSize = member.size;
                return true;
            }
        }
        return false;
    }

    void BuildLookup()
    {
        bufferLookup.clear();
        for (uint32 i = 0; i < static_cast<uint32>(bufferBindings.size()); ++i)
        {
            bufferBindings[i].nameHash = StringHash(bufferBindings[i].name);
            bufferLookup[bufferBindings[i].nameHash] = i;

            for (auto& member : bufferBindings[i].members)
            {
                member.nameHash = StringHash(member.name);
            }
        }
        for (auto& tex : textureBindings)
        {
            tex.nameHash = StringHash(tex.name);
        }
        for (auto& samp : samplerBindings)
        {
            samp.nameHash = StringHash(samp.name);
        }
    }
};

// Per-stage compile result
struct HS_SHADER_SYSTEM_API ShaderStageOutput
{
    EShaderStage stage = EShaderStage::None;
    std::vector<uint8> bytecode;
    std::string entryPoint;     // SPIRV: "main", Metal: original name
    EShaderLanguage language = EShaderLanguage::Invalid;
    bool isValid = false;
};

// Full compile output
struct HS_SHADER_SYSTEM_API ShaderCompileOutputEx
{
    std::vector<ShaderStageOutput> stages;
    ShaderReflectionDataEx reflection;
    std::string diagnostics;
    bool isValid = false;

    const ShaderStageOutput* GetStageOutput(EShaderStage stage) const
    {
        for (const auto& s : stages)
        {
            if (s.stage == stage) return &s;
        }
        return nullptr;
    }
};

HS_NS_END

#endif
