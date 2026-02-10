# ECS + Scene 시스템 구현 계획

## 결정 사항

### EnTT vs Scratch 비교 결과

| 기준 | EnTT | Scratch 구현 |
|:-----|:-----|:-------------|
| **개발 시간** | 1-2일 (통합) | 1-2주 |
| **성능** | 최적화됨 (Minecraft 사용) | 직접 최적화 필요 |
| **C++ 버전** | C++20 필요 ✅ | 제약 없음 |
| **학습 가치** | 낮음 | 높음 (하지만 핵심 목표 아님) |
| **SceneGraph 통합** | 별도 구현 필요 | 처음부터 설계 |

### 결론: **EnTT 사용**

**이유:**
1. ECS 자체는 학습 보호 영역이 아님
2. 시간을 GPU-Driven/RenderGraph 등 핵심 영역에 투자하는 것이 목적에 부합
3. 검증된 라이브러리로 안정성 확보
4. SceneGraph는 어차피 별도 구현 필요

---

## 아키텍처 개요

```
┌─────────────────────────────────────────────────────────┐
│                      Scene                              │
│  ┌─────────────────┐    ┌─────────────────────────────┐ │
│  │  entt::registry │    │     SceneGraph              │ │
│  │  (ECS 저장소)    │    │  (계층 구조, Transform 전파) │ │
│  └────────┬────────┘    └──────────────┬──────────────┘ │
│           │                            │                │
│           └────────────┬───────────────┘                │
│                        ▼                                │
│              ┌─────────────────┐                        │
│              │  Scene Query API │                        │
│              │  (통합 인터페이스) │                        │
│              └────────┬────────┘                        │
└───────────────────────┼─────────────────────────────────┘
                        │
                        ▼ [직접 구현 - 보호 영역]
              ┌─────────────────┐
              │  SceneResource  │
              │  - Flatten      │
              │  - GPU 버퍼 관리  │
              │  - Culling 연동  │
              └─────────────────┘
```

---

## 구현 범위

### AI 구현 (이 계획의 범위)
- [x] EnTT 라이브러리 통합
- [ ] 기본 Component 정의
- [ ] SceneGraph 노드 구조
- [ ] Scene 클래스 (통합 관리)
- [ ] Transform 컴포넌트 및 계층 전파

### 직접 구현 (보호 영역 - 범위 외)
- SceneResource (GPU Flat Buffer)
- Flatten 전략 및 Dirty 관리
- GPU Culling 연동

---

## 1단계: EnTT 라이브러리 통합

### 1.1 라이브러리 다운로드
**위치**: `Dependency/include/entt/`

```bash
# EnTT는 header-only, 단일 헤더 사용
# https://github.com/skypjack/entt/releases 에서 최신 버전 다운로드
# entt.hpp → Dependency/include/entt/entt.hpp
```

### 1.2 CMake 설정 확인
EnTT는 header-only이므로 추가 링크 불필요. 기존 include 경로에 포함됨:
```cmake
include_directories(SYSTEM ${HS_DEPS_INCLUDE_DIR})  # 이미 존재
```

---

## 2단계: 디렉터리 구조

```
Source/Engine/Scene/
├── CMakeLists.txt          (신규)
├── Scene.h                  (Scene 클래스)
├── SceneGraph.h             (계층 구조)
├── Entity.h                 (Entity wrapper)
├── Components/
│   ├── TransformComponent.h
│   ├── MeshComponent.h
│   ├── MaterialComponent.h
│   ├── CameraComponent.h
│   └── LightComponent.h
└── Private/
    ├── Scene.cpp
    ├── SceneGraph.cpp
    └── Components/
        └── TransformComponent.cpp
```

---

## 3단계: Entity Wrapper

**파일**: `Source/Engine/Scene/Entity.h`

```cpp
#pragma once
#include "Precompile.h"
#include <entt/entt.hpp>

HS_NS_BEGIN

class Scene;

// EnTT entity를 감싸는 편의 클래스
class HS_API Entity
{
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene);
    Entity(const Entity&) = default;

    template<typename T, typename... Args>
    T& AddComponent(Args&&... args);

    template<typename T>
    T& GetComponent();

    template<typename T>
    const T& GetComponent() const;

    template<typename T>
    bool HasComponent() const;

    template<typename T>
    void RemoveComponent();

    bool IsValid() const;

    entt::entity GetHandle() const { return _handle; }

    bool operator==(const Entity& other) const;
    bool operator!=(const Entity& other) const;

private:
    entt::entity _handle{ entt::null };
    Scene* _scene = nullptr;
};

HS_NS_END
```

---

## 4단계: Transform Component

**파일**: `Source/Engine/Scene/Components/TransformComponent.h`

```cpp
#pragma once
#include "Precompile.h"
#include "Core/Math/Common.h"

HS_NS_BEGIN

struct HS_API TransformComponent
{
    // Local transform (부모 기준)
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };  // identity
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

    // Cached world transform (SceneGraph에서 계산)
    glm::mat4 worldMatrix{ 1.0f };

    // Hierarchy (SceneGraph 연동)
    entt::entity parent{ entt::null };
    std::vector<entt::entity> children;

    // Dirty flag (변경 감지)
    bool isDirty = true;

    // Local matrix 계산
    glm::mat4 GetLocalMatrix() const;

    // Transform 조작
    void SetPosition(const glm::vec3& pos);
    void SetRotation(const glm::quat& rot);
    void SetScale(const glm::vec3& scl);
    void SetEulerAngles(const glm::vec3& euler);

    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;
};

HS_NS_END
```

---

## 5단계: 기타 Component

### MeshComponent
```cpp
struct HS_API MeshComponent
{
    Mesh* mesh = nullptr;
    uint32 submeshIndex = 0;
};
```

### MaterialComponent
```cpp
struct HS_API MaterialComponent
{
    Material* material = nullptr;
};
```

### CameraComponent
```cpp
struct HS_API CameraComponent
{
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    bool isActive = false;
};
```

### LightComponent
```cpp
enum class ELightType : uint8 { Directional, Point, Spot };

struct HS_API LightComponent
{
    ELightType type = ELightType::Directional;
    glm::vec3 color{ 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;
    float range = 10.0f;        // Point/Spot
    float spotAngle = 45.0f;    // Spot only
};
```

### TagComponent
```cpp
struct HS_API TagComponent
{
    std::string name;
    uint32 layer = 0;
    bool isStatic = false;
};
```

---

## 6단계: SceneGraph

**파일**: `Source/Engine/Scene/SceneGraph.h`

```cpp
#pragma once
#include "Precompile.h"
#include <entt/entt.hpp>

HS_NS_BEGIN

class HS_API SceneGraph
{
public:
    SceneGraph(entt::registry& registry);

    // 계층 관계 설정
    void SetParent(entt::entity child, entt::entity parent);
    void RemoveParent(entt::entity child);

    // 계층 쿼리
    entt::entity GetParent(entt::entity entity) const;
    const std::vector<entt::entity>& GetChildren(entt::entity entity) const;
    std::vector<entt::entity> GetDescendants(entt::entity entity) const;
    entt::entity GetRoot(entt::entity entity) const;

    // Transform 전파
    void UpdateWorldTransforms();
    void MarkDirty(entt::entity entity);

    // 계층 순회
    template<typename Func>
    void TraverseDepthFirst(entt::entity root, Func&& func);

    template<typename Func>
    void TraverseBreadthFirst(entt::entity root, Func&& func);

private:
    void updateWorldTransformRecursive(entt::entity entity, const glm::mat4& parentWorld);
    void markDirtyRecursive(entt::entity entity);

    entt::registry& _registry;
    std::vector<entt::entity> _roots;  // 루트 엔티티 목록
};

HS_NS_END
```

---

## 7단계: Scene 클래스

**파일**: `Source/Engine/Scene/Scene.h`

```cpp
#pragma once
#include "Precompile.h"
#include "Scene/Entity.h"
#include "Scene/SceneGraph.h"
#include <entt/entt.hpp>

HS_NS_BEGIN

class HS_API Scene
{
public:
    Scene(const std::string& name = "Untitled");
    ~Scene();

    // Entity 생성/삭제
    Entity CreateEntity(const std::string& name = "Entity");
    Entity CreateChildEntity(Entity parent, const std::string& name = "Entity");
    void DestroyEntity(Entity entity);

    // Entity 검색
    Entity FindEntityByName(const std::string& name);
    std::vector<Entity> FindEntitiesByTag(const std::string& tag);

    // Component 쿼리 (EnTT view 래핑)
    template<typename... Components>
    auto View();

    template<typename... Components>
    auto View() const;

    // SceneGraph 접근
    SceneGraph& GetSceneGraph() { return _sceneGraph; }
    const SceneGraph& GetSceneGraph() const { return _sceneGraph; }

    // 매 프레임 호출
    void Update(float deltaTime);

    // Scene 정보
    const std::string& GetName() const { return _name; }
    void SetName(const std::string& name) { _name = name; }

    // Registry 직접 접근 (고급 사용)
    entt::registry& GetRegistry() { return _registry; }
    const entt::registry& GetRegistry() const { return _registry; }

private:
    std::string _name;
    entt::registry _registry;
    SceneGraph _sceneGraph;
};

HS_NS_END
```

---

## 8단계: CMake 통합

### Scene 모듈 CMakeLists.txt

**파일**: `Source/Engine/Scene/CMakeLists.txt`

```cmake
set(SCENE_SOURCES
    Private/Scene.cpp
    Private/SceneGraph.cpp
    Private/Components/TransformComponent.cpp
)

set(SCENE_HEADERS
    Scene.h
    SceneGraph.h
    Entity.h
    Components/TransformComponent.h
    Components/MeshComponent.h
    Components/MaterialComponent.h
    Components/CameraComponent.h
    Components/LightComponent.h
    Components/TagComponent.h
)

# Engine 라이브러리에 포함
target_sources(Engine PRIVATE ${SCENE_SOURCES} ${SCENE_HEADERS})
```

### Engine CMakeLists.txt 수정

```cmake
# 기존 add_subdirectory 또는 source 추가에 Scene 포함
add_subdirectory(Scene)
```

---

## 9단계: 기존 시스템 연동

### Model → Entity 변환 유틸리티

```cpp
// SceneLoader.h
class HS_API SceneLoader
{
public:
    // 기존 Model을 Scene의 Entity 계층으로 변환
    static Entity LoadModelAsEntity(Scene& scene, Model* model);

    // GLTF 직접 로드 (향후)
    static Entity LoadGLTF(Scene& scene, const std::string& path);
};
```

### Camera 통합
기존 `Camera` 클래스와 `CameraComponent` 연동:
```cpp
// Camera 클래스를 CameraComponent의 데이터 소스로 활용
// 또는 CameraComponent가 Camera 인스턴스를 참조
```

---

## 구현 순서

| 순서 | 작업 | 파일 |
|:----:|:-----|:-----|
| 1 | EnTT 헤더 다운로드 및 배치 | `Dependency/include/entt/` |
| 2 | Scene 디렉터리 구조 생성 | `Source/Engine/Scene/` |
| 3 | Entity wrapper 구현 | `Entity.h` |
| 4 | TransformComponent 구현 | `TransformComponent.h/.cpp` |
| 5 | 기타 Component 정의 | `*Component.h` |
| 6 | SceneGraph 구현 | `SceneGraph.h/.cpp` |
| 7 | Scene 클래스 구현 | `Scene.h/.cpp` |
| 8 | CMake 통합 | `CMakeLists.txt` |
| 9 | 빌드 및 테스트 | - |
| 10 | 기존 시스템 연동 (선택) | `SceneLoader.h` |

---

## 향후 작업 (직접 구현 - 보호 영역)

구현 완료 후 사용자가 직접 작업할 영역:

1. **SceneResource 구조 설계**
   - GPU에 전달할 Flat 버퍼 레이아웃
   - `GPUObjectData` 구조체 정의

2. **Flatten 전략**
   - Dirty flag 기반 부분 업데이트
   - 배치 처리 최적화

3. **렌더링 연동**
   - Scene → RenderPath 데이터 흐름
   - Culling 시스템 통합

---

## 참고 자료

- [EnTT GitHub](https://github.com/skypjack/entt)
- [EnTT Crash Course](https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system)
- [EnTT in Minecraft](https://www.codingwiththomas.com/blog/use-entt-when-you-need-an-ecs)
