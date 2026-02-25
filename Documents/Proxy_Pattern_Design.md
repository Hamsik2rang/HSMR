# HSMR Proxy 패턴 도입 설계 + 언리얼 엔진 코드 참조 가이드

## Context

현재 `Renderer::Render()` 내부에서 CPU 원본 객체(Model*, Camera*, Material*)를 직접 참조하여 SceneResource를 빌드한다. 이 구조에서는 Game Thread가 Render 완료까지 블로킹되며, 스레드 분리 시 data race가 발생한다.

이 문서는:
1. 언리얼 엔진의 Proxy 아키텍처를 **실제 코드 위치와 함께** 정리하여 학습 가이드를 제공
2. HSMR에 맞는 Proxy 패턴 도입 방향을 구체적으로 설계

---

## Part A: 언리얼 엔진 Proxy 아키텍처 학습 가이드

### A.1 핵심 클래스 맵

```
[Game Thread 소유]                              [Render Thread 소유]
──────────────────                              ────────────────────
UPrimitiveComponent                             FPrimitiveSceneProxy
├─ UStaticMeshComponent                         ├─ FStaticMeshSceneProxy
├─ USkeletalMeshComponent                       ├─ FSkeletalMeshSceneProxy
└─ UCustomMeshComponent (사용자 정의)             └─ FCustomSceneProxy

UWorld → FScene (양쪽 접근, but 내부적으로 스레드 구분)
```

### A.2 읽어야 할 핵심 파일들 (UE5 소스)

> UE5 소스 접근: https://github.com/EpicGames/UnrealEngine (Epic Games 계정 필요)

| 파일 경로 | 핵심 내용 | 우선순위 |
|:----------|:---------|:---------|
| `Engine/Source/Runtime/Engine/Public/PrimitiveSceneProxy.h` | FPrimitiveSceneProxy 기본 클래스 — 모든 Proxy의 부모 | ★★★ |
| `Engine/Source/Runtime/Engine/Public/StaticMeshSceneProxy.h` | FStaticMeshSceneProxy — 가장 대표적인 구현 | ★★★ |
| `Engine/Source/Runtime/Engine/Private/StaticMeshRender.cpp` | GetDynamicMeshElements(), DrawStaticElements() 구현 | ★★★ |
| `Engine/Source/Runtime/Engine/Private/PrimitiveSceneProxy.cpp` | 기본 Proxy 생성자에서 어떤 데이터를 복사하는지 | ★★★ |
| `Engine/Source/Runtime/Renderer/Private/SceneRendering.h` | FSceneRenderer — 프레임 단위 렌더러 | ★★☆ |
| `Engine/Source/Runtime/Renderer/Private/Scene.h` | FScene — Primitive 등록/해제/관리 | ★★☆ |
| `Engine/Source/Runtime/Renderer/Private/Scene.cpp` | AddPrimitive(), RemovePrimitive() 구현 | ★★☆ |
| `Engine/Source/Runtime/RHI/Public/RenderThread.h` | ENQUEUE_RENDER_COMMAND 매크로 정의 | ★★☆ |
| `Engine/Source/Runtime/Renderer/Private/MeshDrawCommands.h` | FMeshDrawCommand — 최종 GPU 드로우 커맨드 | ★☆☆ |

### A.3 FPrimitiveSceneProxy 생성자 — 데이터 복사 패턴

**파일:** `PrimitiveSceneProxy.h` / `PrimitiveSceneProxy.cpp`

```cpp
// 생성자: Game Thread에서 호출, UPrimitiveComponent의 데이터를 "복사"
FPrimitiveSceneProxy::FPrimitiveSceneProxy(const UPrimitiveComponent* InComponent)
    // 값 복사 (참조 X)
    : LocalToWorld(InComponent->GetRenderMatrix())           // ← Transform 복사
    , Bounds(InComponent->Bounds)                             // ← 바운딩 복사
    , ActorPosition(InComponent->GetActorPositionForRenderer())
    , bCastShadow(InComponent->CastShadow)                   // ← 플래그 복사
    , bReceivesDecals(InComponent->bReceivesDecals)
    , bOnlyOwnerSee(InComponent->bOnlyOwnerSee)
    // ...
{
    // Component 포인터는 저장하지 않음! (또는 저장해도 Render Thread에서 접근 금지)
}
```

**핵심 포인트:**
- 생성자에서 Component의 데이터를 **값으로 복사**
- 생성 이후에는 원본 Component에 접근하지 않음
- Transform이 변경되면 별도 메커니즘(`SendRenderTransform_Concurrent`)으로 전달

### A.4 Proxy 생명주기 전체 흐름

**파일:** `Components/UPrimitiveComponent.h`, `Scene.cpp`

```
Phase 1: 등록 (Game Thread)
─────────────────────────────────
UPrimitiveComponent::RegisterComponent()
  → CreateRenderState_Concurrent()
    → CreateSceneProxy()           ← 파생 클래스가 오버라이드
      → return new FStaticMeshSceneProxy(this);
    → GetWorld()->Scene->AddPrimitive(this)
      → ENQUEUE_RENDER_COMMAND() {
           FScene::AddPrimitive_RenderThread(proxy);  ← Render Thread에서 실행
         }

Phase 2: 초기화 (Render Thread)
─────────────────────────────────
FScene::AddPrimitive_RenderThread()
  → FPrimitiveSceneInfo 생성 (Proxy의 렌더러 내부 래퍼)
  → proxy->CreateRenderThreadResources()       ← GPU 리소스 생성
  → proxy->DrawStaticElements(Collector)       ← Static Mesh → FMeshBatch 캐시
  → FPrimitiveSceneInfo::CacheMeshDrawCommands()  ← FMeshBatch → FMeshDrawCommand 변환

Phase 3: 매 프레임 Transform 업데이트 (Game → Render)
─────────────────────────────────────────────────────
Game Thread:
  UPrimitiveComponent::SetWorldLocation(newPos)
    → MarkRenderTransformDirty()

프레임 끝:
  SendRenderTransform_Concurrent()
    → ENQUEUE_RENDER_COMMAND(UpdateTransform) {
         proxy->SetTransform(newLocalToWorld);    ← 값 복사
         FScene::UpdatePrimitiveTransform()       ← Octree 갱신
       }

Phase 4: 머티리얼/메시 변경 (무거운 경로)
───────────────────────────────────────────
Game Thread:
  UPrimitiveComponent::MarkRenderStateDirty()
    → RecreateRenderState_Concurrent()
      → ENQUEUE_RENDER_COMMAND {
           FScene::RemovePrimitive(oldProxy);    ← 기존 Proxy 제거
           FScene::AddPrimitive(newProxy);       ← 새 Proxy 생성/등록
         }

Phase 5: 소멸 (Game → Render)
─────────────────────────────
Game Thread:
  UPrimitiveComponent::DestroyRenderState_Concurrent()
    → ENQUEUE_RENDER_COMMAND {
         FScene::RemovePrimitive(proxy);
         DeferredDelete(proxy);     ← N프레임 후 삭제 (GPU 완료 보장)
       }
```

### A.5 ENQUEUE_RENDER_COMMAND — 스레드 간 통신

**파일:** `Engine/Source/Runtime/RHI/Public/RenderThread.h`

```cpp
// Game Thread에서 호출
ENQUEUE_RENDER_COMMAND(UpdateMyProxy)(
    [NewTransform = Component->GetTransform(),     // ← 값 복사 (= 캡처)
     ProxyPtr = SceneProxy]                         // ← Proxy 포인터는 Render Thread 소유
    (FRHICommandListImmediate& RHICmdList)
    {
        // 이 람다는 Render Thread에서 실행됨
        ProxyPtr->UpdateTransform(NewTransform);
    }
);
```

**핵심:**
- 람다 캡처는 **반드시 값 복사** (참조 캡처 시 Game Thread 데이터가 변경될 수 있음)
- 대용량 데이터는 `TSharedPtr` 또는 별도 할당 후 포인터 캡처
- Render Thread는 큐에서 순서대로 커맨드를 꺼내 실행

### A.6 FMeshBatch → FMeshDrawCommand 흐름

**파일:** `MeshDrawCommands.h`, `SceneRendering.cpp`

```
FPrimitiveSceneProxy::DrawStaticElements(Collector)
  │
  └─ Collector.AddMesh(FMeshBatch)     ← Proxy가 FMeshBatch를 생성
       │
       └─ FMeshBatch {
            VertexFactory*             ← 정점 해석 방법
            MaterialRenderProxy*       ← 머티리얼 (UMaterial이 아닌 렌더 전용 Proxy)
            Elements[0] {
              IndexBuffer*             ← 인덱스 버퍼
              FirstIndex, NumPrimitives
              MinVertexIndex, MaxVertexIndex
            }
          }
              │
              ▼
FMeshPassProcessor::AddMeshBatch(FMeshBatch)
  │
  └─ FMeshDrawCommand {              ← GPU 드로우 커맨드로 변환 (사전 캐시됨)
       ShaderBindings               ← Shader 파라미터 바인딩
       VertexStreams                 ← VB 바인딩
       IndexBuffer                  ← IB
       CachedPipelineId             ← PSO (Pipeline State Object) 캐시 ID
       DrawPrimitiveType            ← Indexed, etc.
     }
```

**Static vs Dynamic:**
| 경로 | 빌드 시점 | 수명 | 예시 |
|:-----|:---------|:-----|:-----|
| Static (캐시) | `AddPrimitive()` 시 1회 | Proxy 수명 동안 유지 | StaticMesh, Landscape |
| Dynamic (매 프레임) | `GetDynamicMeshElements()` | 1프레임 | SkeletalMesh, Particle |

### A.7 학습 순서 추천

```
1단계: Proxy 기본 구조
  → PrimitiveSceneProxy.h 읽기 (생성자, 가상함수 목록)
  → StaticMeshSceneProxy.h 읽기 (구체적 구현)

2단계: 등록/해제 흐름
  → UPrimitiveComponent::CreateSceneProxy() 읽기
  → FScene::AddPrimitive() / RemovePrimitive() 읽기

3단계: 데이터 전달
  → ENQUEUE_RENDER_COMMAND 사용 예시 검색 (UE5 소스에서 grep)
  → SendRenderTransform_Concurrent() 구현 읽기

4단계: 드로우 커맨드
  → DrawStaticElements() / GetDynamicMeshElements() 구현 읽기
  → FMeshBatch 구조 이해
```

---

## Part B: HSMR Proxy 도입 설계

### B.1 현재 렌더링 경로에서 CPU 객체 접근 맵

`BuildSceneResource()` + `_updateSceneBuffers()`에서 접근하는 모든 데이터:

| CPU 객체 | 접근 메서드 | 추출 데이터 | 접근 빈도 |
|:---------|:-----------|:-----------|:---------|
| **Model** | `GetWorldMatrix()` | mat4 | 매 프레임 |
| | `GetInverseWorldMatrix()` | mat4 | 매 프레임 |
| | `GetMaterial()` | Material* | 매 프레임 (BuildSceneResource) |
| | `GetMesh()` | Mesh* | 매 프레임 (BuildSceneResource) |
| | `Update()` | void | 매 프레임 |
| **Camera** | `GetViewMatrix()` 외 6개 매트릭스 | mat4 × 6 | 매 프레임 |
| | `GetPosition()` | vec3 | 매 프레임 |
| | `Update()` | void | 매 프레임 |
| **Material** | `GetShader()` | Shader* | 리소스 생성 시 (캐시) |
| | `GetTexture(type)` | Image* | 리소스 생성 시 (캐시) |
| | `IsTwoSided()` | bool | 파이프라인 생성 시 (캐시) |
| **Mesh** | `GetPosition/Normal/TexCoord/...()` | vector<float> | 리소스 생성 시 (캐시, 1회) |
| | `GetIndices()` | vector<uint32> | 리소스 생성 시 (캐시, 1회) |
| **Image** | `GetRawData()` | uint8* | 리소스 생성 시 (캐시, 1회) |
| | `GetWidth/Height/Channel()` | uint16/uint8 | 리소스 생성 시 (캐시, 1회) |
| **Shader** | `GetReflection()` | ShaderReflectionDataEx | 리소스 생성 시 (캐시) |
| | `GetBytecode(stage)` | vector<uint8>* | 리소스 생성 시 (캐시, 1회) |

### B.2 데이터 분류: Proxy에 담아야 할 것 vs GPU 캐시에 남길 것

```
┌─ 매 프레임 갱신 데이터 (Proxy 스냅샷 필수) ──────────────────────┐
│                                                                  │
│  RenderProxy {                                                   │
│    // Model 스냅샷                                                │
│    glm::mat4  worldMatrix;                                       │
│    glm::mat4  inverseWorldMatrix;                                │
│    Material*  material;    // → MaterialProxy ID로 대체 가능      │
│    Mesh*      mesh;        // → MeshProxy ID로 대체 가능          │
│  }                                                               │
│                                                                  │
│  CameraSnapshot {                                                │
│    glm::mat4  viewMatrix, projectionMatrix, ... (6개)            │
│    glm::vec3  position;                                          │
│  }                                                               │
└──────────────────────────────────────────────────────────────────┘

┌─ 등록 시 1회 생성, 변경 시 재생성 (GPU 캐시 유지) ──────────────┐
│                                                                  │
│  MaterialResource (기존 유지)                                     │
│    RHIShader*, RHIResourceLayout*, RHIResourceSet*               │
│    → 변경 시: MarkMaterialDirty() → 재생성                       │
│                                                                  │
│  MeshResource (기존 유지)                                         │
│    RHIBuffer* vertexBuffer, indexBuffer                           │
│    → Mesh 데이터는 GPU 업로드 후 CPU 접근 불필요                  │
│                                                                  │
│  ImageResource (기존 유지)                                        │
│    RHITexture*, RHISampler*                                      │
│    → Image 픽셀 데이터는 GPU 업로드 후 CPU 접근 불필요            │
└──────────────────────────────────────────────────────────────────┘
```

### B.3 제안 아키텍처: 3단계 점진적 도입

현재 HSMR은 단일 스레드이므로, 실제 멀티스레드 도입 전에 **구조만 먼저 분리**하는 것이 안전하다.

#### 1단계: FrameSnapshot 도입 (스레드 분리 없이 구조만)

**목표:** `Renderer::Render()`가 CPU 원본 객체를 직접 접근하지 않도록 만든다.

```
변경 전:
  Game Logic → Renderer::Render(models, cameras, scene, ...)
                 └─ model->GetMaterial()  ← 원본 직접 접근

변경 후:
  Game Logic → FrameSnapshot 구축 (값 복사)
             → Renderer::Render(frameSnapshot)
                 └─ snapshot.renderables[i].worldMatrix  ← 복사본 접근
```

**새로운 구조체:**

```cpp
// 매 프레임 Game Thread에서 구축, Renderer에 전달
struct RenderableSnapshot
{
    uint32      id;                 // 고유 식별자 (포인터 대신)
    glm::mat4   worldMatrix;
    glm::mat4   inverseWorldMatrix;
    uint32      materialId;         // Material 식별
    uint32      meshId;             // Mesh 식별
};

struct CameraSnapshot
{
    glm::mat4   viewMatrix;
    glm::mat4   projectionMatrix;
    glm::mat4   viewProjectionMatrix;
    glm::mat4   inverseViewMatrix;
    glm::mat4   inverseProjectionMatrix;
    glm::mat4   inverseViewProjectionMatrix;
    glm::vec4   position;
};

struct FrameSnapshot
{
    std::vector<RenderableSnapshot>  renderables;
    std::vector<CameraSnapshot>      cameras;
    // (미래) std::vector<LightSnapshot> lights;
};
```

**대응하는 언리얼 개념:**
- `RenderableSnapshot` ≈ FPrimitiveSceneProxy의 생성자에서 복사하는 Transform 데이터
- `CameraSnapshot` ≈ FSceneView가 Game Thread에서 구축되어 Render Thread로 전달되는 구조
- `FrameSnapshot` ≈ FSceneRenderer에 전달되는 FSceneViewFamily

#### 2단계: RenderScene 도입 (이벤트 기반 등록/해제)

**목표:** 매 프레임 전체 재조립 대신, 등록/해제/변경 이벤트로 렌더러의 내부 리스트를 점진 갱신한다.

```
변경 전:
  매 프레임: BuildSceneResource(allModels)  ← 전체 순회

변경 후:
  등록 시:   RenderScene::AddRenderable(id, materialId, meshId)  ← GPU 리소스 생성
  삭제 시:   RenderScene::RemoveRenderable(id)                   ← GPU 리소스 해제 예약
  매 프레임: RenderScene::UpdateTransforms(frameSnapshot)         ← UBO만 갱신
```

**대응하는 언리얼 개념:**
- `RenderScene` ≈ FScene
- `AddRenderable()` ≈ FScene::AddPrimitive()
- `RemoveRenderable()` ≈ FScene::RemovePrimitive()
- `UpdateTransforms()` ≈ SendRenderTransform_Concurrent()가 큐잉한 Transform 업데이트 처리

#### 3단계: Command Queue + 스레드 분리 (최종 목표)

**목표:** Game Thread와 Render Thread를 실제로 분리한다.

```
[Game Thread]                              [Render Thread]
Frame N:                                   Frame N-1:
  Tick()                                     ProcessCommands()
  Transform 갱신                              RenderScene::Render()
  FrameSnapshot 구축                           RenderPass::Execute()
  CommandQueue.Push(snapshot) ──────►          Submit → Present
  (다음 프레임으로 즉시 진행)
```

**대응하는 언리얼 개념:**
- `CommandQueue` ≈ ENQUEUE_RENDER_COMMAND 매크로가 사용하는 내부 큐
- 1프레임 지연 ≈ 언리얼의 Game Thread/Render Thread pipelining

### B.4 ID 기반 리소스 관리 (포인터 키 대체)

```
변경 전:
  unordered_map<Model*, RHIBuffer*>      ← raw pointer 키 → dangling 위험

변경 후:
  unordered_map<uint32, RHIBuffer*>      ← ID 키 → 안전

  ID 발급: Model 등록 시 RenderScene이 고유 ID 부여
  ID 해제: Model 삭제 시 Game Thread가 RemoveRenderable(id) 호출
```

**대응하는 언리얼 개념:**
- `FPrimitiveComponentId` — UPrimitiveComponent마다 부여되는 고유 ID
- `FPrimitiveSceneInfo` 내부에 `PrimitiveSceneProxy`와 `PrimitiveComponentId` 매핑

### B.5 Deferred Deletion (GPU 안전 삭제)

```cpp
// 현재: 즉시 삭제 → GPU in-flight 리소스 접근 위험
void RenderResourceManager::ReleaseAll() {
    _rhiContext->DestroyBuffer(buffer);  // GPU가 아직 사용 중일 수 있음
}

// 개선: N프레임 후 삭제
struct DeferredDeletion {
    RHIBuffer* resource;
    uint32 frameToDelete;  // frameIndex + MAX_FRAMES_IN_FLIGHT
};
std::vector<DeferredDeletion> _deletionQueue;

void ProcessDeletions(uint32 currentFrame) {
    // currentFrame >= frameToDelete 인 항목만 실제 삭제
}
```

**대응하는 언리얼 개념:**
- `FDeferredCleanupInterface` / `BeginCleanup()` — Render Thread에서 안전한 시점에 삭제
- `FRenderResource::ReleaseResource()` — 즉시 삭제가 아닌 큐잉

---

## Part C: 현재 HSMR 코드와의 대응 맵

| HSMR 현재 | 역할 | 언리얼 대응 | 파일 위치 (UE5) |
|:----------|:-----|:-----------|:----------------|
| `Model*` (Game 객체) | 렌더 가능 엔티티 | `UPrimitiveComponent` | `Components/PrimitiveComponent.h` |
| `RenderModel` (Renderer 내부) | 렌더링용 데이터 묶음 | `FPrimitiveSceneProxy` + `FPrimitiveSceneInfo` | `PrimitiveSceneProxy.h`, `PrimitiveSceneInfo.h` |
| `BuildSceneResource()` | 매 프레임 CPU→GPU 변환 | `FScene::AddPrimitive()` (1회) + `GetDynamicMeshElements()` (매 프레임) | `Scene.cpp`, `StaticMeshRender.cpp` |
| `RenderResourceManager` | GPU 리소스 캐시 관리 | `FPrimitiveSceneInfo::CacheMeshDrawCommands()` | `MeshDrawCommands.cpp` |
| `MaterialResource` | 머티리얼 GPU 리소스 | `FMaterialRenderProxy` | `MaterialRenderProxy.h` |
| `Renderer::Render()` | 프레임 렌더링 오케스트레이션 | `FSceneRenderer::Render()` | `SceneRendering.cpp` |
| `SceneResource` | 씬의 GPU 리소스 집합 | `FScene` (영속) + `FSceneRenderer` (프레임 단위) | `Scene.h`, `SceneRendering.h` |
| `_curCommandBuffer` (1개) | GPU 커맨드 기록 | `FRHICommandListImmediate` (+ 병렬 커맨드 리스트) | `RHICommandList.h` |
| 없음 | 스레드 간 통신 | `ENQUEUE_RENDER_COMMAND()` | `RenderThread.h` |
| 없음 | 지연 삭제 | `FDeferredCleanupInterface` | `DeferredCleanupInterface.h` |

---

## Part D: 구현 우선순위 & 의존관계

```
1단계: FrameSnapshot (선행 조건 없음)
  ├─ RenderableSnapshot, CameraSnapshot 구조체 정의
  ├─ Game 측에서 FrameSnapshot 구축 로직
  ├─ Renderer::Render(FrameSnapshot&) 시그니처 변경
  └─ BuildSceneResource()가 snapshot 데이터만 사용하도록 수정

2단계: ID 기반 리소스 관리 (1단계 완료 후)
  ├─ 리소스 맵 키를 포인터 → ID로 전환
  ├─ RenderScene 클래스 도입 (AddRenderable/RemoveRenderable)
  └─ 매 프레임 전체 순회 제거, 이벤트 기반 갱신

3단계: Deferred Deletion (2단계와 병행 가능)
  ├─ DeletionQueue 구현
  ├─ MAX_FRAMES_IN_FLIGHT 기반 삭제 타이밍
  └─ WaitForIdle() 호출 제거

4단계: Command Queue + 스레드 분리 (1~3단계 완료 후)
  ├─ RenderCommand 정의 (Transform, Add, Remove, ...)
  ├─ SPSC 또는 MPSC 큐 구현
  ├─ Render Thread 루프
  └─ 동기화 포인트 (Fence)
```

---

## 참고 자료

### 언리얼 공식 문서
- [Threaded Rendering](https://dev.epicgames.com/documentation/en-us/unreal-engine/threaded-rendering-in-unreal-engine)
- [Mesh Drawing Pipeline](https://dev.epicgames.com/documentation/en-us/unreal-engine/mesh-drawing-pipeline-in-unreal-engine)
- [Graphics Programming Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/graphics-programming-overview-for-unreal-engine)

### API 레퍼런스
- [FPrimitiveSceneProxy](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/FPrimitiveSceneProxy)
- [FStaticMeshSceneProxy](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/FStaticMeshSceneProxy)
- [FMeshBatch](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/FMeshBatch)
- [FMeshDrawCommand](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Renderer/FMeshDrawCommand)

### 커뮤니티 자료
- [Unreal Source Explained - Rendering](https://github.com/donaldwuid/unreal_source_explained/blob/master/main/rendering.md)
- [Gamedev Guide - Render Architecture](https://ikrima.dev/ue4guide/graphics-development/render-architecture/overview/)
- [Creating a Custom Mesh Component - Scene Proxy](https://medium.com/realities-io/creating-a-custom-mesh-component-in-ue4-part-3-the-mesh-components-scene-proxy-6965a3ea4cc9)
