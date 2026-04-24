#ifndef __HS_RENDER_GRAPH_BUILDER_H__
#define __HS_RENDER_GRAPH_BUILDER_H__

#include "Precompile.h"

#include "RHI/CommandHandle.h"
#include "RHI/RHIContext.h"
#include "Renderer/RenderDefinition.h"
#include "Renderer/RenderGraph/RenderGraphTransientAllocator.h"
#include "Renderer/RenderGraph/RenderGraphResource.h"

#include "Core/Memory/MemoryPool.h"

#include <functional>
#include <concepts>

HS_NS_BEGIN

class HS_RENDERER_API RenderGraphBuilder
{
public:
    // 리소스-패스 간 접근 방식을 기록하는 구조체.
    // Read()/Write() 호출 시 생성되며 Compile/Execute 단계에서 사용됩니다.
    struct RGResourceAccess
    {
        RGResource*       resource;
        bool              isWrite;
        ERGTextureAccess  textureAccess; // resource->_type == Texture일 때 유효
        ERGBufferAccess   bufferAccess;  // resource->_type == Buffer일 때 유효
    };

public:
    RenderGraphBuilder();
    RenderGraphBuilder(const RenderGraphBuilder&) = delete;
    ~RenderGraphBuilder();

    RenderGraphBuilder& operator=(const RenderGraphBuilder&) = delete;

    void Initialize(RHIContext* rhiContext);

    // -----------------------------------------------------------------------
    // 리소스 등록 / 생성 / 조회
    // -----------------------------------------------------------------------

    // 스왑체인 등 외부에서 이미 생성된 텍스처를 RenderGraph에 등록합니다.
    RGTexture* RegisterExternalTexture(RHITexture* texture);

    // RenderGraph가 생명주기를 관리하는 트랜지언트 텍스처를 생성합니다.
    RGTexture* CreateTexture(const RGTextureDescriptor& desc);

    RGTexture* FindTexture(RHITexture* texture) const;
    RGTexture* FindTexture(uint32 id) const;

    RGBuffer* RegisterExternalBuffer(RHIBuffer* buffer);
    RGBuffer* CreateBuffer(const RGBufferDescriptor& desc);
    RGBuffer* FindBuffer(RHIBuffer* buffer) const;

    // -----------------------------------------------------------------------
    // 의존성 등록 — Setup 람다 내에서 호출합니다.
    // -----------------------------------------------------------------------

    void Read(RGPass* pass, RGTexture* texture,
              ERGTextureAccess access = ERGTextureAccess::ReadOnly);
    void Write(RGPass* pass, RGTexture* texture,
               ERGTextureAccess access = ERGTextureAccess::ColorAttachmentWrite);

    void Read(RGPass* pass, RGBuffer* buffer,
              ERGBufferAccess access = ERGBufferAccess::ReadOnly);
    void Write(RGPass* pass, RGBuffer* buffer,
               ERGBufferAccess access = ERGBufferAccess::ReadWrite);

    // -----------------------------------------------------------------------
    // 패스 추가
    // -----------------------------------------------------------------------

    template <typename TPassParams,
        std::invocable<RenderGraphBuilder&, RGPass*, TPassParams*> TFnSetup,
        std::invocable<RHICommandBuffer&> TFnExecute>
    void AddPass(const char* passName,
        ERGPassFlag passFlags,
        TPassParams* passParams,
        TFnSetup fnSetup,
        TFnExecute fnExecute)
    {
        RGPass* pass = _allocator.Allocate<RGLambdaPass<TPassParams>>(
            _frameIndex, passName, passFlags, passParams, std::move(fnExecute));
        fnSetup(*this, pass, passParams);
        _passes.push_back(pass);
    }

    // -----------------------------------------------------------------------
    // 프레임 생명주기
    // -----------------------------------------------------------------------

    // 새 프레임을 시작합니다. Compile/Execute 전에 호출해야 합니다.
    void Setup(RHICommandBuffer* cmdBuffer);

    // 의존성 그래프 분석, 위상 정렬, RHI 리소스 풀 할당을 수행합니다.
    void Compile();

    // Compile 이후 실제 패스를 순서대로 실행합니다. 배리어를 자동으로 삽입합니다.
    void Execute();

    // 프레임 종료 후 호출합니다. 패스/리소스 객체를 정리하고 할당자를 리셋합니다.
    void Reset();

private:
    void allocTexture(RGTexture* rgTexture);
    void freeTexture(RGTexture* rgTexture);
    void allocBuffer(RGBuffer* rgBufer);
    void freeBuffer(RGBuffer* rgBuffer);
    bool isExternalResource(RGResource* rgResource) const;


    RHIContext*       _rhiContext       = nullptr;
    RHICommandBuffer* _currentCmdBuffer = nullptr;
    RenderGraphTransientAllocator _transientAllocator;

    uint8 _frameIndex = static_cast<uint8>(-1);
    constexpr static uint8 s_maxFramesInFlight = 2;

    // 이 빌더가 소유하는 모든 RHI 리소스 (풀에서 재사용되며 빌더 소멸 시 파괴됩니다).
    std::vector<Scoped<RHITexture>> _ownedTextures;
    std::vector<Scoped<RHIBuffer>>  _ownedBuffers;

    std::vector<RGPass*>     _passes;
    std::vector<RGPass*>     _executablePasses;
    std::vector<RGResource*> _allResources; // 소멸자 호출용 추적 목록

    // Pass → 해당 패스가 사용하는 리소스 접근 목록
    std::unordered_map<RGPass*, std::vector<RGResourceAccess>> _resourceDependencyMap;
    std::unordered_map<RGPass*, std::vector<RHITextureBarrierDesc>> _textureBarriers;
    std::unordered_map<RGPass*, std::vector<RHITextureBarrierDesc>> _texturePostBarriers;

    // TODO: BufferBarrierDesc도 필요할 수 있습니다(ex. Compute UAV Write -> Read 의존성)

    // 외부에서 등록된 RHIHandle → RGResource 역방향 매핑
    std::unordered_map<RHIHandle*, RGResource*> _externalRHIHandleMap;

    // Execute 시 배리어 삽입을 위한 현재 리소스 접근 상태 추적
    std::unordered_map<RGResource*, ERGTextureAccess> _resourceCurrentAccess;

    struct RHITextureRegistry
    {
        std::unordered_map<TextureInfo, std::vector<RHITexture*>> _freedTextures;
        std::unordered_map<TextureInfo, std::vector<RHITexture*>> _usedTextures;
    } _rhiTextureRegistry;

    struct RHIBufferRegistry
    {
        std::unordered_map<BufferInfo, std::vector<RHIBuffer*>> _freedBuffers;
        std::unordered_map<BufferInfo, std::vector<RHIBuffer*>> _usedBuffers;
    } _rhiBufferRegistry;

    // RGPass/RGResource 객체를 프레임 단위로 할당하는 선형 할당자.
    // Reset 시 오프셋만 리셋되므로 소멸자는 Cleanup()으로 명시적으로 처리합니다.
    LinearAllocator<65536, 8, 2> _allocator;
};

HS_NS_END

#endif
