#include "ShaderSystem/ShaderSystemDefinition.h"

HS_NS_BEGIN

const ShaderBufferBindingInfo* ShaderReflectionDataEx::FindBuffer(const std::string& name) const
{
    uint32 hash = StringHash(name);
    auto it = bufferLookup.find(hash);
    if (it != bufferLookup.end() && it->second < bufferBindings.size())
    {
        return &bufferBindings[it->second];
    }
    return nullptr;
}

bool ShaderReflectionDataEx::FindMemberOffset(const std::string& bufferName, const std::string& memberName,
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

void ShaderReflectionDataEx::BuildLookup()
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

HS_NS_END
