# HSMR 경량화 프로토타이핑 프레임워크 설계

## 목표

렌더링 기법을 빠르게 프로토타이핑하고 성능을 측정하기 위한 경량 구조로 리팩토링

## 현재 구조 vs 목표 구조

### 현재 (복잡)
```
main.cpp
  → EditorApplication
    → EditorWindow : Window
      → GUIContext
      → Panel 시스템 (7개 패널)
      → RenderPath
      → EditorCamera
```

### 목표 (경량)
```
main.cpp
  → Application
    ├── Window (최소한)
    ├── Camera
    ├── Renderer
    ├── Scene (JSON 로드)
    ├── Profiler (ImPlot)
    └── Gizmo (ImGuizmo)
```

---

## 핵심 컴포넌트 설계

### 1. Application 클래스

```cpp
class Application
{
public:
    bool Init(const char* configPath);    // RHI 초기화, 설정 로드
    bool LoadScene(const char* scenePath); // JSON 씬 로드
    void Run();                            // 메인 루프
    void Shutdown();

private:
    // 핵심 컴포넌트
    Window*     _window;
    Camera*     _camera;
    Renderer*   _renderer;
    Scene*      _scene;
    Profiler*   _profiler;

    // ImGui 관련
    bool        _showProfiler = true;
    bool        _showGizmo = true;
};
```

### 2. Scene 클래스 (JSON 기반)

```cpp
class Scene
{
public:
    bool LoadFromJSON(const std::string& path);
    void Clear();

    // 리소스 접근
    const std::vector<Model*>& GetModels() const;
    const std::vector<Shader*>& GetShaders() const;
    const std::vector<Texture*>& GetTextures() const;

    // 오브젝트 조작 (ImGuizmo용)
    SceneObject* GetSelectedObject();
    void SetSelectedObject(SceneObject* obj);

private:
    std::vector<Scoped<Model>> _models;
    std::vector<Scoped<Shader>> _shaders;
    std::vector<Scoped<Texture>> _textures;
    std::vector<SceneObject> _objects;  // Transform + Model 참조
    SceneObject* _selectedObject = nullptr;
};
```

### 3. JSON 씬 포맷

```json
{
  "name": "PBR Test Scene",

  "camera": {
    "position": [0, 2, -5],
    "target": [0, 0, 0],
    "fov": 60,
    "near": 0.1,
    "far": 1000
  },

  "shaders": [
    {
      "name": "pbr",
      "path": "shaders/PBR.slang",
      "stages": ["vertex", "fragment"]
    }
  ],

  "textures": [
    { "name": "envMap", "path": "textures/environment.hdr" },
    { "name": "albedo", "path": "textures/rustediron_albedo.png" }
  ],

  "models": [
    {
      "name": "sponza",
      "path": "models/sponza.gltf",
      "settings": {
        "generateNormals": true,
        "generateTangents": true,
        "scale": 0.01
      }
    }
  ],

  "objects": [
    {
      "name": "Main Model",
      "model": "sponza",
      "shader": "pbr",
      "transform": {
        "position": [0, 0, 0],
        "rotation": [0, 0, 0],
        "scale": [1, 1, 1]
      }
    }
  ]
}
```

---

## 프로파일링 시스템

### 4. Profiler 클래스

```cpp
class Profiler
{
public:
    void BeginFrame();
    void EndFrame();

    // GPU 타이밍
    void BeginGPUTimer(const char* name);
    void EndGPUTimer(const char* name);

    // CPU 타이밍
    void BeginCPUTimer(const char* name);
    void EndCPUTimer(const char* name);

    // ImPlot으로 렌더링
    void DrawUI();

private:
    struct TimingData {
        std::string name;
        std::deque<float> history;  // 최근 N프레임
        float current;
        float average;
        float min, max;
    };

    std::unordered_map<std::string, TimingData> _gpuTimings;
    std::unordered_map<std::string, TimingData> _cpuTimings;

    float _frameTimeHistory[256];
    int   _frameIndex = 0;

    // RHI GPU Query 핸들
    std::vector<RHIQuery*> _gpuQueries;
};
```

### 5. ImPlot 프로파일러 UI

```cpp
void Profiler::DrawUI()
{
    if (ImGui::Begin("Profiler"))
    {
        // 프레임 타임 그래프
        if (ImPlot::BeginPlot("Frame Time", ImVec2(-1, 150)))
        {
            ImPlot::PlotLine("ms", _frameTimeHistory, 256);
            ImPlot::EndPlot();
        }

        // GPU 타이밍 막대 그래프
        if (ImPlot::BeginPlot("GPU Timings", ImVec2(-1, 200)))
        {
            for (auto& [name, data] : _gpuTimings)
            {
                ImPlot::PlotBars(name.c_str(), data.history.data(), data.history.size());
            }
            ImPlot::EndPlot();
        }

        // 상세 테이블
        if (ImGui::BeginTable("Timings", 5))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Current");
            ImGui::TableSetupColumn("Avg");
            ImGui::TableSetupColumn("Min");
            ImGui::TableSetupColumn("Max");
            ImGui::TableHeadersRow();

            for (auto& [name, data] : _gpuTimings)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%s", name.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%.3f ms", data.current);
                ImGui::TableNextColumn(); ImGui::Text("%.3f ms", data.average);
                ImGui::TableNextColumn(); ImGui::Text("%.3f ms", data.min);
                ImGui::TableNextColumn(); ImGui::Text("%.3f ms", data.max);
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}
```

---

## ImGuizmo 통합

### 6. Gizmo 시스템

```cpp
class GizmoController
{
public:
    enum class Mode { Translate, Rotate, Scale };
    enum class Space { Local, World };

    void SetMode(Mode mode) { _mode = mode; }
    void SetSpace(Space space) { _space = space; }

    // 매 프레임 호출
    bool Manipulate(Camera* camera, SceneObject* object);

    // 단축키 처리
    void ProcessInput();  // W=Translate, E=Rotate, R=Scale

private:
    Mode  _mode = Mode::Translate;
    Space _space = Space::World;
};

// 사용 예시 (Application::Update)
void Application::Update()
{
    _gizmo.ProcessInput();

    if (SceneObject* selected = _scene->GetSelectedObject())
    {
        if (_gizmo.Manipulate(_camera, selected))
        {
            // Transform이 변경됨
        }
    }
}
```

### 7. ImGuizmo 렌더링

```cpp
bool GizmoController::Manipulate(Camera* camera, SceneObject* object)
{
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    // 뷰포트 설정
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    // 행렬 준비
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 proj = camera->GetProjectionMatrix();
    glm::mat4 model = object->GetWorldMatrix();

    // Gizmo 조작
    ImGuizmo::OPERATION op = GetImGuizmoOperation(_mode);
    ImGuizmo::MODE mode = (_space == Space::Local) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

    bool changed = ImGuizmo::Manipulate(
        glm::value_ptr(view),
        glm::value_ptr(proj),
        op,
        mode,
        glm::value_ptr(model)
    );

    if (changed)
    {
        // Decompose하여 Transform에 적용
        glm::vec3 pos, rot, scale;
        ImGuizmo::DecomposeMatrixToComponents(
            glm::value_ptr(model),
            glm::value_ptr(pos),
            glm::value_ptr(rot),
            glm::value_ptr(scale)
        );
        object->SetPosition(pos);
        object->SetRotation(rot);
        object->SetScale(scale);
    }

    return changed;
}
```

---

## 사용 예시

### main.cpp (샘플 프로젝트)

```cpp
#include "Application.h"

int main()
{
    Application app;

    // 초기화 (RHI, ImGui, ImPlot, ImGuizmo)
    if (!app.Init("config.json"))
        return -1;

    // 씬 로드 (모델, 텍스처, 셰이더 일괄 로드)
    if (!app.LoadScene("scenes/pbr_test.json"))
        return -1;

    // 메인 루프
    app.Run();

    return 0;
}
```

### 메인 루프 내부

```cpp
void Application::Run()
{
    while (_window->IsOpen())
    {
        _window->ProcessEvents();

        float dt = CalculateDeltaTime();

        // 프로파일링 시작
        _profiler->BeginFrame();

        // 입력 처리 & 카메라 업데이트
        UpdateCamera(dt);

        // Gizmo 처리
        _gizmo.ProcessInput();
        if (auto* obj = _scene->GetSelectedObject())
            _gizmo.Manipulate(_camera, obj);

        // 렌더링
        _profiler->BeginGPUTimer("Render");
        _renderer->Render(_scene, _camera);
        _profiler->EndGPUTimer("Render");

        // ImGui UI
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        _profiler->DrawUI();      // 프로파일러 창
        DrawSceneHierarchy();     // 간단한 오브젝트 목록

        ImGui::Render();
        RenderImGui();

        // 프레임 완료
        _profiler->EndFrame();
        _window->Present();
    }
}
```

---

## 디렉토리 구조

```
Source/
├── Application/
│   ├── Application.h/cpp       # 메인 애플리케이션
│   ├── Scene.h/cpp             # JSON 씬 로더
│   ├── SceneObject.h/cpp       # Transform + 렌더링 정보
│   ├── Camera.h/cpp            # 카메라 (기존 EditorCamera 기반)
│   ├── MousePicker.h/cpp       # Ray casting 피킹
│   ├── GizmoController.h/cpp   # ImGuizmo 래퍼
│   └── ShaderWatcher.h/cpp     # 셰이더 핫 리로드
│
├── Profiler/
│   ├── Profiler.h/cpp          # CPU/GPU 타이밍
│   └── GPUQuery.h/cpp          # RHI GPU 쿼리 래퍼
│
├── ThirdParty/
│   ├── ImPlot/                 # 새로 추가
│   └── ImGuizmo/               # 새로 추가
│
└── Samples/                    # 샘플별 씬 파일
    ├── pbr_test.json
    ├── shadow_mapping.json
    └── ssao_test.json
```

---

## 외부 라이브러리

### nlohmann/json (JSON 파서)
- GitHub: https://github.com/nlohmann/json
- 헤더 온리: `json.hpp` 하나만 추가
- 사용법: `nlohmann::json j = nlohmann::json::parse(fileContent);`

### ImPlot
- GitHub: https://github.com/epezent/implot
- 헤더: `implot.h`, `implot_internal.h`
- 소스: `implot.cpp`, `implot_items.cpp`
- 의존성: ImGui (이미 있음)

### ImGuizmo
- GitHub: https://github.com/CedricGuillemet/ImGuizmo
- 헤더/소스: `ImGuizmo.h`, `ImGuizmo.cpp`
- 의존성: ImGui (이미 있음)

---

## 마우스 피킹 시스템

### Ray Casting 기반 오브젝트 선택

```cpp
class MousePicker
{
public:
    // 스크린 좌표 → 월드 Ray
    Ray ScreenToWorldRay(Camera* camera, float screenX, float screenY);

    // Ray-AABB 교차 테스트
    bool RayIntersectsAABB(const Ray& ray, const AABB& bounds, float& outDistance);

    // 씬에서 오브젝트 선택
    SceneObject* PickObject(Camera* camera, Scene* scene, float screenX, float screenY);

private:
    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
    };
};
```

### 구현

```cpp
Ray MousePicker::ScreenToWorldRay(Camera* camera, float screenX, float screenY)
{
    // NDC 변환 (-1 ~ 1)
    float ndcX = (2.0f * screenX / screenWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY / screenHeight);

    // 클립 공간 → 월드 공간
    glm::vec4 clipNear(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 clipFar(ndcX, ndcY, 1.0f, 1.0f);

    glm::mat4 invVP = glm::inverse(camera->GetViewProjectionMatrix());

    glm::vec4 worldNear = invVP * clipNear;
    glm::vec4 worldFar = invVP * clipFar;
    worldNear /= worldNear.w;
    worldFar /= worldFar.w;

    Ray ray;
    ray.origin = glm::vec3(worldNear);
    ray.direction = glm::normalize(glm::vec3(worldFar - worldNear));
    return ray;
}

SceneObject* MousePicker::PickObject(Camera* camera, Scene* scene, float screenX, float screenY)
{
    Ray ray = ScreenToWorldRay(camera, screenX, screenY);

    SceneObject* closest = nullptr;
    float closestDist = FLT_MAX;

    for (auto& obj : scene->GetObjects())
    {
        float dist;
        if (RayIntersectsAABB(ray, obj.GetWorldBounds(), dist))
        {
            if (dist < closestDist)
            {
                closestDist = dist;
                closest = &obj;
            }
        }
    }

    return closest;
}
```

### 사용 (마우스 클릭 시)

```cpp
void Application::OnMouseClick(float x, float y)
{
    if (SceneObject* picked = _picker.PickObject(_camera, _scene, x, y))
    {
        _scene->SetSelectedObject(picked);
    }
}
```

---

## 셰이더 핫 리로드

### ShaderWatcher 클래스

```cpp
class ShaderWatcher
{
public:
    void Watch(const std::string& shaderPath, RHIShader* shader);
    void Unwatch(const std::string& shaderPath);

    // 매 프레임 호출 - 변경된 셰이더 재컴파일
    void Update();

    // 콜백 등록
    using ReloadCallback = std::function<void(const std::string& path, RHIShader* newShader)>;
    void SetReloadCallback(ReloadCallback callback);

private:
    struct WatchEntry {
        std::string path;
        RHIShader* shader;
        std::filesystem::file_time_type lastModified;
    };

    std::vector<WatchEntry> _watchList;
    ReloadCallback _callback;
    float _checkInterval = 0.5f;  // 0.5초마다 체크
    float _timeSinceLastCheck = 0.0f;
};
```

### 구현

```cpp
void ShaderWatcher::Update()
{
    _timeSinceLastCheck += deltaTime;
    if (_timeSinceLastCheck < _checkInterval)
        return;

    _timeSinceLastCheck = 0.0f;

    for (auto& entry : _watchList)
    {
        auto currentTime = std::filesystem::last_write_time(entry.path);

        if (currentTime != entry.lastModified)
        {
            entry.lastModified = currentTime;

            // 셰이더 재컴파일
            RHIShader* newShader = RecompileShader(entry.path, entry.shader->info.stage);

            if (newShader)
            {
                // 기존 셰이더 교체
                if (_callback)
                    _callback(entry.path, newShader);

                entry.shader = newShader;
                Log::Info("Shader reloaded: %s", entry.path.c_str());
            }
            else
            {
                Log::Error("Shader reload failed: %s", entry.path.c_str());
            }
        }
    }
}
```

### 파이프라인 재생성

```cpp
// Application에서 콜백 등록
_shaderWatcher.SetReloadCallback([this](const std::string& path, RHIShader* newShader) {
    // 해당 셰이더를 사용하는 파이프라인 찾아서 재생성
    for (auto& pipeline : _activePipelines)
    {
        if (pipeline.UsesShader(path))
        {
            pipeline.Recreate(newShader);
        }
    }
});
```

---

## 구현 단계

### Phase 1: 기반 구조
1. nlohmann/json 라이브러리 추가
2. Application 클래스 생성 (Window, Camera, Renderer 통합)
3. Scene 클래스 및 JSON 로더 구현
4. SceneObject (Transform + AABB) 구현

### Phase 2: 프로파일링
5. ImPlot 라이브러리 통합
6. Profiler 클래스 구현 (CPU 타이밍)
7. GPU Query 래퍼 및 GPU 타이밍 구현

### Phase 3: 오브젝트 조작
8. ImGuizmo 라이브러리 통합
9. GizmoController 구현
10. MousePicker 구현 (Ray-AABB 피킹)

### Phase 4: 셰이더 시스템
11. ShaderWatcher 구현 (파일 감시)
12. 핫 리로드 시 파이프라인 재생성 로직

### Phase 5: 샘플 & 테스트
13. 샘플 씬 JSON 작성
14. 첫 번째 샘플 (Forward Rendering) 테스트
15. 문서화

---

## 수정할 파일 목록

### 새로 생성
| 파일 | 설명 |
|------|------|
| `Source/Application/PrototypeApplication.h/cpp` | 메인 애플리케이션 클래스 |
| `Source/Application/Scene.h/cpp` | JSON 씬 로더 |
| `Source/Application/SceneObject.h/cpp` | Transform + Bounds |
| `Source/Application/Camera.h/cpp` | 카메라 (EditorCamera 기반) |
| `Source/Application/MousePicker.h/cpp` | Ray casting 피킹 |
| `Source/Application/GizmoController.h/cpp` | ImGuizmo 래퍼 |
| `Source/Application/ShaderWatcher.h/cpp` | 셰이더 핫 리로드 |
| `Source/Profiler/Profiler.h/cpp` | CPU/GPU 프로파일러 |
| `Source/Profiler/GPUQuery.h/cpp` | RHI GPU 쿼리 래퍼 |

### 외부 라이브러리 추가
| 파일 | 위치 |
|------|------|
| `json.hpp` | `Dependency/include/` (이미 존재) |
| `implot.*` | `Source/ThirdParty/ImPlot/` |
| `ImGuizmo.*` | `Source/ThirdParty/ImGuizmo/` |

### CMake 수정
| 파일 | 변경 |
|------|------|
| `CMakeLists.txt` | Application, Profiler 모듈 추가, ThirdParty 경로 추가 |
| `Source/Application/CMakeLists.txt` | 새 모듈 빌드 설정 |
| `Source/Profiler/CMakeLists.txt` | 새 모듈 빌드 설정 |
| `Source/ThirdParty/CMakeLists.txt` | ImPlot, ImGuizmo 경로 설정 |

---

## 검증 방법

1. **빌드 테스트**: `cmake --build Build --config Debug`
2. **씬 로드 테스트**: 샘플 JSON 로드 후 모델 렌더링 확인
3. **프로파일러 테스트**: GPU/CPU 타이밍이 ImPlot 그래프에 표시되는지 확인
4. **마우스 피킹 테스트**: 3D 뷰에서 오브젝트 클릭 → 선택 확인
5. **Gizmo 테스트**: 오브젝트 선택 후 W/E/R 키로 Transform 조작 확인
6. **핫 리로드 테스트**: 셰이더 파일 수정 → 자동 재컴파일 및 화면 반영 확인
