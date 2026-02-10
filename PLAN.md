# 컴포넌트 구조 리팩토링 계획

## 현재 상태 분석

### 현재 구조 (분리형)

```
Entity
├── TagComponent
├── TransformComponent
├── MeshComponent       ← mesh*, submeshIndex, castShadow
└── MaterialComponent   ← material*
```

### 렌더링 파이프라인 현황

현재 **두 개의 분리된 시스템**이 존재:

1. **Model 클래스** (실제 렌더링에 사용)
   ```cpp
   class Model {
       Scoped<Mesh> _mesh;
       Scoped<Material> _material;  // Mesh + Material 이미 통합!
       glm::vec3 _position, _rotation, _scale;
       glm::mat4 _worldMatrix;
   };
   ```
   - `ForwardOpaquePass`에서 `param.models` 순회
   - `model->GetMesh()`, `model->GetMaterial()` 사용

2. **ECS Scene/Entity** (에디터 UI용, 렌더링 미연결)
   ```cpp
   struct MeshComponent { Mesh* mesh; };
   struct MaterialComponent { Material* material; };
   ```
   - Hierarchy/Inspector UI에서만 사용
   - 실제 렌더링과 **완전히 분리**됨

### 문제점

1. **이중 구조**: Model과 ECS가 따로 놂
2. **데이터 불일치**: ECS에서 Transform 수정해도 렌더링에 반영 안 됨
3. **불필요한 분리**: MeshComponent, MaterialComponent가 별도인데 의미 없음
4. **Submesh-Material 매핑 불가**: 현재 구조로는 불가능

---

## 리팩토링 목표

### 통합형 구조 (Unity/Unreal 스타일)

```
Entity
├── TagComponent
├── TransformComponent
└── MeshRendererComponent    ← 새로운 통합 컴포넌트
    ├── mesh*
    ├── materials[]          ← submesh별 머티리얼 배열
    ├── castShadow
    ├── receiveShadow
    └── bounds (AABB)
```

### 핵심 변경

1. **MeshComponent + MaterialComponent → MeshRendererComponent** 통합
2. **렌더러가 ECS를 직접 쿼리**: `View<TransformComponent, MeshRendererComponent>`
3. **Model 클래스 역할 축소**: 에셋 로딩용으로만 사용, 런타임은 ECS

---

## 상세 설계

### 1. MeshRendererComponent

```cpp
// Source/Engine/Scene/Components/MeshRendererComponent.h

struct HS_API MeshRendererComponent
{
    Mesh* mesh = nullptr;
    std::vector<Material*> materials;  // submesh별 머티리얼

    bool castShadow = true;
    bool receiveShadow = true;
    bool isVisible = true;

    // Bounds (피킹, 컬링용)
    AABB localBounds;
    AABB worldBounds;  // TransformComponent와 연동하여 계산

    // 렌더링 레이어/마스크
    uint32 renderLayerMask = 0xFFFFFFFF;

    MeshRendererComponent() = default;
    MeshRendererComponent(Mesh* m, Material* mat = nullptr)
        : mesh(m)
    {
        if (mat) materials.push_back(mat);
    }

    // Submesh 개수와 Material 개수 동기화
    void SetMesh(Mesh* m);
    Material* GetMaterial(uint32 submeshIndex = 0) const;
    void SetMaterial(Material* mat, uint32 submeshIndex = 0);
};
```

### 2. Components.h 수정

```cpp
// Source/Engine/Scene/Components/Components.h

#include "Scene/Components/TagComponent.h"
#include "Scene/Components/TransformComponent.h"
#include "Scene/Components/MeshRendererComponent.h"  // 새로 추가
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/LightComponent.h"

// MeshComponent, MaterialComponent 제거 또는 deprecated
```

### 3. 렌더러 통합

```cpp
// ForwardOpaquePass::Execute() 수정

void ForwardOpaquePass::Execute(RHICommandBuffer* cmd, RHIRenderPass* pass, const RenderParameter& param)
{
    Scene* scene = param.scene;  // Scene 직접 전달
    if (!scene) return;

    // ECS 쿼리로 렌더링 대상 수집
    auto view = scene->View<TransformComponent, MeshRendererComponent>();

    for (auto entity : view)
    {
        auto& transform = view.get<TransformComponent>(entity);
        auto& renderer = view.get<MeshRendererComponent>(entity);

        if (!renderer.mesh || !renderer.isVisible) continue;

        // Frustum culling
        if (!camera->IsBoundsVisible(renderer.worldBounds)) continue;

        // 각 submesh 렌더링
        for (uint32 i = 0; i < renderer.mesh->GetSubmeshCount(); ++i)
        {
            Material* mat = renderer.GetMaterial(i);
            if (!mat) continue;

            // 렌더링 로직...
        }
    }
}
```

### 4. RenderParameter 수정

```cpp
struct HS_API RenderParameter
{
    Scene* scene = nullptr;              // ECS Scene 직접 참조
    std::vector<Camera*> cameras;
    RenderResourceManager* resourceManager = nullptr;
    ShaderLibrary* shaderLibrary = nullptr;

    // std::vector<Model*> models;       // 제거 또는 legacy용
};
```

---

## 마이그레이션 단계

### Phase 1: MeshRendererComponent 추가 (하위 호환 유지)

| 순서 | 작업 | 파일 |
|:----:|:-----|:-----|
| 1-1 | MeshRendererComponent 정의 | `Components/MeshRendererComponent.h` |
| 1-2 | Components.h에 추가 | `Components/Components.h` |
| 1-3 | InspectorPanel에 MeshRenderer 편집 UI 추가 | `InspectorPanel.cpp` |
| 1-4 | 빌드 및 테스트 | - |

### Phase 2: 렌더러 ECS 통합

| 순서 | 작업 | 파일 |
|:----:|:-----|:-----|
| 2-1 | RenderParameter에 Scene* 추가 | `RendererDefinition.h` |
| 2-2 | ForwardOpaquePass에서 Scene 쿼리 지원 | `ForwardOpaquePass.cpp` |
| 2-3 | EditorWindow에서 Scene 전달 | `EditorWindow.cpp` |
| 2-4 | Model 기반 렌더링과 병행 지원 | - |
| 2-5 | 빌드 및 테스트 | - |

### Phase 3: 기존 구조 제거 (선택)

| 순서 | 작업 | 파일 |
|:----:|:-----|:-----|
| 3-1 | MeshComponent, MaterialComponent deprecated | 헤더에 주석 |
| 3-2 | Model 클래스를 에셋 로딩 전용으로 변경 | `Model.h/.cpp` |
| 3-3 | ObjectManager가 Model → Entity 변환 지원 | `ObjectManager.cpp` |

---

## 영향받는 파일

### 수정 필요

```
Source/Engine/Scene/Components/
├── MeshRendererComponent.h    (신규)
├── Components.h               (수정: include 추가)
├── MeshComponent.h            (deprecated 마킹)
└── MaterialComponent.h        (deprecated 마킹)

Source/Engine/Renderer/
├── RendererDefinition.h       (RenderParameter 수정)
└── RenderPass/Private/ForwardOpaquePass.cpp  (ECS 쿼리)

Source/Editor/
├── Panel/InspectorPanel.cpp   (MeshRenderer UI)
├── Panel/HierarchyPanel.cpp   (아이콘 변경)
└── Core/EditorWindow.cpp      (Scene 전달)
```

### 하위 호환

- Model 클래스는 유지 (에셋 로딩용)
- 기존 `param.models` 방식도 당분간 지원

---

## 트레이드오프 분석

### 장점

1. **논리적 일관성**: Mesh 없이 Material 의미 없음
2. **Submesh-Material 매핑**: `materials[submeshIdx]` 자연스러움
3. **렌더러 단순화**: 한 컴포넌트만 쿼리
4. **Unity/Unreal 패턴**: 검증된 설계

### 단점

1. **마이그레이션 비용**: 기존 코드 수정 필요
2. **Material 공유 복잡**: 여러 Entity가 같은 Material 인스턴스 사용 시 관리 필요

### 결론

**통합형 권장**. Mesh와 Material은 렌더링에서 분리 불가능한 쌍이며,
현재 Model 클래스도 이미 이 방식을 사용 중.
ECS도 이에 맞춰 정리하는 것이 일관성 있음.

---

## 구현 우선순위

1. **[필수] MeshRendererComponent 추가** - 새 컴포넌트로 병행 사용
2. **[권장] 렌더러 ECS 통합** - Scene 직접 쿼리
3. **[선택] 기존 구조 제거** - 안정화 후 진행

---

## 예상 결과

리팩토링 후:

```cpp
// Entity 생성
Entity cube = scene->CreateEntity("Cube");
auto& renderer = cube.AddComponent<MeshRendererComponent>();
renderer.mesh = meshAsset;
renderer.materials.push_back(materialAsset);

// 렌더링 (ForwardOpaquePass)
auto view = scene->View<TransformComponent, MeshRendererComponent>();
for (auto entity : view) {
    // Transform + MeshRenderer 바로 사용
}
```

Inspector UI:
```
▼ Mesh Renderer
  Mesh: [Cube.fbx]
  ▼ Materials
    [0] DefaultPBR
    [1] GlassMaterial
  ☑ Cast Shadow
  ☑ Receive Shadow
```
