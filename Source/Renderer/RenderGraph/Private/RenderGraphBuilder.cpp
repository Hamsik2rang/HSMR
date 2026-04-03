#include "Renderer/RenderGraph/RenderGraphBuilder.h"

#include <algorithm>

HS_NS_BEGIN

RenderGraphBuilder::RenderGraphBuilder()
{}

RenderGraphBuilder::~RenderGraphBuilder()
{}

void RenderGraphBuilder::Initialize(RHIContext* rhiContext)
{
    _rhiContext = rhiContext;
}

// =============================================================================
// 리소스 등록 / 생성 / 조회
// =============================================================================

RGTexture* RenderGraphBuilder::RegisterExternalTexture(RHITexture* texture)
{
    auto it = _externalRHIHandleMap.find(texture);
    if (it != _externalRHIHandleMap.end())
    {
        return static_cast<RGTexture*>(it->second);
    }

    RGTextureDescriptor desc =
    {
        .info   = texture->info,
        .access = ERGTextureAccess::ReadOnly,
        .name   = texture->GetName()
    };
    RGTexture* rgTexture        = _allocator.Allocate<RGTexture>(_frameIndex, desc);
    rgTexture->_rhiTexture      = texture;
    _externalRHIHandleMap[texture] = rgTexture;
    _allResources.push_back(rgTexture);

    return rgTexture;
}

RGTexture* RenderGraphBuilder::CreateTexture(const RGTextureDescriptor& desc)
{
    RGTexture* texture = _allocator.Allocate<RGTexture>(_frameIndex, desc);
    _allResources.push_back(texture);
    return texture;
}

RGTexture* RenderGraphBuilder::FindTexture(RHITexture* texture) const
{
    auto it = _externalRHIHandleMap.find(texture);
    if (it != _externalRHIHandleMap.end())
    {
        return static_cast<RGTexture*>(it->second);
    }
    return nullptr;
}

RGTexture* RenderGraphBuilder::FindTexture(uint32 id) const
{
    for (auto* resource : _allResources)
    {
        if (resource->_type != RGResource::EType::Texture)
        {
            continue;
        }
        RGTexture* tex = static_cast<RGTexture*>(resource);
        if (tex->_rhiTexture != nullptr && tex->_rhiTexture->GetHash() == id)
        {
            return tex;
        }
    }
    return nullptr;
}

RGBuffer* RenderGraphBuilder::RegisterExternalBuffer(RHIBuffer* buffer)
{
    auto it = _externalRHIHandleMap.find(buffer);
    if (it != _externalRHIHandleMap.end())
    {
        return static_cast<RGBuffer*>(it->second);
    }

    RGBufferDescriptor desc
    {
        .info     = buffer->info,
        .access   = ERGBufferAccess::ReadOnly,
        .name     = buffer->GetName(),
        .byteSize = buffer->byteSize
    };
    RGBuffer* rgBuffer           = _allocator.Allocate<RGBuffer>(_frameIndex, desc);
    rgBuffer->_rhiBuffer         = buffer;
    _externalRHIHandleMap[buffer] = rgBuffer;
    _allResources.push_back(rgBuffer);

    return rgBuffer;
}

RGBuffer* RenderGraphBuilder::CreateBuffer(const RGBufferDescriptor& desc)
{
    RGBuffer* buffer = _allocator.Allocate<RGBuffer>(_frameIndex, desc);
    _allResources.push_back(buffer);
    return buffer;
}

RGBuffer* RenderGraphBuilder::FindBuffer(RHIBuffer* buffer) const
{
    auto it = _externalRHIHandleMap.find(buffer);
    if (it != _externalRHIHandleMap.end())
    {
        return static_cast<RGBuffer*>(it->second);
    }
    return nullptr;
}

// =============================================================================
// 의존성 등록
// Read/Write는 호출 시점에 즉시 의존성 그래프를 구성합니다.
// 따라서 Compile Phase 1에서는 추가 엣지 구성 없이 Culling만 수행합니다.
// =============================================================================

void RenderGraphBuilder::Read(RGPass* pass, RGTexture* texture, ERGTextureAccess access)
{
    texture->_readers.push_back(pass);
    texture->_refCount++;

    // 이 텍스처를 쓰는 패스들은 현재 패스보다 먼저 실행되어야 합니다.
    for (auto* writer : texture->_writers)
    {
        pass->_upstreams.push_back(writer);
        writer->_downstreams.push_back(pass);
    }

    RGResourceAccess resAccess;
    resAccess.resource      = texture;
    resAccess.isWrite       = false;
    resAccess.textureAccess = access;
    resAccess.bufferAccess  = ERGBufferAccess::ReadOnly;
    _resourceDependencyMap[pass].push_back(resAccess);
}

void RenderGraphBuilder::Write(RGPass* pass, RGTexture* texture, ERGTextureAccess access)
{
    texture->_writers.push_back(pass);
    texture->_refCount++;

    // 이 텍스처를 읽는 패스들은 현재 패스보다 나중에 실행되어야 합니다.
    for (auto* reader : texture->_readers)
    {
        pass->_downstreams.push_back(reader);
        reader->_upstreams.push_back(pass);
    }

    RGResourceAccess resAccess;
    resAccess.resource      = texture;
    resAccess.isWrite       = true;
    resAccess.textureAccess = access;
    resAccess.bufferAccess  = ERGBufferAccess::ReadOnly;
    _resourceDependencyMap[pass].push_back(resAccess);
}

void RenderGraphBuilder::Read(RGPass* pass, RGBuffer* buffer, ERGBufferAccess access)
{
    buffer->_readers.push_back(pass);
    buffer->_refCount++;

    for (auto* writer : buffer->_writers)
    {
        pass->_upstreams.push_back(writer);
        writer->_downstreams.push_back(pass);
    }

    RGResourceAccess resAccess;
    resAccess.resource      = buffer;
    resAccess.isWrite       = false;
    resAccess.textureAccess = ERGTextureAccess::ReadOnly;
    resAccess.bufferAccess  = access;
    _resourceDependencyMap[pass].push_back(resAccess);
}

void RenderGraphBuilder::Write(RGPass* pass, RGBuffer* buffer, ERGBufferAccess access)
{
    buffer->_writers.push_back(pass);
    buffer->_refCount++;

    for (auto* reader : buffer->_readers)
    {
        pass->_downstreams.push_back(reader);
        reader->_upstreams.push_back(pass);
    }

    RGResourceAccess resAccess;
    resAccess.resource      = buffer;
    resAccess.isWrite       = true;
    resAccess.textureAccess = ERGTextureAccess::ReadOnly;
    resAccess.bufferAccess  = access;
    _resourceDependencyMap[pass].push_back(resAccess);
}

// =============================================================================
// 프레임 생명주기
// =============================================================================

void RenderGraphBuilder::Setup(RHICommandBuffer* cmdBuffer)
{
    _currentCmdBuffer = cmdBuffer;
    _frameIndex       = (_frameIndex + 1) % s_maxFramesInFlight;
}

void RenderGraphBuilder::Compile()
{
    size_t executablePassCount = 0;

    // -------------------------------------------------------------------------
    // Phase 1: Culling
    // 리소스 의존성이 없고 NeverCull 플래그가 없는 패스를 컬링합니다.
    // -------------------------------------------------------------------------
    for (auto* pass : _passes)
    {
        HS_ASSERT(pass->IsCompiled() == false, "Pass '%s' is already compiled!", pass->GetName());

        bool hasAnyDependency = (_resourceDependencyMap.count(pass) > 0)
                                && (!_resourceDependencyMap[pass].empty());
        bool neverCull        = ERGPassFlag::None != (pass->GetFlags() & ERGPassFlag::NeverCull);

        if (!hasAnyDependency && !neverCull)
        {
            pass->_isCulled = true;
        }
        else
        {
            executablePassCount++;
        }
    }

    // -------------------------------------------------------------------------
    // Phase 2: Topological Sort (DFS)
    // 의존성 그래프를 위상 정렬하여 실행 순서를 결정합니다.
    // -------------------------------------------------------------------------
    _executablePasses.clear();
    _executablePasses.reserve(executablePassCount);

    // static 대신 로컬 변수를 사용합니다 (static은 프레임 간 잔여 데이터가 남는 버그 유발).
    std::vector<RGPass*> dfsStack;
    dfsStack.reserve(executablePassCount);

    for (auto* startPass : _passes)
    {
        RGPass* curPass = startPass;

        while (!curPass->_isChecked && !curPass->IsCulled())
        {
            bool allUpstreamsReady = true;
            for (auto* up : curPass->_upstreams)
            {
                // 컬링된 upstream은 이미 처리된 것으로 취급합니다.
                if (!up->_isChecked && !up->IsCulled())
                {
                    allUpstreamsReady = false;
                    dfsStack.push_back(curPass);
                    curPass = up;
                    break;
                }
            }

            if (allUpstreamsReady)
            {
                curPass->_isChecked  = true;
                curPass->_isCompiled = true;
                _executablePasses.push_back(curPass);

                if (dfsStack.empty())
                {
                    break;
                }
                curPass = dfsStack.back();
                dfsStack.pop_back();
            }
        }
    }

    HS_ASSERT(_executablePasses.size() == executablePassCount,
        "Topological Sort Fail! Sorted Pass: %zu, Expetation: %zu",
        _executablePasses.size(), executablePassCount);

    // -------------------------------------------------------------------------
    // Phase 3: RHI 리소스 풀 할당 (수명 기반 aliasing)
    //
    // 각 리소스의 수명 [firstPassIdx, lastPassIdx]를 계산합니다.
    // forward 순회하면서:
    //   - i-1에서 수명이 끝난 리소스를 freed 풀로 반환 (다음 alloc에서 재사용 가능)
    //   - i에서 수명이 시작하는 리소스를 freed 풀에서 할당 (없으면 신규 생성)
    // 수명이 겹치지 않는 동일 스펙의 리소스는 동일한 RHI 핸들을 공유합니다.
    // -------------------------------------------------------------------------
    struct ResourceLifetime
    {
        int firstPassIdx = -1;
        int lastPassIdx  = -1;
    };
    std::unordered_map<RGResource*, ResourceLifetime> lifetimes;

    const int n = static_cast<int>(_executablePasses.size());
    for (int i = 0; i < n; i++)
    {
        auto it = _resourceDependencyMap.find(_executablePasses[i]);
        if (it == _resourceDependencyMap.end())
        {
            continue;
        }
        for (auto& access : it->second)
        {
            auto& lt = lifetimes[access.resource];
            if (lt.firstPassIdx < 0)
            {
                lt.firstPassIdx = i;
            }
            lt.lastPassIdx = i;
        }
    }

    auto isExternal = [&](RGResource* resource) -> bool
    {
        RHIHandle* handle = resource->GetRHIHandle();
        return (handle != nullptr) && (_externalRHIHandleMap.count(handle) > 0);
    };

    auto allocTexture = [&](RGTexture* rgTex)
    {
        auto& freedMap = _rhiTextureRegistry._freedTextures;
        auto& usedMap  = _rhiTextureRegistry._usedTextures;
        const TextureInfo& info = rgTex->_desc.info;

        auto freedIt = freedMap.find(info);
        if (freedIt != freedMap.end() && !freedIt->second.empty())
        {
            // 풀에서 재사용
            rgTex->_rhiTexture = freedIt->second.back();
            freedIt->second.pop_back();
        }
        else
        {
            // 신규 생성 — 소유권은 _ownedTextures가 가집니다.
            const char* name = rgTex->_desc.name ? rgTex->_desc.name : "RenderGraph Transient Texture";
            RHITexture* newTex = RHIContext::Get()->CreateTexture(name, nullptr, info);
            HS_ASSERT(newTex != nullptr, "RGTexture '%s' RHI 텍스처 생성 실패!", name);
            _ownedTextures.push_back(Scoped<RHITexture>(newTex));
            rgTex->_rhiTexture = newTex;
        }
        usedMap[info].push_back(rgTex->_rhiTexture);
    };

    auto freeTexture = [&](RGTexture* rgTex)
    {
        const TextureInfo& info = rgTex->_desc.info;
        auto& usedPool = _rhiTextureRegistry._usedTextures[info];
        auto usedIt    = std::find(usedPool.begin(), usedPool.end(), rgTex->_rhiTexture);
        if (usedIt != usedPool.end())
        {
            usedPool.erase(usedIt);
        }
        _rhiTextureRegistry._freedTextures[info].push_back(rgTex->_rhiTexture);
    };

    auto allocBuffer = [&](RGBuffer* rgBuf)
    {
        auto& freedMap = _rhiBufferRegistry._freedBuffers;
        auto& usedMap  = _rhiBufferRegistry._usedBuffers;
        const BufferInfo& info = rgBuf->_desc.info;

        auto freedIt = freedMap.find(info);
        if (freedIt != freedMap.end() && !freedIt->second.empty())
        {
            rgBuf->_rhiBuffer = freedIt->second.back();
            freedIt->second.pop_back();
        }
        else
        {
            const char* name = rgBuf->_desc.name ? rgBuf->_desc.name : "RenderGraph Transient Buffer";
            RHIBuffer* newBuf = RHIContext::Get()->CreateBuffer(name, nullptr, rgBuf->_desc.byteSize, info);
            HS_ASSERT(newBuf != nullptr, "RGBuffer '%s' RHI 버퍼 생성 실패!", name);
            _ownedBuffers.push_back(Scoped<RHIBuffer>(newBuf));
            rgBuf->_rhiBuffer = newBuf;
        }
        usedMap[info].push_back(rgBuf->_rhiBuffer);
    };

    auto freeBuffer = [&](RGBuffer* rgBuf)
    {
        const BufferInfo& info = rgBuf->_desc.info;
        auto& usedPool = _rhiBufferRegistry._usedBuffers[info];
        auto usedIt    = std::find(usedPool.begin(), usedPool.end(), rgBuf->_rhiBuffer);
        if (usedIt != usedPool.end())
        {
            usedPool.erase(usedIt);
        }
        _rhiBufferRegistry._freedBuffers[info].push_back(rgBuf->_rhiBuffer);
    };

    // n+1번 순회: i=0..n-1 에서 할당/해제, i=n 에서 마지막 정리
    for (int i = 0; i <= n; i++)
    {
        // Step A: i-1 에서 수명이 끝난 리소스를 freed 풀로 반환합니다.
        // 이후 pass i 의 alloc에서 재사용될 수 있습니다 (aliasing).
        if (i > 0)
        {
            for (auto& [resource, lt] : lifetimes)
            {
                if (lt.lastPassIdx != i - 1 || isExternal(resource))
                {
                    continue;
                }
                if (resource->_type == RGResource::EType::Texture)
                {
                    freeTexture(static_cast<RGTexture*>(resource));
                }
                else if (resource->_type == RGResource::EType::Buffer)
                {
                    freeBuffer(static_cast<RGBuffer*>(resource));
                }
            }
        }

        if (i == n)
        {
            break;
        }

        // Step B: pass i 에서 수명이 시작하는 리소스를 할당합니다.
        for (auto& [resource, lt] : lifetimes)
        {
            if (lt.firstPassIdx != i || isExternal(resource))
            {
                continue;
            }
            if (resource->_type == RGResource::EType::Texture)
            {
                allocTexture(static_cast<RGTexture*>(resource));
            }
            else if (resource->_type == RGResource::EType::Buffer)
            {
                allocBuffer(static_cast<RGBuffer*>(resource));
            }
        }
    }
}

void RenderGraphBuilder::Execute()
{
    for (auto* pass : _executablePasses)
    {
        // 리소스 배리어 삽입: 필요한 접근 상태로 전환합니다.
        // NOTE: 현재 RHI TextureBarrier()는 텍스처 인수만 받습니다.
        //       레이아웃/스테이지 정보를 갖춘 확장이 필요하나,
        //       v1에서는 상태가 달라질 때 full barrier를 삽입합니다.
        auto depIt = _resourceDependencyMap.find(pass);
        if (depIt != _resourceDependencyMap.end())
        {
            for (auto& access : depIt->second)
            {
                if (access.resource->_type != RGResource::EType::Texture)
                {
                    continue;
                }
                RGTexture* rgTex = static_cast<RGTexture*>(access.resource);
                if (rgTex->_rhiTexture == nullptr)
                {
                    continue;
                }

                auto stateIt = _resourceCurrentAccess.find(access.resource);
                ERGTextureAccess currentAccess = (stateIt != _resourceCurrentAccess.end())
                    ? stateIt->second
                    : ERGTextureAccess::ReadOnly;

                if (currentAccess != access.textureAccess)
                {
                    _currentCmdBuffer->TextureBarrier(rgTex->_rhiTexture);
                    _resourceCurrentAccess[access.resource] = access.textureAccess;
                }
            }
        }

        pass->Execute(*_currentCmdBuffer);
        pass->_isExecuted = true;
    }
}

void RenderGraphBuilder::Reset()
{
    // LinearAllocator는 소멸자를 호출하지 않습니다.
    // std::vector/std::function 등 힙 할당을 포함하는 객체를 명시적으로 정리합니다.
    for (auto* pass : _passes)
    {
        pass->Cleanup();
    }
    for (auto* resource : _allResources)
    {
        resource->Cleanup();
    }

    _passes.clear();
    _allResources.clear();
    _executablePasses.clear();
    _resourceDependencyMap.clear();
    _externalRHIHandleMap.clear();
    _resourceCurrentAccess.clear();
    _allocator.Reset(_frameIndex);
    _currentCmdBuffer = nullptr;
}

HS_NS_END
