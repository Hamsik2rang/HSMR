# FrameGraph 설계 문서

> **목표**: 경량화된 Framework 위에서 동작하는 FrameGraph 시스템 설계
> **참고**: Frostbite Frame Graph, Unreal RDG, Godot RenderingDevice
> **작성일**: 2026-01-25

---

## 1. FrameGraph 개요

### 1.1 FrameGraph란?

렌더링 파이프라인을 **선언적 그래프**로 표현하고, **의존성을 자동 분석**하여 실행하는 시스템.

```
기존 방식 (명령형):
    ShadowPass->Execute();      // 순서를 직접 관리
    GBufferPass->Execute();
    SSAOPass->Execute();
    LightingPass->Execute();

FrameGraph 방식 (선언적):
    fg.AddPass("Shadow", ...);     // 순서 상관없이 선언
    fg.AddPass("Lighting", ...);
    fg.AddPass("SSAO", ...);
    fg.AddPass("GBuffer", ...);
    fg.Compile();                  // 의존성 분석 → 순서 자동 결정
    fg.Execute(cmd);
```

### 1.2 핵심 이점

| 이점 | 설명 |
|------|------|
| **자동 의존성 해결** | 패스 간 실행 순서 자동 결정 |
| **리소스 생명주기 관리** | Transient 리소스 자동 할당/해제 |
| **패스 컬링** | 최종 출력에 기여하지 않는 패스 자동 제거 |
| **메모리 최적화** | 리소스 aliasing으로 메모리 재사용 |
| **배리어 자동화** | Vulkan/Metal 동기화 자동 삽입 |
| **시각화/디버깅** | 그래프 구조로 파이프라인 분석 용이 |

---

## 2. 아키텍처 개요

### 2.1 경량 Framework와의 통합

```
Framework/
├── App.h
├── RHI/
├── Shader.h
├── Utility.h
└── Graph/                    # FrameGraph 모듈 (신규)
    ├── FrameGraph.h          # 메인 클래스
    ├── PassNode.h            # 패스 노드
    ├── ResourceNode.h        # 리소스 노드
    ├── PassBuilder.h         # 패스 빌더 (DSL)
    ├── ResourceRegistry.h    # 리소스 레지스트리
    ├── TransientAllocator.h  # Transient 리소스 할당기
    └── BarrierBatcher.h      # 배리어 최적화
```

예상 규모: **~1,500 LOC**

### 2.2 핵심 컴포넌트

```
┌─────────────────────────────────────────────────────────────┐
│                      FrameGraph                              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │                   Build Phase                         │   │
│  │  PassBuilder → PassNode[] + ResourceNode[]           │   │
│  └─────────────────────────────────────────────────────┘   │
│                           ↓                                  │
│  ┌─────────────────────────────────────────────────────┐   │
│  │                  Compile Phase                        │   │
│  │  1. Dependency Analysis (Read/Write tracking)        │   │
│  │  2. Pass Culling (backward propagation)              │   │
│  │  3. Resource Lifetime Calculation                     │   │
│  │  4. Memory Aliasing                                   │   │
│  │  5. Barrier Insertion                                 │   │
│  └─────────────────────────────────────────────────────┘   │
│                           ↓                                  │
│  ┌─────────────────────────────────────────────────────┐   │
│  │                  Execute Phase                        │   │
│  │  TransientAllocator.Allocate()                       │   │
│  │  for each pass in sorted order:                      │   │
│  │      BarrierBatcher.InsertBarriers()                 │   │
│  │      pass.Execute(cmd)                               │   │
│  │  TransientAllocator.Release()                        │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

---

## 3. 핵심 클래스 설계

### 3.1 FrameGraph (메인 클래스)

```cpp
// Framework/Graph/FrameGraph.h
#pragma once
#include "PassNode.h"
#include "ResourceNode.h"
#include "ResourceRegistry.h"

namespace hs {

class FrameGraph {
public:
    FrameGraph(RHIContext* rhi);
    ~FrameGraph();

    //=========================================================================
    // Build Phase - 패스 및 리소스 등록
    //=========================================================================

    // 패스 추가 (람다로 Setup과 Execute 정의)
    template<typename SetupFunc, typename ExecuteFunc>
    PassNode& AddPass(const char* name, SetupFunc&& setup, ExecuteFunc&& execute);

    // 외부 리소스 임포트 (스왑체인 등)
    ResourceHandle Import(const char* name, RHITexture* texture);
    ResourceHandle Import(const char* name, RHIBuffer* buffer);

    //=========================================================================
    // Compile Phase - 그래프 분석 및 최적화
    //=========================================================================

    void Compile();

    //=========================================================================
    // Execute Phase - 실행
    //=========================================================================

    void Execute(RHICommandBuffer* cmd);

    //=========================================================================
    // 디버깅
    //=========================================================================

    void ExportGraphviz(const char* path);  // DOT 형식 출력
    void PrintStats();                       // 패스/리소스 통계

private:
    RHIContext* _rhi;
    ResourceRegistry _registry;

    std::vector<Scoped<PassNode>> _passNodes;
    std::vector<Scoped<ResourceNode>> _resourceNodes;
    std::vector<PassNode*> _sortedPasses;  // 컴파일 결과

    bool _isCompiled = false;

    // Compile 단계 내부 함수
    void BuildDependencyGraph();
    void CullUnusedPasses();
    void CalculateResourceLifetimes();
    void ComputeMemoryAliasing();
    void InsertBarriers();
    void TopologicalSort();
};

} // namespace hs
```

### 3.2 ResourceHandle (리소스 핸들)

```cpp
// Framework/Graph/ResourceNode.h
#pragma once

namespace hs {

// 타입 안전한 리소스 핸들
struct ResourceHandle {
    uint32_t index = UINT32_MAX;
    uint32_t version = 0;  // 같은 슬롯 재사용 감지

    bool IsValid() const { return index != UINT32_MAX; }
    bool operator==(const ResourceHandle& other) const {
        return index == other.index && version == other.version;
    }
};

// 리소스 타입
enum class ResourceType {
    Texture,
    Buffer
};

// 리소스 사용 방식
enum class ResourceUsage : uint8_t {
    None        = 0,
    ShaderRead  = 1 << 0,  // SRV
    ShaderWrite = 1 << 1,  // UAV
    ColorTarget = 1 << 2,  // RTV
    DepthTarget = 1 << 3,  // DSV
    Present     = 1 << 4,  // 스왑체인 출력
};
HS_ENABLE_BITMASK_OPERATORS(ResourceUsage);

// 리소스 노드 (그래프의 노드)
class ResourceNode {
public:
    const char* name;
    ResourceType type;
    ResourceHandle handle;

    // 텍스처 정보
    struct TextureDesc {
        uint32_t width = 0;
        uint32_t height = 0;
        EPixelFormat format = EPixelFormat::RGBA8;
        ETextureUsage usage = ETextureUsage::SAMPLED;
    } textureDesc;

    // 버퍼 정보
    struct BufferDesc {
        size_t size = 0;
        EBufferUsage usage = EBufferUsage::UNIFORM;
    } bufferDesc;

    // 생명주기 (Compile 단계에서 계산)
    uint32_t firstUse = UINT32_MAX;   // 처음 사용하는 패스
    uint32_t lastUse = 0;              // 마지막 사용하는 패스

    // 실제 RHI 리소스 (외부 임포트 또는 Transient 할당)
    RHITexture* texture = nullptr;
    RHIBuffer* buffer = nullptr;
    bool isImported = false;  // 외부에서 임포트된 리소스인가?
    bool isTransient = true;  // FrameGraph가 생명주기 관리하는가?
};

} // namespace hs
```

### 3.3 PassNode (패스 노드)

```cpp
// Framework/Graph/PassNode.h
#pragma once
#include "ResourceNode.h"
#include <functional>
#include <vector>

namespace hs {

class PassBuilder;  // forward declaration

// 패스 실행 컨텍스트 (Execute 람다에 전달)
struct PassContext {
    RHICommandBuffer* cmd;

    // 리소스 접근
    RHITexture* GetTexture(ResourceHandle handle);
    RHIBuffer* GetBuffer(ResourceHandle handle);

    // 편의 함수
    void SetRenderTarget(ResourceHandle color, ResourceHandle depth = {});
    void BindTexture(uint32_t slot, ResourceHandle handle);
    void BindBuffer(uint32_t slot, ResourceHandle handle);
};

// 패스 노드
class PassNode {
public:
    const char* name;
    uint32_t index;  // 정렬 전 원래 인덱스

    // 리소스 의존성
    struct ResourceAccess {
        ResourceHandle handle;
        ResourceUsage usage;
    };
    std::vector<ResourceAccess> reads;   // 읽기 의존성
    std::vector<ResourceAccess> writes;  // 쓰기 의존성

    // 실행 함수
    std::function<void(PassContext&)> executeFunc;

    // 컴파일 결과
    bool isCulled = false;                    // 컬링되었는가?
    std::vector<PassNode*> dependsOn;         // 이 패스가 의존하는 패스들
    std::vector<PassNode*> dependents;        // 이 패스에 의존하는 패스들
    uint32_t sortedIndex = UINT32_MAX;        // 정렬 후 인덱스

    // 배리어 정보 (Execute 직전에 삽입)
    struct BarrierInfo {
        ResourceHandle resource;
        ResourceUsage fromUsage;
        ResourceUsage toUsage;
    };
    std::vector<BarrierInfo> barriers;
};

} // namespace hs
```

### 3.4 PassBuilder (패스 설정 DSL)

```cpp
// Framework/Graph/PassBuilder.h
#pragma once
#include "ResourceNode.h"

namespace hs {

class FrameGraph;
class PassNode;

class PassBuilder {
public:
    PassBuilder(FrameGraph* graph, PassNode* pass);

    //=========================================================================
    // 리소스 생성 (Transient)
    //=========================================================================

    // 텍스처 생성
    ResourceHandle CreateTexture(
        const char* name,
        uint32_t width,
        uint32_t height,
        EPixelFormat format,
        ETextureUsage usage = ETextureUsage::SAMPLED
    );

    // 상대적 크기 텍스처 (스왑체인 기준)
    ResourceHandle CreateTexture(
        const char* name,
        float widthScale,   // 1.0 = 스왑체인과 동일
        float heightScale,
        EPixelFormat format,
        ETextureUsage usage = ETextureUsage::SAMPLED
    );

    // 버퍼 생성
    ResourceHandle CreateBuffer(
        const char* name,
        size_t size,
        EBufferUsage usage
    );

    //=========================================================================
    // 리소스 읽기/쓰기 선언
    //=========================================================================

    // 읽기 (이전 패스의 출력을 입력으로)
    ResourceHandle Read(ResourceHandle input, ResourceUsage usage = ResourceUsage::ShaderRead);

    // 쓰기 (새 버전 생성)
    ResourceHandle Write(ResourceHandle output, ResourceUsage usage = ResourceUsage::ShaderWrite);

    // 렌더 타겟으로 쓰기
    ResourceHandle WriteRenderTarget(ResourceHandle target);
    ResourceHandle WriteDepthStencil(ResourceHandle target);

    // 읽기 + 쓰기 (in-place 수정)
    ResourceHandle ReadWrite(ResourceHandle resource, ResourceUsage usage);

    //=========================================================================
    // Side Effect 선언
    //=========================================================================

    // 이 패스는 컬링하면 안 됨 (최종 출력, 디버그 등)
    void SideEffect();

private:
    FrameGraph* _graph;
    PassNode* _pass;
};

} // namespace hs
```

### 3.5 ResourceRegistry (리소스 레지스트리)

```cpp
// Framework/Graph/ResourceRegistry.h
#pragma once
#include "ResourceNode.h"
#include <unordered_map>

namespace hs {

class ResourceRegistry {
public:
    ResourceRegistry(RHIContext* rhi);
    ~ResourceRegistry();

    // 리소스 노드 생성
    ResourceHandle CreateTextureNode(const char* name, const ResourceNode::TextureDesc& desc);
    ResourceHandle CreateBufferNode(const char* name, const ResourceNode::BufferDesc& desc);
    ResourceHandle ImportTexture(const char* name, RHITexture* texture);
    ResourceHandle ImportBuffer(const char* name, RHIBuffer* buffer);

    // 리소스 노드 접근
    ResourceNode* GetNode(ResourceHandle handle);
    const ResourceNode* GetNode(ResourceHandle handle) const;

    // Transient 리소스 할당/해제 (Execute 단계)
    void AllocateTransientResources();
    void ReleaseTransientResources();

    // 실제 RHI 리소스 얻기
    RHITexture* GetTexture(ResourceHandle handle);
    RHIBuffer* GetBuffer(ResourceHandle handle);

private:
    RHIContext* _rhi;
    std::vector<Scoped<ResourceNode>> _nodes;

    // Transient 리소스 풀 (메모리 재사용)
    struct TransientPool {
        std::vector<RHITexture*> textures;
        std::vector<RHIBuffer*> buffers;
    } _pool;
};

} // namespace hs
```

---

## 4. 사용 예시

### 4.1 기본 사용법

```cpp
// Samples/DeferredPBR/main.cpp
#include "Framework/App.h"
#include "Framework/Graph/FrameGraph.h"

int main() {
    hs::App app({ .title = "Deferred PBR", .width = 1920, .height = 1080 });
    auto* rhi = app.GetRHI();

    // 셰이더, 파이프라인 준비...

    app.Run(
        [](float dt) { /* update */ },
        [&](RHICommandBuffer* cmd) {
            hs::FrameGraph fg(rhi);

            // 스왑체인 임포트
            auto backbuffer = fg.Import("Backbuffer", app.GetSwapchain()->GetCurrentTexture());

            // GBuffer Pass
            ResourceHandle gPosition, gNormal, gAlbedo, depth;
            fg.AddPass("GBuffer",
                [&](hs::PassBuilder& builder) {
                    gPosition = builder.CreateTexture("GPosition", 1.0f, 1.0f, EPixelFormat::RGBA16F);
                    gNormal   = builder.CreateTexture("GNormal", 1.0f, 1.0f, EPixelFormat::RGBA16F);
                    gAlbedo   = builder.CreateTexture("GAlbedo", 1.0f, 1.0f, EPixelFormat::RGBA8);
                    depth     = builder.CreateTexture("Depth", 1.0f, 1.0f, EPixelFormat::DEPTH32);

                    gPosition = builder.WriteRenderTarget(gPosition);
                    gNormal   = builder.WriteRenderTarget(gNormal);
                    gAlbedo   = builder.WriteRenderTarget(gAlbedo);
                    depth     = builder.WriteDepthStencil(depth);
                },
                [&](hs::PassContext& ctx) {
                    ctx.SetRenderTarget({gPosition, gNormal, gAlbedo}, depth);
                    ctx.cmd->BindPipeline(gbufferPipeline);
                    DrawScene(ctx.cmd);
                }
            );

            // SSAO Pass
            ResourceHandle ssao;
            fg.AddPass("SSAO",
                [&](hs::PassBuilder& builder) {
                    builder.Read(gPosition, ResourceUsage::ShaderRead);
                    builder.Read(gNormal, ResourceUsage::ShaderRead);
                    ssao = builder.CreateTexture("SSAO", 1.0f, 1.0f, EPixelFormat::R8);
                    ssao = builder.WriteRenderTarget(ssao);
                },
                [&](hs::PassContext& ctx) {
                    ctx.SetRenderTarget(ssao);
                    ctx.BindTexture(0, gPosition);
                    ctx.BindTexture(1, gNormal);
                    ctx.cmd->BindPipeline(ssaoPipeline);
                    ctx.cmd->DrawFullscreenQuad();
                }
            );

            // Lighting Pass (최종 출력)
            fg.AddPass("Lighting",
                [&](hs::PassBuilder& builder) {
                    builder.Read(gPosition);
                    builder.Read(gNormal);
                    builder.Read(gAlbedo);
                    builder.Read(ssao);
                    builder.WriteRenderTarget(backbuffer);
                    builder.SideEffect();  // 최종 출력이므로 컬링 방지
                },
                [&](hs::PassContext& ctx) {
                    ctx.SetRenderTarget(backbuffer);
                    ctx.BindTexture(0, gPosition);
                    ctx.BindTexture(1, gNormal);
                    ctx.BindTexture(2, gAlbedo);
                    ctx.BindTexture(3, ssao);
                    ctx.cmd->BindPipeline(lightingPipeline);
                    ctx.cmd->DrawFullscreenQuad();
                }
            );

            fg.Compile();
            fg.Execute(cmd);
        }
    );

    return 0;
}
```

### 4.2 조건부 패스

```cpp
// 조건에 따라 패스 활성화/비활성화
bool enableSSAO = true;
bool enableBloom = true;

fg.AddPass("SSAO", [&](PassBuilder& builder) {
    if (!enableSSAO) {
        builder.Skip();  // 이 패스와 의존 패스들 컬링
        return;
    }
    // ...
});

fg.AddPass("Bloom", [&](PassBuilder& builder) {
    if (!enableBloom) {
        builder.Skip();
        return;
    }
    // ...
});
```

### 4.3 컴퓨트 패스

```cpp
// 컴퓨트 셰이더 패스
ResourceHandle particleBuffer;
fg.AddPass("ParticleSimulation",
    [&](PassBuilder& builder) {
        particleBuffer = builder.CreateBuffer("Particles", sizeof(Particle) * MAX_PARTICLES,
                                               EBufferUsage::STORAGE_BUFFER);
        particleBuffer = builder.ReadWrite(particleBuffer, ResourceUsage::ShaderWrite);
    },
    [&](PassContext& ctx) {
        ctx.cmd->BindComputePipeline(particleSimPipeline);
        ctx.cmd->BindBuffer(0, particleBuffer);
        ctx.cmd->Dispatch(MAX_PARTICLES / 256, 1, 1);
    }
);
```

---

## 5. Compile 단계 상세

### 5.1 의존성 그래프 구축

```cpp
void FrameGraph::BuildDependencyGraph() {
    // 각 리소스를 마지막으로 쓴 패스 추적
    std::unordered_map<uint32_t, PassNode*> lastWriter;

    for (auto& pass : _passNodes) {
        // 읽기 의존성: 해당 리소스를 마지막으로 쓴 패스에 의존
        for (auto& read : pass->reads) {
            if (auto* writer = lastWriter[read.handle.index]) {
                pass->dependsOn.push_back(writer);
                writer->dependents.push_back(pass.get());
            }
        }

        // 쓰기: 이 패스가 해당 리소스의 마지막 writer가 됨
        for (auto& write : pass->writes) {
            lastWriter[write.handle.index] = pass.get();
        }
    }
}
```

### 5.2 패스 컬링 (역방향 전파)

```cpp
void FrameGraph::CullUnusedPasses() {
    // 1. 모든 패스를 일단 컬링 대상으로
    for (auto& pass : _passNodes) {
        pass->isCulled = true;
    }

    // 2. SideEffect가 있는 패스들을 시작점으로
    std::queue<PassNode*> queue;
    for (auto& pass : _passNodes) {
        if (pass->hasSideEffect) {
            pass->isCulled = false;
            queue.push(pass.get());
        }
    }

    // 3. 역방향으로 의존성 따라가며 필요한 패스 마킹
    while (!queue.empty()) {
        PassNode* pass = queue.front();
        queue.pop();

        for (PassNode* dependency : pass->dependsOn) {
            if (dependency->isCulled) {
                dependency->isCulled = false;
                queue.push(dependency);
            }
        }
    }

    // 결과: SideEffect 패스에 기여하지 않는 패스들은 isCulled = true
}
```

### 5.3 리소스 생명주기 계산

```cpp
void FrameGraph::CalculateResourceLifetimes() {
    for (uint32_t passIdx = 0; passIdx < _sortedPasses.size(); ++passIdx) {
        PassNode* pass = _sortedPasses[passIdx];
        if (pass->isCulled) continue;

        // 이 패스가 사용하는 모든 리소스의 생명주기 갱신
        auto updateLifetime = [&](ResourceHandle handle) {
            ResourceNode* node = _registry.GetNode(handle);
            node->firstUse = std::min(node->firstUse, passIdx);
            node->lastUse = std::max(node->lastUse, passIdx);
        };

        for (auto& read : pass->reads) updateLifetime(read.handle);
        for (auto& write : pass->writes) updateLifetime(write.handle);
    }
}
```

### 5.4 메모리 Aliasing

```cpp
void FrameGraph::ComputeMemoryAliasing() {
    // 생명주기가 겹치지 않는 리소스들끼리 같은 메모리 공유 가능
    //
    // 예: 리소스 A (패스 0~2), 리소스 B (패스 4~6)
    //     → 같은 메모리 슬롯 사용 가능
    //
    // 구현: 그리디 알고리즘 또는 그래프 컬러링

    std::vector<MemorySlot> slots;

    for (auto& node : _resourceNodes) {
        if (node->isImported || !node->isTransient) continue;

        // 기존 슬롯 중 재사용 가능한 것 찾기
        MemorySlot* availableSlot = nullptr;
        for (auto& slot : slots) {
            if (slot.lastUse < node->firstUse &&
                slot.IsCompatible(node->textureDesc)) {
                availableSlot = &slot;
                break;
            }
        }

        if (availableSlot) {
            node->aliasedSlot = availableSlot;
            availableSlot->lastUse = node->lastUse;
        } else {
            slots.push_back(CreateNewSlot(node.get()));
        }
    }
}
```

### 5.5 배리어 삽입 (Vulkan/Metal 동기화)

```cpp
void FrameGraph::InsertBarriers() {
    // 리소스별 현재 상태 추적
    std::unordered_map<uint32_t, ResourceUsage> currentState;

    for (PassNode* pass : _sortedPasses) {
        if (pass->isCulled) continue;

        for (auto& read : pass->reads) {
            ResourceUsage prev = currentState[read.handle.index];
            ResourceUsage next = read.usage;

            if (NeedsBarrier(prev, next)) {
                pass->barriers.push_back({read.handle, prev, next});
            }
            currentState[read.handle.index] = next;
        }

        for (auto& write : pass->writes) {
            ResourceUsage prev = currentState[write.handle.index];
            ResourceUsage next = write.usage;

            if (NeedsBarrier(prev, next)) {
                pass->barriers.push_back({write.handle, prev, next});
            }
            currentState[write.handle.index] = next;
        }
    }
}

bool NeedsBarrier(ResourceUsage from, ResourceUsage to) {
    // Write → Read: 배리어 필요
    // Write → Write: 배리어 필요
    // Read → Read: 배리어 불필요 (대부분)
    if (from & (ResourceUsage::ShaderWrite | ResourceUsage::ColorTarget | ResourceUsage::DepthTarget)) {
        return true;
    }
    return false;
}
```

### 5.6 위상 정렬 (Topological Sort)

```cpp
void FrameGraph::TopologicalSort() {
    // Kahn's algorithm
    std::vector<uint32_t> inDegree(_passNodes.size(), 0);

    for (auto& pass : _passNodes) {
        inDegree[pass->index] = pass->dependsOn.size();
    }

    std::queue<PassNode*> queue;
    for (auto& pass : _passNodes) {
        if (!pass->isCulled && inDegree[pass->index] == 0) {
            queue.push(pass.get());
        }
    }

    _sortedPasses.clear();
    while (!queue.empty()) {
        PassNode* pass = queue.front();
        queue.pop();

        pass->sortedIndex = _sortedPasses.size();
        _sortedPasses.push_back(pass);

        for (PassNode* dependent : pass->dependents) {
            if (--inDegree[dependent->index] == 0 && !dependent->isCulled) {
                queue.push(dependent);
            }
        }
    }
}
```

---

## 6. Execute 단계 상세

```cpp
void FrameGraph::Execute(RHICommandBuffer* cmd) {
    HS_ASSERT(_isCompiled, "FrameGraph must be compiled before execution");

    // 1. Transient 리소스 할당
    _registry.AllocateTransientResources();

    // 2. 각 패스 실행
    PassContext ctx;
    ctx.cmd = cmd;
    ctx.registry = &_registry;

    for (PassNode* pass : _sortedPasses) {
        if (pass->isCulled) continue;

        // 배리어 삽입
        for (auto& barrier : pass->barriers) {
            InsertRHIBarrier(cmd, barrier);
        }

        // 패스 실행
        pass->executeFunc(ctx);
    }

    // 3. Transient 리소스 해제 (또는 풀에 반환)
    _registry.ReleaseTransientResources();
}

void InsertRHIBarrier(RHICommandBuffer* cmd, const BarrierInfo& barrier) {
    // Vulkan: vkCmdPipelineBarrier
    // Metal: MTLResourceUsage 설정 또는 memoryBarrier

    #if defined(HS_VULKAN)
        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.oldLayout = ToVkLayout(barrier.fromUsage);
        imgBarrier.newLayout = ToVkLayout(barrier.toUsage);
        // ...
        vkCmdPipelineBarrier(cmd->GetHandle(), ...);
    #elif defined(HS_METAL)
        // Metal은 대부분 자동 처리, 필요시 memoryBarrier 호출
        if (NeedsExplicitBarrier(barrier)) {
            [cmd->GetHandle() memoryBarrierWithResources:...];
        }
    #endif
}
```

---

## 7. 디버깅 및 시각화

### 7.1 Graphviz 출력

```cpp
void FrameGraph::ExportGraphviz(const char* path) {
    std::ofstream file(path);
    file << "digraph FrameGraph {\n";
    file << "  rankdir=LR;\n";

    // 패스 노드
    for (auto& pass : _passNodes) {
        const char* color = pass->isCulled ? "gray" : "lightblue";
        file << "  " << pass->name << " [shape=box, style=filled, fillcolor=" << color << "];\n";
    }

    // 리소스 노드
    for (auto& res : _resourceNodes) {
        const char* color = res->isTransient ? "lightyellow" : "lightgreen";
        file << "  " << res->name << " [shape=ellipse, style=filled, fillcolor=" << color << "];\n";
    }

    // 엣지 (패스 → 리소스, 리소스 → 패스)
    for (auto& pass : _passNodes) {
        for (auto& read : pass->reads) {
            auto* res = _registry.GetNode(read.handle);
            file << "  " << res->name << " -> " << pass->name << " [color=blue];\n";
        }
        for (auto& write : pass->writes) {
            auto* res = _registry.GetNode(write.handle);
            file << "  " << pass->name << " -> " << res->name << " [color=red];\n";
        }
    }

    file << "}\n";
}
```

출력 예시:
```
digraph FrameGraph {
  rankdir=LR;
  GBuffer [shape=box, style=filled, fillcolor=lightblue];
  SSAO [shape=box, style=filled, fillcolor=lightblue];
  Lighting [shape=box, style=filled, fillcolor=lightblue];

  GPosition [shape=ellipse, style=filled, fillcolor=lightyellow];
  GNormal [shape=ellipse, style=filled, fillcolor=lightyellow];

  GBuffer -> GPosition [color=red];
  GBuffer -> GNormal [color=red];
  GPosition -> SSAO [color=blue];
  GNormal -> SSAO [color=blue];
  ...
}
```

### 7.2 통계 출력

```cpp
void FrameGraph::PrintStats() {
    uint32_t totalPasses = _passNodes.size();
    uint32_t culledPasses = std::count_if(_passNodes.begin(), _passNodes.end(),
                                          [](auto& p) { return p->isCulled; });
    uint32_t transientResources = std::count_if(_resourceNodes.begin(), _resourceNodes.end(),
                                                 [](auto& r) { return r->isTransient; });

    HS_LOG("=== FrameGraph Stats ===");
    HS_LOG("Passes: %u total, %u executed, %u culled",
           totalPasses, totalPasses - culledPasses, culledPasses);
    HS_LOG("Resources: %u total, %u transient, %u imported",
           _resourceNodes.size(), transientResources,
           _resourceNodes.size() - transientResources);
    HS_LOG("Memory aliased slots: %u", _aliasedSlotCount);
    HS_LOG("Barriers inserted: %u", _barrierCount);
}
```

---

## 8. Metal vs Vulkan 차이점 처리

### 8.1 배리어/동기화

| 항목 | Vulkan | Metal |
|------|--------|-------|
| 이미지 레이아웃 | 명시적 전환 필요 | 자동 (대부분) |
| 파이프라인 배리어 | `vkCmdPipelineBarrier` | `memoryBarrier` (제한적) |
| 렌더패스 의존성 | Subpass dependency | Tile memory 자동 관리 |

```cpp
// RHI 레벨에서 추상화
class RHICommandBuffer {
public:
    // 공통 인터페이스
    void ResourceBarrier(RHITexture* texture, EResourceState from, EResourceState to);

    // 내부적으로:
    // - Vulkan: vkCmdPipelineBarrier 호출
    // - Metal: 필요한 경우만 memoryBarrier, 대부분 no-op
};
```

### 8.2 Transient 리소스

| 항목 | Vulkan | Metal |
|------|--------|-------|
| 메모리 할당 | VMA 또는 수동 할당 | MTLHeap 기반 할당 |
| Aliasing | 명시적 메모리 바인딩 | `makeAliasable()` |
| Tile Memory | 없음 | Memoryless 렌더 타겟 지원 |

```cpp
// Metal 최적화: Memoryless 렌더 타겟
#if defined(HS_METAL)
if (resource->isTransient && resource->OnlyUsedWithinSinglePass()) {
    // Tile memory에만 존재, VRAM 할당 안 함
    textureDesc.storageMode = MTLStorageModeMemoryless;
}
#endif
```

---

## 9. 구현 우선순위

### Phase 1: 기본 구조 (~800 LOC)
1. `FrameGraph`, `PassNode`, `ResourceNode` 기본 클래스
2. `PassBuilder` 기본 기능 (Create, Read, Write)
3. 의존성 그래프 구축 + 위상 정렬
4. 단순 Execute (배리어 없이)

**목표**: 기본 멀티패스 렌더링 동작

### Phase 2: 최적화 (~400 LOC)
5. 패스 컬링
6. 리소스 생명주기 계산
7. 기본 배리어 삽입

**목표**: 불필요한 패스/리소스 제거

### Phase 3: 고급 기능 (~300 LOC)
8. 메모리 aliasing
9. Transient 리소스 풀링
10. Graphviz 출력

**목표**: 메모리 최적화 + 디버깅

---

## 10. 예상 LOC

| 컴포넌트 | LOC |
|---------|-----|
| FrameGraph.h/cpp | 400 |
| PassNode.h/cpp | 150 |
| ResourceNode.h/cpp | 150 |
| PassBuilder.h/cpp | 200 |
| ResourceRegistry.h/cpp | 300 |
| TransientAllocator.h/cpp | 200 |
| BarrierBatcher.h/cpp | 100 |
| **합계** | **~1,500** |

전체 Framework 예상: 5,200 + 1,500 = **~6,700 LOC**

---

## 11. 사례 연구: Unreal Engine RDG

### 11.1 문제 제기

FrameGraph의 `AddPass`는 람다를 받는데, 복잡한 렌더 파이프라인을 람다 안에 다 넣으면 코드가 비대해지지 않을까?

```cpp
// 이렇게 되면 안 됨
fg.AddPass("ComplexPass", [&](PassBuilder& builder) {
    // 100줄의 Setup 코드...
}, [&](PassContext& ctx) {
    // 200줄의 Execute 코드...
});
```

### 11.2 언리얼의 해결책: 계층 분리

언리얼 RDG는 **람다 자체를 복잡하게 만들지 않고, 여러 계층으로 분리**합니다.

```
┌─────────────────────────────────────────────────┐
│         FDeferredShadingSceneRenderer           │  ← 파이프라인 구성
│  - Render()에서 전체 흐름 제어                    │
│  - 어떤 패스를 어떤 조건에서 실행할지 결정          │
└─────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────┐
│              RenderXXX() 함수들                  │  ← 기능별 그룹
│  - RenderSSAO(), RenderShadows(), ...           │
│  - 관련 패스들을 묶어서 관리                      │
└─────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────┐
│         AddPass() + Parameters 구조체            │  ← 개별 패스
│  - 파라미터는 구조체로 캡슐화                     │
│  - 람다는 드로우 콜만 담당                        │
└─────────────────────────────────────────────────┘
```

### 11.3 언리얼 코드 분석

#### A. Pass Parameters 구조체

```cpp
// 패스에 필요한 모든 데이터를 구조체로 정의
// 매크로로 리플렉션 정보 자동 생성
BEGIN_SHADER_PARAMETER_STRUCT(FSSAOPassParameters, )
    SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
    SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GBufferA)
    SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GBufferB)
    SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneDepth)
    SHADER_PARAMETER_SAMPLER(SamplerState, PointClampSampler)
    SHADER_PARAMETER(float, Radius)
    SHADER_PARAMETER(float, Bias)
    RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()
```

#### B. 람다는 단순하게 유지

```cpp
// AddPass의 람다는 실제 드로우 콜만 담당
FSSAOPassParameters* PassParameters = GraphBuilder.AllocParameters<FSSAOPassParameters>();
PassParameters->GBufferA = GBufferA;
PassParameters->GBufferB = GBufferB;
PassParameters->Radius = SSAOSettings.Radius;
// ... 파라미터 설정 ...

GraphBuilder.AddPass(
    RDG_EVENT_NAME("SSAO"),
    PassParameters,
    ERDGPassFlags::Raster,
    [PassParameters, ShaderMap](FRHICommandList& RHICmdList) {
        // 람다 내부는 단순 - 드로우만
        TShaderMapRef<FSSAOPassPS> PixelShader(ShaderMap);
        FPixelShaderUtils::DrawFullscreenQuad(RHICmdList, PixelShader, ...);
    }
);
```

#### C. Renderer 클래스에서 파이프라인 구성

```cpp
// FDeferredShadingSceneRenderer - 전체 렌더링 흐름 관리
class FDeferredShadingSceneRenderer : public FSceneRenderer
{
    void Render(FRDGBuilder& GraphBuilder)
    {
        // 전체 파이프라인을 여기서 구성
        RenderPrePass(GraphBuilder);
        RenderBasePass(GraphBuilder);      // 내부에서 여러 AddPass 호출
        RenderShadowDepths(GraphBuilder);
        RenderLights(GraphBuilder);

        if (ShouldRenderSSAO())            // 조건부 패스
        {
            RenderSSAO(GraphBuilder);
        }

        RenderTranslucency(GraphBuilder);
        RenderPostProcessing(GraphBuilder);
    }

    void RenderSSAO(FRDGBuilder& GraphBuilder)
    {
        // SSAO 관련 모든 패스들을 여기서 관리
        FRDGTextureRef SSAOTexture = AddSSAOPass(GraphBuilder);
        FRDGTextureRef BlurredSSAO = AddSSAOBlurPass(GraphBuilder, SSAOTexture);
        AddSSAOApplyPass(GraphBuilder, BlurredSSAO);
    }

    FRDGTextureRef AddSSAOPass(FRDGBuilder& GraphBuilder)
    {
        // 실제 AddPass 호출은 여기서
        FSSAOPassParameters* Params = GraphBuilder.AllocParameters<...>();
        // ...
        GraphBuilder.AddPass(...);
        return SSAOOutput;
    }
};
```

### 11.4 핵심 패턴 정리

| 패턴 | 역할 | 예시 |
|------|------|------|
| **Parameters 구조체** | 패스 입력 캡슐화 | `FSSAOPassParameters` |
| **AddXXXPass 함수** | 단일 패스 생성 로직 | `AddSSAOPass()` |
| **RenderXXX 함수** | 관련 패스 그룹 관리 | `RenderSSAO()` |
| **Renderer 클래스** | 전체 파이프라인 구성 | `FDeferredShadingSceneRenderer` |

### 11.5 HSMR 적용 설계

#### A. Pass Parameters 구조체

```cpp
// Framework/Graph/PassParams.h

// SSAO 패스 파라미터
struct SSAOPassParams {
    ResourceHandle gPosition;
    ResourceHandle gNormal;
    ResourceHandle depthBuffer;
    ResourceHandle noiseTexture;
    ResourceHandle output;

    float radius = 0.5f;
    float bias = 0.025f;
    int kernelSize = 64;
};

// GBuffer 패스 파라미터
struct GBufferPassParams {
    // 입력
    const Scene* scene;
    const Camera* camera;

    // 출력
    ResourceHandle position;
    ResourceHandle normal;
    ResourceHandle albedo;
    ResourceHandle depth;
};

// Lighting 패스 파라미터
struct LightingPassParams {
    ResourceHandle gPosition;
    ResourceHandle gNormal;
    ResourceHandle gAlbedo;
    ResourceHandle ssao;          // Optional
    ResourceHandle shadowMap;     // Optional
    ResourceHandle output;

    std::vector<Light*> lights;
};
```

#### B. 패스 추가 헬퍼 함수

```cpp
// Framework/Graph/CommonPasses.h

namespace hs::passes {

// GBuffer 패스 추가 - 여러 리소스를 한번에 반환
struct GBufferOutput {
    ResourceHandle position;
    ResourceHandle normal;
    ResourceHandle albedo;
    ResourceHandle depth;
};

GBufferOutput AddGBufferPass(
    FrameGraph& fg,
    const GBufferPassParams& params,
    RHIGraphicsPipeline* pipeline)
{
    GBufferOutput output;

    fg.AddPass("GBuffer",
        [&](PassBuilder& builder) {
            output.position = builder.CreateTexture("GPosition", 1.0f, 1.0f, RGBA16F);
            output.normal   = builder.CreateTexture("GNormal", 1.0f, 1.0f, RGBA16F);
            output.albedo   = builder.CreateTexture("GAlbedo", 1.0f, 1.0f, RGBA8);
            output.depth    = builder.CreateTexture("Depth", 1.0f, 1.0f, DEPTH32);

            output.position = builder.WriteRenderTarget(output.position);
            output.normal   = builder.WriteRenderTarget(output.normal);
            output.albedo   = builder.WriteRenderTarget(output.albedo);
            output.depth    = builder.WriteDepthStencil(output.depth);
        },
        [params, pipeline](PassContext& ctx) {
            ctx.cmd->BindPipeline(pipeline);
            params.scene->Draw(ctx.cmd, params.camera);
        }
    );

    return output;
}

// SSAO 패스 추가
ResourceHandle AddSSAOPass(
    FrameGraph& fg,
    const SSAOPassParams& params,
    RHIGraphicsPipeline* pipeline)
{
    ResourceHandle output;

    fg.AddPass("SSAO",
        [&](PassBuilder& builder) {
            builder.Read(params.gPosition);
            builder.Read(params.gNormal);
            builder.Read(params.noiseTexture);

            output = builder.CreateTexture("SSAO", 1.0f, 1.0f, R8);
            output = builder.WriteRenderTarget(output);
        },
        [params, pipeline](PassContext& ctx) {
            ctx.BindTexture(0, params.gPosition);
            ctx.BindTexture(1, params.gNormal);
            ctx.BindTexture(2, params.noiseTexture);
            ctx.cmd->BindPipeline(pipeline);
            ctx.cmd->SetUniform("radius", params.radius);
            ctx.cmd->SetUniform("bias", params.bias);
            ctx.cmd->DrawFullscreenQuad();
        }
    );

    return output;
}

// Blur 패스 (재사용 가능)
ResourceHandle AddBlurPass(
    FrameGraph& fg,
    const char* name,
    ResourceHandle input,
    bool horizontal,
    RHIGraphicsPipeline* pipeline)
{
    ResourceHandle output;

    fg.AddPass(name,
        [&](PassBuilder& builder) {
            builder.Read(input);
            output = builder.CreateTexture("BlurOutput", 1.0f, 1.0f, R8);
            output = builder.WriteRenderTarget(output);
        },
        [input, horizontal, pipeline](PassContext& ctx) {
            ctx.BindTexture(0, input);
            ctx.cmd->BindPipeline(pipeline);
            ctx.cmd->SetUniform("horizontal", horizontal ? 1 : 0);
            ctx.cmd->DrawFullscreenQuad();
        }
    );

    return output;
}

} // namespace hs::passes
```

#### C. Renderer 클래스

```cpp
// Sample 또는 Framework에서 정의
class DeferredRenderer {
public:
    DeferredRenderer(RHIContext* rhi);

    void Initialize();  // 파이프라인, 셰이더 생성
    void Shutdown();

    // 설정
    struct Settings {
        bool enableSSAO = true;
        bool enableBloom = true;
        bool enableShadows = true;
        float ssaoRadius = 0.5f;
        float ssaoBias = 0.025f;
    };
    Settings settings;

    // 메인 렌더 함수
    void Render(FrameGraph& fg, const Scene* scene, const Camera* camera, ResourceHandle output);

private:
    // 기능별 렌더 함수
    GBufferOutput RenderGBuffer(FrameGraph& fg, const Scene* scene, const Camera* camera);
    ResourceHandle RenderSSAO(FrameGraph& fg, const GBufferOutput& gbuffer);
    ResourceHandle RenderShadows(FrameGraph& fg, const Scene* scene);
    void RenderLighting(FrameGraph& fg, const GBufferOutput& gbuffer,
                        ResourceHandle ssao, ResourceHandle shadowMap, ResourceHandle output);
    void RenderPostProcess(FrameGraph& fg, ResourceHandle input, ResourceHandle output);

    // 리소스
    RHIContext* _rhi;

    // 파이프라인들
    RHIGraphicsPipeline* _gbufferPipeline;
    RHIGraphicsPipeline* _ssaoPipeline;
    RHIGraphicsPipeline* _blurPipeline;
    RHIGraphicsPipeline* _lightingPipeline;

    // 공유 리소스
    ResourceHandle _noiseTexture;
    Ref<RHIBuffer> _ssaoKernel;
};
```

#### D. Renderer 구현

```cpp
// DeferredRenderer.cpp

void DeferredRenderer::Render(
    FrameGraph& fg,
    const Scene* scene,
    const Camera* camera,
    ResourceHandle output)
{
    // 1. GBuffer
    GBufferOutput gbuffer = RenderGBuffer(fg, scene, camera);

    // 2. SSAO (조건부)
    ResourceHandle ssao;
    if (settings.enableSSAO) {
        ssao = RenderSSAO(fg, gbuffer);
    }

    // 3. Shadows (조건부)
    ResourceHandle shadowMap;
    if (settings.enableShadows) {
        shadowMap = RenderShadows(fg, scene);
    }

    // 4. Lighting
    ResourceHandle lit;
    RenderLighting(fg, gbuffer, ssao, shadowMap, lit);

    // 5. Post-processing (조건부)
    if (settings.enableBloom) {
        RenderPostProcess(fg, lit, output);
    } else {
        // Blit to output
        AddBlitPass(fg, lit, output);
    }
}

GBufferOutput DeferredRenderer::RenderGBuffer(
    FrameGraph& fg,
    const Scene* scene,
    const Camera* camera)
{
    return hs::passes::AddGBufferPass(fg, {
        .scene = scene,
        .camera = camera
    }, _gbufferPipeline);
}

ResourceHandle DeferredRenderer::RenderSSAO(
    FrameGraph& fg,
    const GBufferOutput& gbuffer)
{
    // SSAO 메인 패스
    ResourceHandle ssaoRaw = hs::passes::AddSSAOPass(fg, {
        .gPosition = gbuffer.position,
        .gNormal = gbuffer.normal,
        .noiseTexture = _noiseTexture,
        .radius = settings.ssaoRadius,
        .bias = settings.ssaoBias
    }, _ssaoPipeline);

    // Blur (horizontal + vertical)
    ResourceHandle blurH = hs::passes::AddBlurPass(fg, "SSAO_BlurH", ssaoRaw, true, _blurPipeline);
    ResourceHandle blurV = hs::passes::AddBlurPass(fg, "SSAO_BlurV", blurH, false, _blurPipeline);

    return blurV;
}
```

#### E. Sample에서 사용

```cpp
// Samples/DeferredPBR/main.cpp
int main() {
    hs::App app({ .title = "Deferred PBR", .width = 1920, .height = 1080 });

    // 렌더러 초기화
    DeferredRenderer renderer(app.GetRHI());
    renderer.Initialize();

    // 씬 로드
    Scene scene;
    scene.Load("Scenes/Sponza.scene");

    Camera camera;

    app.Run(
        [&](float dt) {
            camera.Update(app, dt);

            // ImGui로 설정 조정
            ImGui::Begin("Render Settings");
            ImGui::Checkbox("SSAO", &renderer.settings.enableSSAO);
            ImGui::Checkbox("Bloom", &renderer.settings.enableBloom);
            ImGui::SliderFloat("SSAO Radius", &renderer.settings.ssaoRadius, 0.1f, 2.0f);
            ImGui::End();
        },
        [&](RHICommandBuffer* cmd) {
            hs::FrameGraph fg(app.GetRHI());

            auto backbuffer = fg.Import("Backbuffer", app.GetSwapchain()->GetTexture());

            // 한 줄로 전체 파이프라인 실행
            renderer.Render(fg, &scene, &camera, backbuffer);

            fg.Compile();
            fg.Execute(cmd);
        }
    );

    renderer.Shutdown();
    return 0;
}
```

### 11.6 정리: 복잡성 관리 전략

| 계층 | 역할 | 복잡도 위치 |
|------|------|------------|
| **Sample main.cpp** | 앱 설정, 렌더러 호출 | 최소 (~50 LOC) |
| **Renderer 클래스** | 파이프라인 흐름 제어, 조건부 패스 | 중간 (~200 LOC) |
| **RenderXXX 함수** | 관련 패스 그룹화 | 중간 (~50 LOC each) |
| **AddXXXPass 함수** | 단일 패스 정의 | 낮음 (~30 LOC each) |
| **PassParams 구조체** | 패스 입력 캡슐화 | 최소 (~10 LOC each) |
| **람다** | 실제 드로우 콜만 | 최소 (~5-10 LOC) |

**핵심**: 람다가 복잡해지는 게 아니라, **복잡성을 상위 계층(Renderer 클래스)으로 밀어올림**.

---

*이 문서는 경량화된 HSMR Framework 위에서 동작하는 FrameGraph 시스템의 설계 제안입니다.*
