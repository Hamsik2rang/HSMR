# 아키텍처 분석: SceneResource 빌드 방식 vs 언리얼 Proxy 패턴

## Context
현재 `Renderer::Render()` 안에서 `BuildSceneResource()`를 매 프레임 호출하여 CPU 객체(Model*, Camera*)를 GPU-ready `SceneResource`로 변환하는 구조가, 게임스레드/렌더스레드 분리에 적합한지 분석한다. 언리얼 엔진의 Proxy 객체 아키텍처와 비교하여 차이점과 미흡한 점을 정리한다.

---

## 1. 현재 HSMR 아키텍처 요약

### 데이터 흐름 (단일 스레드)
```
EditorWindow::onRender()  [메인 스레드]
  │
  ├─ Renderer::Render(models, cameras, scene, renderTarget)
  │     │
  │     ├─ Model::Update()                    ← CPU Transform 갱신
  │     ├─ BuildSceneResource(models, ...)    ← GPU 리소스 resolve + SceneResource 조립
  │     ├─ _updateSceneBuffers(sceneResource) ← UBO 업로드 (PerView, PerDraw)
  │     └─ RenderPass::Execute(cmdBuffer, sceneResource)  ← Draw 기록
  │
  └─ Submit → Present
```

### 핵심 특징
- **모든 작업이 동일 스레드에서 순차 실행**: Transform 갱신 → 리소스 빌드 → UBO 업로드 → Draw 기록
- **CPU 객체 포인터를 직접 참조**: `RenderModel.source = model` (원본 Model* 저장)
- **리소스 캐싱은 CPU 포인터 키 기반**: `unordered_map<Model*, RHIBuffer*>`
- **SceneResource는 매 프레임 재구축**: 개별 GPU 리소스는 캐시되지만, 조립(assembly)은 매번 반복
- **단일 커맨드 버퍼**: `_curCommandBuffer` 하나로 모든 패스 기록

---

## 2. 언리얼 엔진 Proxy 아키텍처 요약

### 핵심 개념: "Game Thread 객체와 Render Thread 객체의 완전 분리"

```
[Game Thread]                          [Render Thread]
UPrimitiveComponent                    FPrimitiveSceneProxy
ULightComponent          ──enqueue──►  FLightSceneProxy
UStaticMeshComponent     ENQUEUE_      FStaticMeshSceneProxy
                         RENDER_
                         COMMAND()
```

### 언리얼의 주요 설계 원칙

| 원칙 | 설명 |
|:-----|:-----|
| **Proxy 객체** | 게임 스레드 컴포넌트마다 렌더 스레드 전용 Proxy 객체 존재. Proxy는 렌더링에 필요한 데이터만 **복사**하여 보관 |
| **소유권 분리** | Game Thread는 UObject 소유, Render Thread는 FSceneProxy 소유. 서로의 데이터를 직접 접근하지 않음 |
| **Command Queue** | `ENQUEUE_RENDER_COMMAND()` 매크로로 Game→Render 단방향 메시지 전달. 렌더 스레드가 큐를 소비 |
| **프레임 지연(Frame Lag)** | Game Thread는 Render Thread보다 1~2프레임 앞서 실행. 동기화 포인트에서만 대기 |
| **불변 스냅샷** | Proxy 생성 시 필요한 데이터를 복사. 이후 업데이트는 명시적 `SendRenderTransform_Concurrent()` 등으로만 전달 |

### 언리얼 프레임 타임라인 (간략)
```
Frame N:
  [Game Thread]  Tick() → Transform 변경 → ENQUEUE_RENDER_COMMAND(UpdateTransform, ...)
  [Render Thread] (Frame N-1 렌더링 중...)

Frame N+1:
  [Game Thread]  Tick() → (다음 프레임 로직)
  [Render Thread] Dequeue commands → FSceneProxy::GetDynamicMeshElements() → Draw
```

---

## 3. 핵심 차이점 비교

### 3.1 데이터 소유권 & 접근 패턴

| 항목 | HSMR (현재) | 언리얼 |
|:-----|:------------|:-------|
| 렌더링 시 CPU 객체 접근 | `model->GetMaterial()`, `model->GetMesh()` — **원본 직접 참조** | Proxy가 **복사본** 보유. 원본 접근 금지 |
| 키 방식 | `unordered_map<Model*, ...>` — raw pointer | Proxy별 고유 ID, Component 포인터는 Game Thread에서만 사용 |
| 삭제 안전성 | Model 삭제 시 dangling pointer 위험 | Proxy는 별도 소멸 큐, `DeregisterComponent()` → Render Thread에서 Proxy 삭제 |

**문제점**: `BuildSceneResource()`에서 `model->GetMaterial()`, `model->GetMesh()`를 호출하는 시점에 해당 객체가 다른 스레드에서 수정/삭제될 수 있음. 단일 스레드에서는 문제없지만, **스레드 분리 시 즉시 data race**.

### 3.2 동기화 모델

| 항목 | HSMR (현재) | 언리얼 |
|:-----|:------------|:-------|
| 스레드 모델 | 단일 스레드 (순차) | Game/Render 2스레드 (병렬 + Command Queue) |
| 데이터 전달 | 함수 파라미터 직접 전달 (`models` vector) | `ENQUEUE_RENDER_COMMAND()` — 람다 캡처로 값 복사 |
| 동기화 | 없음 (필요 없음) | `FRenderCommandFence`, `FlushRenderingCommands()` |
| 프레임 지연 | 0 (동일 프레임) | 1~2 프레임 |

**문제점**: 현재 구조에서 Game/Render 분리를 시도하면, `Renderer::Render()`에 전달되는 `const std::vector<Model*>& models`가 Game Thread에서 수정 중일 수 있음.

### 3.3 리소스 생명주기

| 항목 | HSMR (현재) | 언리얼 |
|:-----|:------------|:-------|
| GPU 리소스 생성 시점 | `BuildSceneResource()` 내 lazy 생성 (렌더 경로에서) | Proxy 등록 시 또는 별도 InitViews 단계 |
| 무효화 | `isValid` 플래그만 있음, 외부 삭제 감지 불가 | Proxy 소멸 → 렌더 커맨드로 GPU 리소스 해제 지시 |
| 해제 | `ReleaseAll()` 일괄 해제 | 개별 Proxy 소멸 시 Deferred Deletion Queue |
| 프레임 안전성 | GPU in-flight 체크 없음 | Deferred Deletion은 N프레임 뒤 실행 (GPU 완료 보장) |

**문제점**: GPU가 아직 사용 중인 리소스를 CPU에서 바로 해제하면 validation error. 현재는 `WaitForIdle()` 호출이 유일한 보호 수단인데, 이는 성능 저하를 유발함.

### 3.4 SceneResource 빌드 vs. FScene::GetRelevantPrimitives()

| 항목 | HSMR `BuildSceneResource()` | 언리얼 `FScene` |
|:-----|:---------------------------|:----------------|
| 호출 빈도 | **매 프레임** 전체 재조립 | 등록/해제 시에만 리스트 변경 |
| 가시성 판단 | 없음 (전체 모델 패스) | Octree + Frustum Culling → Relevant Primitives |
| 정렬 | 없음 | Draw Policy → State Bucket Sorting |
| 출력 | `SceneResource` (flat vector) | `FMeshBatch` → `FMeshDrawCommand` (사전 정렬) |

**문제점**: 매 프레임 `BuildSceneResource()`에서 전체 모델을 순회하며 `GetOrCreate*`를 호출하는 것은 씬 규모가 커지면 병목. 언리얼은 등록/해제 이벤트 기반으로 렌더러의 리스트를 유지하므로 매 프레임 재구축이 불필요.

---

## 4. 구체적 미흡한 점 (스레드 분리 관점)

### 4.1 직접 포인터 참조 (가장 큰 문제)
```cpp
// 현재: BuildSceneResource() 내부
Material* mat = model->GetMaterial();  // ← Game Thread 객체를 Render 경로에서 직접 접근
Mesh* mesh = model->GetMesh();
```
- 스레드 분리 시 Game Thread가 `model->SetMaterial(newMat)` 하는 동안 Render Thread가 읽으면 data race
- **해결 방향**: Proxy 또는 스냅샷 패턴 — 렌더링에 필요한 데이터를 값으로 복사

### 4.2 암시적 상태 전달 (`_active*` 패턴)
```cpp
// 현재: BuildSceneResource() 내부
SetActiveCameraResource(sceneResource.cameraResources[0]);
_activePerDrawBuffer = perDrawBuffer;
// ... 이후 createResourceLayoutFromReflection()에서 _activeCameraResource 참조
```
- 함수 파라미터가 아닌 멤버 변수로 상태 전달 → 재진입 불가, 스레드 안전하지 않음
- **해결 방향**: Context 구조체에 담아 명시적 파라미터로 전달

### 4.3 프레임 동기화 부재
- GPU가 Frame N-1의 커맨드를 실행 중인데, CPU가 Frame N의 UBO를 같은 버퍼에 덮어쓸 수 있음
- 현재 `EBufferMemoryOption::Dynamic`으로 생성하지만, 더블/트리플 버퍼링 없음
- **해결 방향**: Per-frame ring buffer 또는 프레임별 UBO 복사

### 4.4 리소스 삭제 안전성 없음
- `_perDrawBuffers[deletedModel]` → dangling key
- `_materialResources[deletedMaterial]` → dangling key + 리소스 누수
- **해결 방향**: 약한 참조(weak_ptr), 이벤트 기반 등록/해제, 또는 ID 기반 맵핑

### 4.5 매 프레임 전체 재조립 비용
- 매 프레임 모든 Model에 대해 `GetOrCreate*` 호출 (캐시 히트여도 map lookup 발생)
- 씬 1000개 오브젝트 기준: 프레임당 ~5000회 hash lookup
- **해결 방향**: 이벤트 기반(dirty flag) — 변경된 객체만 업데이트

---

## 5. 요약: 현재 구조의 적합성 평가

| 평가 항목 | 현재 상태 | 스레드 분리 적합성 |
|:----------|:---------|:-----------------|
| 데이터 소유권 분리 | 미분리 (원본 직접 참조) | ❌ 부적합 |
| 스레드간 통신 채널 | 없음 (동일 스레드) | ❌ 부재 |
| 프레임 동기화 | 없음 (순차 실행) | ❌ 부재 |
| 리소스 생명주기 | Lazy + ReleaseAll | ⚠️ 부분적 |
| GPU 리소스 캐싱 | 포인터 키 기반 캐시 | ⚠️ ID 기반 전환 필요 |
| 씬 구축 비용 | 매 프레임 전체 순회 | ⚠️ 이벤트 기반 전환 필요 |

### 결론
현재 구조는 **단일 스레드 프로토타이핑 단계에서는 합리적**이지만, Game/Render 스레드 분리를 도입하려면 다음 순서로 리팩터링이 필요하다:

1. **Proxy/스냅샷 도입**: 렌더링 경로에서 CPU 원본 객체 직접 접근 제거
2. **Command Queue**: Game→Render 단방향 메시지 채널
3. **프레임 리소스 관리**: Per-frame UBO 더블 버퍼링
4. **이벤트 기반 씬 관리**: 등록/해제/변경 이벤트로 `SceneResource` 점진 갱신
5. **Deferred Deletion**: GPU 완료 보장 후 리소스 해제

이 항목들은 모두 CLAUDE.md의 Protected Domain(렌더링 아키텍처 설계)에 해당하므로, 직접 학습하며 구현하는 것을 권장한다.
