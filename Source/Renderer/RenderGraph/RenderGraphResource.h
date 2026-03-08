#ifndef __HS_RENDER_GRAPH_RESOURCE_H__
#define __HS_RENDER_GRAPH_RESOURCE_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"

HS_NS_BEGIN

#pragma region RenderGraph
enum class ERGPassFlag
{
    None           = 0,
    Raster         = 1 << 0,
    Compute        = 1 << 1,
    AsyncCompute   = 1 << 2,
    Copy           = 1 << 3,
    NeverCull      = 1 << 4,
    SkipRenderPass = 1 << 5,
    NeverMerge     = 1 << 6,
    NeverParallel  = 1 << 7,
};

enum class ERGBufferAccess
{
    ReadOnly = 0,
    ReadWrite, // SSBO
};

enum class ERGTextureAccess
{
    ReadOnly = 0,
    ColorAttachmentWrite,
    ReadWrite, // ← General 레이아웃, UAV에 해당
    DepthAttachmentRead,
    DepthAttachmentWrite,
    DepthStencilAttachmentRead,
    DepthStencilAttachmentWrite,
    TransferRead,
    TransferWrite,
    ComputeShaderRead,
    ComputeShaderWrite, // ← Compute UAV Write에 해당
    FragmentShaderReadSampledImageOrUniformTexelBuffer,
    Present
};

struct RGTextureDescriptor
{
    TextureInfo info;
    ERGTextureAccess access;
    const char* name;
};

struct RGBufferDescriptor
{
    BufferInfo info;
    ERGBufferAccess access;
    const char* name;
};

class RGHandle
{
public:
    bool IsCulled() const
    {
        return _refCount == 0;
    }

protected:
    uint32 _version  = 0;
    uint32 _refCount = 0;
};

class RGTexture : public RGHandle
{
public:
private:
    RHITexture* _rhiTexture;
};

// TODO: Compute Pass에서 UAV로 사용할 수 있는 버퍼는 RGBuffer로 표현할 수 있도록 해야 합니다. (예: ReadWrite 접근 권한)
// Graphics Pass에선 사용할 일이 없습니다.
class RGBuffer : public RGHandle
{
public:
private:
    RHIBuffer* _rhiBuffer;
};

class RGPass : public RGHandle
{
public:
private:

};

HS_NS_END

#pragma endregion

#endif