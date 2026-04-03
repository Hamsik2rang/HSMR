#ifndef __HS_RENDER_GRAPH_RESOURCE_H__
#define __HS_RENDER_GRAPH_RESOURCE_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"
#include "RHI/CommandHandle.h"
#include "RHI/ResourceHandle.h"

HS_NS_BEGIN

#pragma region RenderGraph
enum class ERGPassFlag : uint16
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

HS_FORCEINLINE ERGPassFlag operator&(const ERGPassFlag& lhs, const ERGPassFlag& rhs)
{
    return static_cast<ERGPassFlag>(static_cast<uint16>(lhs) & static_cast<uint16>(rhs));
}

HS_FORCEINLINE ERGPassFlag operator|(const ERGPassFlag& lhs, const ERGPassFlag& rhs)
{
    return static_cast<ERGPassFlag>(static_cast<uint16>(lhs) | static_cast<uint16>(rhs));
}

HS_FORCEINLINE bool operator==(ERGPassFlag lhs, ERGPassFlag rhs)
{
    return static_cast<uint16>(lhs) == static_cast<uint16>(rhs);
}

HS_FORCEINLINE bool operator!=(ERGPassFlag lhs, ERGPassFlag rhs)
{
    return !(lhs == rhs);
}

enum class ERGBufferAccess
{
    ReadOnly = 0,
    ReadWrite, // SSBO
};

enum class ERGTextureAccess
{
    ReadOnly = 0,
    ColorAttachmentWrite,
    ReadWrite, // General 레이아웃, UAV에 해당
    DepthAttachmentRead,
    DepthAttachmentWrite,
    DepthStencilAttachmentRead,
    DepthStencilAttachmentWrite,
    TransferRead,
    TransferWrite,
    ComputeShaderRead,
    ComputeShaderWrite, // Compute UAV Write에 해당
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
    size_t byteSize = 0;
};

class RGPass;

class RGResource
{
    friend class RenderGraphBuilder;

public:
    enum class EType
    {
        Texture,
        Buffer,
        // UniformBuffer, // TODO: Uniform Buffer는 RGBuffer로 표현할 수 있도록 해야 합니다. (예: ReadOnly 접근 권한)
    };
    RGResource(EType type) : _type{ type } {}

    HS_FORCEINLINE bool IsCulled() const { return _refCount == 0; }

    virtual RHIHandle* GetRHIHandle() const = 0;

    // LinearAllocator는 소멸자를 호출하지 않으므로 Reset 전에 명시적으로 호출해야 합니다.
    virtual void Cleanup()
    {
        _writers.clear();
        _readers.clear();
        _refCount = 0;
    }

protected:
    EType _type;
    uint32 _refCount = 0;

    std::vector<RGPass*> _writers;
    std::vector<RGPass*> _readers;
};

class RGTexture : public RGResource
{
    friend class RenderGraphBuilder;

public:
    RGTexture(RGTextureDescriptor desc)
        : RGResource(EType::Texture)
        , _desc(desc)
        , _rhiTexture(nullptr)
    {
    }
    RHIHandle* GetRHIHandle() const override { return static_cast<RHIHandle*>(_rhiTexture); }

private:
    RGTextureDescriptor _desc;
    RHITexture* _rhiTexture;
};

// TODO: Compute Pass에서 UAV로 사용할 수 있는 버퍼는 RGBuffer로 표현할 수 있도록 해야 합니다. (예: ReadWrite 접근 권한)
// Graphics Pass에선 사용할 일이 없습니다.
class RGBuffer : public RGResource
{
    friend class RenderGraphBuilder;

public:
    RGBuffer(RGBufferDescriptor desc)
        : RGResource(EType::Buffer)
        , _desc(desc)
        , _rhiBuffer(nullptr)
    {
    }
    RHIHandle* GetRHIHandle() const override { return static_cast<RHIHandle*>(_rhiBuffer); }

private:
    RGBufferDescriptor _desc;
    RHIBuffer* _rhiBuffer;
};

class RGPass
{
    friend class RenderGraphBuilder;

public:
    RGPass(const char* name, ERGPassFlag flag)
        : _name(name)
        , _flags(flag)
        , _isExecuted(false)
        , _isCompiled(false)
        , _isCulled(false)
    {
    }

    virtual ~RGPass() = default;

    virtual void Execute(RHICommandBuffer& cmdBuffer) = 0;

    // LinearAllocator는 소멸자를 호출하지 않으므로 Reset 전에 명시적으로 호출해야 합니다.
    virtual void Cleanup()
    {
        _upstreams.clear();
        _downstreams.clear();
        _isChecked  = false;
        _isCompiled = false;
        _isExecuted = false;
        _isCulled   = false;
    }

    HS_FORCEINLINE bool IsCulled() const { return _isCulled; }
    HS_FORCEINLINE bool IsCompiled() const { return _isCompiled; }
    HS_FORCEINLINE bool IsExecuted() const { return _isExecuted; }

    HS_FORCEINLINE const char* GetName() const { return _name; }
    HS_FORCEINLINE ERGPassFlag GetFlags() const { return _flags; }

private:
    const char* _name;
    bool _isExecuted   = false;
    bool _isCompiled   = false;
    bool _isCulled     = false;
    ERGPassFlag _flags = ERGPassFlag::None;
    std::vector<RGPass*> _upstreams;   // 먼저 실행되어야 하는 패스들
    std::vector<RGPass*> _downstreams; // 이 패스 이후에 실행되어야 하는 패스들

    bool _isChecked = false; // Topological Sort에서 임시로 사용되는 플래그
};

template <typename TPassParams>
class RGLambdaPass : public RGPass
{
public:
    RGLambdaPass(const char* name, ERGPassFlag flags, TPassParams* params, std::function<void(RHICommandBuffer&)> fnExecute)
        : RGPass(name, flags)
        , _params(params)
        , _fnExecute(std::move(fnExecute))
    {
    }
    ~RGLambdaPass() override = default;

    void Execute(RHICommandBuffer& cmdBuffer) override
    {
        _fnExecute(cmdBuffer);
    }

    void Cleanup() override
    {
        RGPass::Cleanup();
        _fnExecute = nullptr; // std::function 내부 힙 할당 해제
    }

private:
    TPassParams* _params;
    std::function<void(RHICommandBuffer&)> _fnExecute;
};

HS_NS_END

#pragma endregion

#endif
