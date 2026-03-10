#ifndef __HS_RENDER_GRAPH_BUILDER_H__
#define __HS_RENDER_GRAPH_BUILDER_H__

#include "Precompile.h"

#include "RHI/CommandHandle.h"
#include "Renderer/RenderDefinition.h"

#include "Renderer/RenderGraph/RenderGraphResource.h"

#include <functional>

HS_NS_BEGIN

struct RGPassParameters
{
    std::vector<RGTextureDescriptor> textures;
    std::vector<RGBufferDescriptor> buffers;
};

class RenderGraphBuilder;

class  RGPass
{
    friend class RenderGraphBuilder;

public:
    RGPass(const char* name, const RGPassParameters& params, std::function<void(RHICommandBuffer&)> fnExecute)
        : _name(name)
        , _params(params)
        , _fnExecute(fnExecute)
        , _isExecuted(false)
        , _isCompiled(false)
        , _isCulled(false)
    {
    }

    virtual ~RGPass();

    HS_FORCEINLINE void Execute(RHICommandBuffer& cmdBuffer)
    {
        _fnExecute(cmdBuffer);
        _isExecuted = true;
    }

    HS_FORCEINLINE bool IsCulled() const { return _isCulled; }
    HS_FORCEINLINE bool IsCompiled() const { return _isCompiled; }
    HS_FORCEINLINE bool IsExecuted() const { return _isExecuted; }

    HS_FORCEINLINE const char* GetName() const { return _name; }


private:
    const char* _name;
    RGPassParameters _params;
    std::function<void(RHICommandBuffer&)> _fnExecute;
    bool _isExecuted = false;
    bool _isCompiled = false;
    bool _isCulled   = false;

    std::vector<RGPass*> _upstreams;   // 먼저 실행되어야 하는 패스들
    std::vector<RGPass*> _downstreams; // 먼저 실행되어야 하는 패스들에 의존하는 패스들, 즉 이 패스가 먼저 실행되어야 하는 패스들
};

class RenderGraphBuilder
{
public:
    RenderGraphBuilder();
    ~RenderGraphBuilder();

    RGTexture* RegisterExternalTexture(RHITexture* texture);
    void UnregisterExternalTexture(RHITexture* texture);
    RGTexture* AcquireTexture(const RGTextureDescriptor& desc);
    RGTexture* FindTexture(RHITexture* texture) const;
    RGTexture* FindTexture(uint32 id) const;

    RGBuffer* RegisterExternalBuffer(RHIBuffer* buffer);
    void UnregisterExternalBuffer(RHIBuffer* buffer);
    RGBuffer* AcquireBuffer(const RGBufferDescriptor& desc);
    RGBuffer* FindBuffer(RHIBuffer* buffer) const;

    void AddPass(const char* passName, const RGPassParameters& passParams, std::function<void(RHICommandBuffer&)> fnExecute);

    void Setup(RHICommandBuffer* cmdBuffer);
    void Compile();
    void Execute();
    void Reset();

private:
    void addDependency(RGTextureDescriptor& desc, RGPass* pass);
    void addDependency(RGBufferDescriptor& desc, RGPass* pass);
    void traverse(RGPass* pass);

    RHICommandBuffer* _currentCmdBuffer = nullptr;

    uint8 _frameIndex                          = static_cast<uint8>(-1);
    constexpr static uint8 s_maxFramesInFlight = 2;

    std::vector<RGTexture*> _textures[s_maxFramesInFlight];
    std::vector<RGBuffer*> _buffers[s_maxFramesInFlight];

    std::vector<RGPass> _passes;
};

HS_NS_END

#endif