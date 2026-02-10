# HSMR 에디터 기능 확장 계획

## 개요

5가지 핵심 기능을 구현하여 에디터를 실용적인 수준으로 발전시킵니다.

| # | 기능 | 설명 | 우선순위 | 상태 |
|---|------|------|---------|------|
| 1 | 리소스 시스템 연동 | Asset Browser + Drag & Drop | 높음 | ✅ 완료 |
| 2 | 씬 직렬화 | JSON 저장/로드 | 높음 | ✅ 완료 |
| 3 | Gizmo 개선 | Light/Camera/Bounds 시각화 | 중간 | 대기 |
| 4 | 프리미티브 생성 | Cube/Sphere/Plane 생성 메뉴 | 중간 | 대기 |
| 5 | **User Project System** | 프로젝트 분리 + 런처 | 높음 | 계획 중 |

---

## 5. User Project System (신규)

### 목표
- 엔진(HSMR.exe)과 유저 프로젝트를 완전 분리
- Unity/Unreal처럼 프로젝트 폴더를 열어서 작업하는 워크플로우
- 프로젝트 로드 전 런처(Hub) GUI 제공

### 현재 상태
- `SystemContext`: executablePath, assetDirectory 등 경로 관리
- `AssetDatabase`: rootPath 기준으로 에셋 스캔 (이미 분리 가능한 구조)
- `EditorApplication::Run()`: 바로 EditorWindow 생성 후 메인 루프 진입

### 아키텍처 변경

```
현재 흐름:
┌─────────────────────────────────────────────────────────────┐
│  main() → EditorApplication → EditorWindow → 에디터 루프     │
└─────────────────────────────────────────────────────────────┘

변경 후 흐름:
┌─────────────────────────────────────────────────────────────┐
│  main() → EditorApplication                                  │
│              │                                               │
│              ├─→ (프로젝트 미선택) → ProjectLauncher         │
│              │                         ├─ 최근 프로젝트 목록  │
│              │                         ├─ 새 프로젝트 생성    │
│              │                         └─ 프로젝트 열기       │
│              │                                ↓              │
│              └─→ (프로젝트 선택됨) → ProjectContext 로드      │
│                                         ↓                   │
│                                    EditorWindow              │
│                                    (기존 에디터 UI)          │
└─────────────────────────────────────────────────────────────┘
```

### 구현 계획

#### 5-1. 프로젝트 파일 형식 (.hsproj)

```json
// MyGame.hsproj
{
    "version": "1.0",
    "name": "MyGame",
    "engineVersion": "0.1.0",
    "settings": {
        "defaultScene": "Scenes/MainScene.scene",
        "buildTarget": "Windows",
        "renderAPI": "Vulkan"
    },
    "directories": {
        "assets": "Assets",
        "scenes": "Scenes",
        "scripts": "Scripts"
    }
}
```

**프로젝트 폴더 구조:**
```
MyGame/
├── MyGame.hsproj           # 프로젝트 설정 파일
├── Assets/                 # 에셋 폴더
│   ├── Models/
│   ├── Textures/
│   └── Materials/
├── Scenes/                 # 씬 파일들
│   └── MainScene.scene
├── Scripts/                # (미래) 스크립트 폴더
└── ProjectSettings/        # 에디터 설정
    ├── EditorLayout.ini
    └── RecentFiles.json
```

#### 5-2. ProjectContext 클래스 (신규)

```cpp
// Source/Editor/Project/ProjectContext.h

struct ProjectSettings
{
    std::string name;
    std::string engineVersion;
    std::string defaultScene;
    std::string buildTarget;
    std::string renderAPI;
};

class ProjectContext
{
public:
    static ProjectContext& Get();

    // 프로젝트 관리
    bool CreateProject(const std::string& path, const std::string& name);
    bool OpenProject(const std::string& projectFilePath);
    void CloseProject();

    bool IsProjectOpen() const { return _isOpen; }

    // 경로 접근자
    const std::string& GetProjectPath() const { return _projectPath; }
    const std::string& GetProjectName() const { return _settings.name; }

    std::string GetAssetPath() const;
    std::string GetScenePath() const;
    std::string GetSettingsPath() const;

    // 설정
    const ProjectSettings& GetSettings() const { return _settings; }
    void SaveSettings();

private:
    bool _isOpen = false;
    std::string _projectPath;       // 프로젝트 루트 폴더
    std::string _projectFilePath;   // .hsproj 파일 경로
    ProjectSettings _settings;

    bool loadProjectFile(const std::string& path);
    bool saveProjectFile();
    void createDefaultDirectories();
};
```

#### 5-3. RecentProjects 관리 (신규)

```cpp
// Source/Editor/Project/RecentProjects.h

struct RecentProjectEntry
{
    std::string name;
    std::string path;
    uint64 lastOpened;      // Unix timestamp
    bool exists = true;     // 파일 존재 여부
};

class RecentProjects
{
public:
    static RecentProjects& Get();

    void Load();
    void Save();

    void AddProject(const std::string& projectPath);
    void RemoveProject(const std::string& projectPath);

    const std::vector<RecentProjectEntry>& GetProjects() const;

    void ValidateProjects();    // 존재하지 않는 프로젝트 마킹

private:
    std::vector<RecentProjectEntry> _projects;
    std::string getConfigPath() const;  // AppData/HSMR/RecentProjects.json
};
```

#### 5-4. ProjectLauncherWindow 클래스 (신규)

```cpp
// Source/Editor/Project/ProjectLauncherWindow.h

class ProjectLauncherWindow : public Window
{
public:
    ProjectLauncherWindow(Application* ownerApp);
    ~ProjectLauncherWindow() override;

    // 프로젝트 선택 결과
    bool HasSelectedProject() const { return !_selectedProjectPath.empty(); }
    const std::string& GetSelectedProjectPath() const { return _selectedProjectPath; }

private:
    bool onInitialize() override;
    void onNextFrame() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onPresent() override;
    void onShutdown() override;

    void drawLauncherUI();
    void drawProjectList();
    void drawNewProjectDialog();
    void drawOpenProjectDialog();

    std::string _selectedProjectPath;

    // UI 상태
    bool _showNewProjectDialog = false;
    char _newProjectName[256] = {0};
    char _newProjectPath[512] = {0};
};
```

#### 5-5. ProjectLauncherWindow UI 레이아웃

```cpp
// ProjectLauncherWindow.cpp

void ProjectLauncherWindow::drawLauncherUI()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("HSMR Project Launcher", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    // 좌측: 액션 버튼들
    ImGui::BeginChild("Actions", ImVec2(200, 0), true);
    {
        ImGui::Text("HSMR Engine");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("New Project", ImVec2(-1, 40)))
        {
            _showNewProjectDialog = true;
        }

        if (ImGui::Button("Open Project...", ImVec2(-1, 40)))
        {
            openProjectDialog();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 버전 정보
        ImGui::TextDisabled("Version 0.1.0");
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // 우측: 최근 프로젝트 목록
    ImGui::BeginChild("Recent Projects", ImVec2(0, 0), true);
    {
        ImGui::Text("Recent Projects");
        ImGui::Separator();
        ImGui::Spacing();

        drawProjectList();
    }
    ImGui::EndChild();

    ImGui::End();

    // 다이얼로그들
    if (_showNewProjectDialog)
    {
        drawNewProjectDialog();
    }
}

void ProjectLauncherWindow::drawProjectList()
{
    auto& recentProjects = RecentProjects::Get().GetProjects();

    for (const auto& project : recentProjects)
    {
        ImGui::PushID(project.path.c_str());

        // 프로젝트 카드
        ImVec2 cardSize(ImGui::GetContentRegionAvail().x, 60);
        bool clicked = ImGui::Selectable("##card", false,
                                         ImGuiSelectableFlags_None, cardSize);

        // 카드 내용
        ImGui::SameLine();
        ImGui::BeginGroup();
        {
            ImGui::Text("%s", project.name.c_str());

            if (project.exists)
            {
                ImGui::TextDisabled("%s", project.path.c_str());
            }
            else
            {
                ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "(Not Found)");
            }
        }
        ImGui::EndGroup();

        if (clicked && project.exists)
        {
            _selectedProjectPath = project.path;
        }

        // 우클릭 메뉴
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Open in Explorer"))
            {
                // 탐색기에서 열기
            }
            if (ImGui::MenuItem("Remove from List"))
            {
                RecentProjects::Get().RemoveProject(project.path);
            }
            ImGui::EndPopup();
        }

        ImGui::PopID();
    }

    if (recentProjects.empty())
    {
        ImGui::TextDisabled("No recent projects");
        ImGui::TextDisabled("Create a new project or open an existing one.");
    }
}

void ProjectLauncherWindow::drawNewProjectDialog()
{
    ImGui::OpenPopup("New Project");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 200));

    if (ImGui::BeginPopupModal("New Project", &_showNewProjectDialog))
    {
        ImGui::Text("Project Name:");
        ImGui::InputText("##name", _newProjectName, sizeof(_newProjectName));

        ImGui::Spacing();

        ImGui::Text("Location:");
        ImGui::InputText("##path", _newProjectPath, sizeof(_newProjectPath));
        ImGui::SameLine();
        if (ImGui::Button("Browse..."))
        {
            // 폴더 선택 다이얼로그
            selectFolderDialog(_newProjectPath, sizeof(_newProjectPath));
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            std::string projectPath = std::string(_newProjectPath) + "/" + _newProjectName;
            if (ProjectContext::Get().CreateProject(projectPath, _newProjectName))
            {
                _selectedProjectPath = projectPath + "/" + _newProjectName + ".hsproj";
                _showNewProjectDialog = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            _showNewProjectDialog = false;
        }

        ImGui::EndPopup();
    }
}
```

#### 5-6. EditorApplication 수정

```cpp
// EditorApplication.cpp 수정

void EditorApplication::Run()
{
    Timer::Start();

    // Phase 1: 프로젝트 선택 (런처)
    std::string projectPath = getProjectFromCommandLine();

    if (projectPath.empty())
    {
        // 런처 윈도우 표시
        _launcherWindow = new ProjectLauncherWindow(this);

        while (_launcherWindow->IsOpened())
        {
            _launcherWindow->ProcessEvent();
            _launcherWindow->NextFrame();
            _launcherWindow->Update(0.016f);
            _launcherWindow->Render();
            _launcherWindow->Present();
            _launcherWindow->Flush();

            // 프로젝트 선택됨
            if (_launcherWindow->HasSelectedProject())
            {
                projectPath = _launcherWindow->GetSelectedProjectPath();
                break;
            }
        }

        _launcherWindow->Shutdown();
        delete _launcherWindow;
        _launcherWindow = nullptr;

        if (projectPath.empty())
        {
            // 사용자가 런처를 닫음 - 종료
            return;
        }
    }

    // Phase 2: 프로젝트 로드
    if (!ProjectContext::Get().OpenProject(projectPath))
    {
        HS_LOG(error, "Failed to open project: {}", projectPath);
        return;
    }

    // AssetDatabase 초기화 (프로젝트 에셋 폴더 기준)
    AssetDatabase::Get().Initialize(ProjectContext::Get().GetAssetPath());

    // Phase 3: 에디터 윈도우 (기존 코드)
    _window = new EditorWindow(this, "HSMR Editor", 1920, 1080, windowFlags);
    // ... 기존 메인 루프 ...
}

std::string EditorApplication::getProjectFromCommandLine()
{
    // 커맨드 라인에서 프로젝트 경로 파싱
    // 예: HSMR.exe --project "C:/Projects/MyGame/MyGame.hsproj"
    // 또는: HSMR.exe "C:/Projects/MyGame/MyGame.hsproj"

    auto* args = GetCommandLineArgs();
    if (args && args->count > 1)
    {
        std::string arg = args->values[1];
        if (arg.ends_with(".hsproj"))
        {
            return arg;
        }
    }
    return "";
}
```

#### 5-7. SystemContext 확장

```cpp
// SystemContext.h 수정

struct HS_API SystemContext
{
public:
    // 기존 필드들
    std::string executablePath      = "";
    std::string executableDirectory = "";
    std::string assetDirectory      = "";

    // 신규 필드들 (프로젝트 시스템용)
    std::string appDataDirectory    = "";   // %APPDATA%/HSMR/ 또는 ~/Library/HSMR/
    std::string userDocumentsDir    = "";   // 기본 프로젝트 생성 위치

    static SystemContext* Get();
    static bool Init();

private:
    bool initializePlatform();
    void finalizePlatform();

    void initAppDataDirectory();
};
```

### 파일 구조

```
Source/Editor/
├── Project/                          (신규 폴더)
│   ├── ProjectContext.h              (신규)
│   ├── ProjectContext.cpp            (신규)
│   ├── RecentProjects.h              (신규)
│   ├── RecentProjects.cpp            (신규)
│   ├── ProjectLauncherWindow.h       (신규)
│   └── ProjectLauncherWindow.cpp     (신규)
├── Core/
│   ├── EditorApplication.h           (수정: 런처 통합)
│   └── EditorApplication.cpp         (수정: 2단계 실행)
├── Asset/
│   └── AssetDatabase.cpp             (수정: ProjectContext 연동)
└── Panel/
    └── MenuPanel.cpp                 (수정: 프로젝트 메뉴 추가)

Source/Core/
├── SystemContext.h                   (수정: AppData 경로 추가)
└── Private/
    └── SystemContext.cpp             (수정: 플랫폼별 경로 초기화)
```

### 작업 항목

| # | 작업 | 예상 시간 | 의존성 |
|---|------|----------|--------|
| 5-1 | SystemContext AppData 경로 확장 | 30분 | - |
| 5-2 | ProjectContext 클래스 구현 | 1시간 | 5-1 |
| 5-3 | .hsproj 파일 읽기/쓰기 | 30분 | 5-2 |
| 5-4 | RecentProjects 클래스 구현 | 45분 | 5-1 |
| 5-5 | ProjectLauncherWindow 기본 구조 | 1시간 | - |
| 5-6 | 런처 UI (프로젝트 리스트) | 1시간 | 5-4, 5-5 |
| 5-7 | 새 프로젝트 생성 다이얼로그 | 45분 | 5-2, 5-6 |
| 5-8 | 프로젝트 열기 다이얼로그 | 30분 | 5-6 |
| 5-9 | EditorApplication 수정 (2단계 실행) | 1시간 | 5-2, 5-5 |
| 5-10 | 커맨드라인 프로젝트 로드 | 30분 | 5-9 |
| 5-11 | AssetDatabase 프로젝트 연동 | 30분 | 5-2 |
| 5-12 | MenuPanel 프로젝트 메뉴 추가 | 30분 | 5-2 |
| 5-13 | 빌드 및 테스트 | 1시간 | 전체 |

**예상 총 작업 시간: 약 9시간**

### 워크플로우 시나리오

#### 시나리오 1: 첫 실행 (프로젝트 없음)
```
1. HSMR.exe 실행
2. ProjectLauncherWindow 표시
3. "New Project" 클릭
4. 이름: "MyGame", 위치: "D:/Projects" 입력
5. "Create" 클릭
6. D:/Projects/MyGame/ 폴더 생성
7. MyGame.hsproj 생성
8. Assets/, Scenes/ 폴더 생성
9. 에디터 윈도우로 전환
10. AssetDatabase가 D:/Projects/MyGame/Assets/ 스캔
```

#### 시나리오 2: 최근 프로젝트 열기
```
1. HSMR.exe 실행
2. ProjectLauncherWindow 표시
3. "MyGame" 프로젝트 카드 클릭
4. 프로젝트 로드
5. 에디터 윈도우로 전환
```

#### 시나리오 3: 커맨드라인 실행
```
# 배치 파일 또는 바로가기
HSMR.exe "D:/Projects/MyGame/MyGame.hsproj"

→ 런처 스킵, 바로 에디터 진입
```

#### 시나리오 4: .hsproj 더블클릭 (Windows 파일 연결)
```
1. 탐색기에서 MyGame.hsproj 더블클릭
2. Windows가 HSMR.exe에 경로 전달
3. 런처 스킵, 바로 해당 프로젝트 열림
```

### 미래 확장 가능성

1. **Project Templates**
   - Empty Project
   - 3D Game Template
   - 2D Game Template

2. **Multi-Window Support**
   - 런처에서 여러 프로젝트를 다른 창으로 열기

3. **Version Management**
   - 엔진 버전별 프로젝트 호환성 체크
   - 프로젝트 업그레이드 마이그레이션

4. **Cloud Integration**
   - GitHub/GitLab 연동
   - 클라우드 프로젝트 동기화

---

## 1. 리소스 시스템 연동

### 목표
- Asset Browser Panel에서 프로젝트 리소스 탐색
- Inspector에서 Mesh/Material 필드에 Drag & Drop 할당

### 현재 상태
- `ObjectManager`: GLTF/Image 로딩 완전 지원
- `ResourcePanel`: 빈 껍데기만 존재
- `MeshRendererComponent`: mesh*, materials[] 포인터로 참조

### 구현 계획

#### 1-1. AssetDatabase 클래스 (신규)

```cpp
// Source/Editor/Asset/AssetDatabase.h

struct AssetEntry
{
    std::string path;           // 상대 경로 (Assets/ 기준)
    std::string name;           // 파일명
    EAssetType type;            // MESH, MATERIAL, TEXTURE, MODEL, UNKNOWN
    uint64 lastModified;        // 파일 수정 시간
    bool isLoaded = false;      // 메모리 로드 여부
    void* cachedResource = nullptr;  // 캐시된 리소스 포인터
};

class AssetDatabase
{
public:
    static AssetDatabase& Get();

    void Scan(const std::string& rootPath);   // Assets 폴더 스캔
    void Refresh();                            // 변경사항 갱신

    const std::vector<AssetEntry>& GetAssets(EAssetType type) const;
    AssetEntry* FindByPath(const std::string& path);

    // 리소스 로딩 (지연 로딩)
    Mesh* LoadMesh(const std::string& path);
    Material* LoadMaterial(const std::string& path);
    Image* LoadTexture(const std::string& path);

private:
    std::unordered_map<std::string, AssetEntry> _assets;
    std::string _rootPath;
};
```

#### 1-2. ResourcePanel 구현

```cpp
// Source/Editor/Panel/ResourcePanel.cpp

void ResourcePanel::Draw()
{
    ImGui::Begin("Assets");

    // 경로 탐색 바
    drawPathBar();

    // 폴더 트리뷰 (좌측)
    if (ImGui::BeginChild("FolderTree", ImVec2(200, 0), true))
    {
        drawFolderTree(_currentPath);
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // 에셋 그리드/리스트 (우측)
    if (ImGui::BeginChild("AssetView", ImVec2(0, 0), true))
    {
        for (auto& asset : getAssetsInFolder(_currentPath))
        {
            drawAssetThumbnail(asset);

            // Drag Source 설정
            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("ASSET_PATH", asset.path.c_str(),
                                          asset.path.size() + 1);
                ImGui::Text("%s", asset.name.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }
    ImGui::EndChild();

    ImGui::End();
}
```

#### 1-3. InspectorPanel Drag & Drop

```cpp
// InspectorPanel::drawMeshRendererComponent() 수정

void InspectorPanel::drawMeshField(MeshRendererComponent& renderer)
{
    const char* meshName = renderer.mesh ? renderer.mesh->GetName() : "None";

    ImGui::Button(meshName, ImVec2(-1, 0));

    // Drop Target
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
        {
            std::string path((const char*)payload->Data);
            if (isMeshFile(path))
            {
                renderer.mesh = AssetDatabase::Get().LoadMesh(path);
            }
        }
        ImGui::EndDragDropTarget();
    }
}
```

### 파일 구조

```
Source/Editor/
├── Asset/
│   ├── AssetDatabase.h       (신규)
│   ├── AssetDatabase.cpp     (신규)
│   └── AssetTypes.h          (신규: EAssetType enum)
├── Panel/
│   ├── ResourcePanel.h       (수정)
│   └── ResourcePanel.cpp     (수정)
```

### 작업 항목

| # | 작업 | 예상 시간 |
|---|------|----------|
| 1-1 | AssetTypes.h 정의 | 15분 |
| 1-2 | AssetDatabase 구현 | 1시간 |
| 1-3 | ResourcePanel 폴더 트리 | 1시간 |
| 1-4 | ResourcePanel 에셋 그리드 | 1시간 |
| 1-5 | Drag & Drop 소스 구현 | 30분 |
| 1-6 | InspectorPanel Drop 타겟 | 30분 |
| 1-7 | 빌드 및 테스트 | 30분 |

---

## 2. 씬 직렬화 (Scene Serialization)

### 목표
- ECS Scene을 JSON으로 저장/로드
- 에디터 메뉴에서 Save/Load Scene 지원

### 현재 상태
- `nlohmann/json` 라이브러리 사용 중
- Application 계층에 JSON 로드 코드 존재 (참고용)
- ECS Component들은 모두 POD 구조

### 구현 계획

#### 2-1. SceneSerializer 클래스

```cpp
// Source/Engine/Scene/SceneSerializer.h

class SceneSerializer
{
public:
    SceneSerializer(Scene* scene);

    // 직렬화
    bool SaveToFile(const std::string& path);
    std::string SaveToString();

    // 역직렬화
    bool LoadFromFile(const std::string& path);
    bool LoadFromString(const std::string& json);

private:
    Scene* _scene;

    // Component 직렬화 헬퍼
    json serializeEntity(Entity entity);
    json serializeTransform(const TransformComponent& t);
    json serializeTag(const TagComponent& t);
    json serializeMeshRenderer(const MeshRendererComponent& mr);
    json serializeCamera(const CameraComponent& c);
    json serializeLight(const LightComponent& l);

    // Component 역직렬화 헬퍼
    void deserializeEntity(Entity entity, const json& j);
    void deserializeTransform(TransformComponent& t, const json& j);
    // ... 등등
};
```

#### 2-2. JSON 스키마

```json
{
    "version": "1.0",
    "name": "Test Scene",
    "entities": [
        {
            "id": 1,
            "tag": {
                "name": "Main Camera",
                "layer": 0,
                "isStatic": false,
                "isActive": true
            },
            "transform": {
                "position": [0, 2, 5],
                "rotation": [0, 0, 0, 1],
                "scale": [1, 1, 1],
                "parent": null,
                "children": []
            },
            "camera": {
                "projectionType": "Perspective",
                "fov": 60,
                "nearPlane": 0.1,
                "farPlane": 1000,
                "isPrimary": true
            }
        },
        {
            "id": 2,
            "tag": { "name": "Cube" },
            "transform": {
                "position": [0, 0, 0],
                "rotation": [0, 0, 0, 1],
                "scale": [1, 1, 1]
            },
            "meshRenderer": {
                "mesh": "Assets/GLTF/Cube/Cube.gltf",
                "materials": ["Assets/Materials/Default.mat"],
                "castShadow": true,
                "receiveShadow": true,
                "isVisible": true
            }
        }
    ]
}
```

#### 2-3. 에디터 메뉴 통합

```cpp
// MenuPanel.cpp 수정

void MenuPanel::drawFileMenu()
{
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New Scene", "Ctrl+N"))
        {
            createNewScene();
        }

        if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
        {
            openSceneDialog();
        }

        if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
        {
            saveCurrentScene();
        }

        if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
        {
            saveSceneAsDialog();
        }

        ImGui::EndMenu();
    }
}
```

### 파일 구조

```
Source/Engine/Scene/
├── SceneSerializer.h     (신규)
└── Private/
    └── SceneSerializer.cpp   (신규)

Source/Editor/Panel/
└── MenuPanel.cpp         (수정: File 메뉴 추가)
```

### 작업 항목

| # | 작업 | 예상 시간 |
|---|------|----------|
| 2-1 | SceneSerializer 헤더 정의 | 20분 |
| 2-2 | SaveToFile 구현 | 1시간 |
| 2-3 | LoadFromFile 구현 | 1시간 |
| 2-4 | 계층 구조 (parent/children) 처리 | 30분 |
| 2-5 | MenuPanel File 메뉴 | 30분 |
| 2-6 | 파일 다이얼로그 (nfd 또는 ImGui) | 30분 |
| 2-7 | 빌드 및 테스트 | 30분 |

---

## 3. Gizmo 개선

### 목표
- Light Gizmo: 방향/범위 시각화
- Camera Gizmo: 프러스텀 시각화
- Bounds Gizmo: 선택된 엔티티의 AABB 와이어프레임

### 현재 상태
- TransformGizmo: ImGuizmo 사용 (완성)
- ViewGizmo: ImGui DrawList 커스텀 (완성)
- Light/Camera/Bounds: 없음

### 구현 계획

#### 3-1. GizmoRenderer 클래스

```cpp
// Source/Editor/Gizmo/GizmoRenderer.h

class GizmoRenderer
{
public:
    GizmoRenderer();

    void Begin(const glm::mat4& view, const glm::mat4& proj,
               ImVec2 viewportMin, ImVec2 viewportSize);
    void End();

    // 3D 와이어프레임 그리기
    void DrawLine(const glm::vec3& from, const glm::vec3& to, ImU32 color);
    void DrawWireBox(const AABB& bounds, ImU32 color);
    void DrawWireSphere(const glm::vec3& center, float radius, ImU32 color);
    void DrawWireCone(const glm::vec3& apex, const glm::vec3& dir,
                      float height, float angle, ImU32 color);
    void DrawFrustum(const glm::mat4& invViewProj, ImU32 color);
    void DrawArrow(const glm::vec3& from, const glm::vec3& to, ImU32 color);

private:
    ImDrawList* _drawList;
    glm::mat4 _viewProj;
    ImVec2 _viewportMin;
    ImVec2 _viewportSize;

    // 3D → 2D 투영
    ImVec2 worldToScreen(const glm::vec3& worldPos);
    bool isInFront(const glm::vec3& worldPos);
};
```

#### 3-2. 컴포넌트별 Gizmo

```cpp
// ScenePanel.cpp 수정

void ScenePanel::drawComponentGizmos()
{
    Scene* scene = EditorContext::Get().GetActiveScene();
    Entity selected = EditorContext::Get().GetSelectedEntity();

    _gizmoRenderer.Begin(_camera->GetViewMatrix(),
                         _camera->GetProjectionMatrix(),
                         _viewportMin,
                         ImVec2(_resolution.width, _resolution.height));

    // Bounds Gizmo (선택된 엔티티)
    if (selected.IsValid() && selected.HasComponent<MeshRendererComponent>())
    {
        auto& mr = selected.GetComponent<MeshRendererComponent>();
        _gizmoRenderer.DrawWireBox(mr.worldBounds, IM_COL32(0, 255, 0, 180));
    }

    // Light Gizmos (모든 라이트)
    auto lightView = scene->View<TransformComponent, LightComponent>();
    for (auto entity : lightView)
    {
        auto& transform = scene->GetEntity(entity).GetComponent<TransformComponent>();
        auto& light = scene->GetEntity(entity).GetComponent<LightComponent>();

        drawLightGizmo(transform, light);
    }

    // Camera Gizmos (비활성 카메라만)
    auto camView = scene->View<TransformComponent, CameraComponent>();
    for (auto entity : camView)
    {
        auto& cam = scene->GetEntity(entity).GetComponent<CameraComponent>();
        if (!cam.isPrimary)
        {
            auto& transform = scene->GetEntity(entity).GetComponent<TransformComponent>();
            drawCameraGizmo(transform, cam);
        }
    }

    _gizmoRenderer.End();
}

void ScenePanel::drawLightGizmo(const TransformComponent& t, const LightComponent& l)
{
    glm::vec3 pos = t.GetWorldPosition();
    ImU32 color = IM_COL32(255, 230, 120, 200);

    switch (l.type)
    {
        case ELightType::Directional:
            // 방향 화살표
            glm::vec3 dir = t.GetForward();
            _gizmoRenderer.DrawArrow(pos, pos + dir * 2.0f, color);
            break;

        case ELightType::Point:
            // 범위 구체
            _gizmoRenderer.DrawWireSphere(pos, l.range, color);
            break;

        case ELightType::Spot:
            // 원뿔
            glm::vec3 dir = t.GetForward();
            _gizmoRenderer.DrawWireCone(pos, dir, l.range, l.outerConeAngle, color);
            break;
    }
}

void ScenePanel::drawCameraGizmo(const TransformComponent& t, const CameraComponent& c)
{
    // 프러스텀 와이어프레임
    glm::mat4 viewProj = c.GetProjectionMatrix() * glm::inverse(t.worldMatrix);
    glm::mat4 invViewProj = glm::inverse(viewProj);

    _gizmoRenderer.DrawFrustum(invViewProj, IM_COL32(150, 150, 255, 180));
}
```

### 파일 구조

```
Source/Editor/
├── Gizmo/
│   ├── GizmoRenderer.h       (신규)
│   └── GizmoRenderer.cpp     (신규)
├── Panel/
│   └── ScenePanel.cpp        (수정: drawComponentGizmos 추가)
```

### 작업 항목

| # | 작업 | 예상 시간 |
|---|------|----------|
| 3-1 | GizmoRenderer 기본 구조 | 30분 |
| 3-2 | worldToScreen 투영 | 20분 |
| 3-3 | DrawLine, DrawWireBox | 30분 |
| 3-4 | DrawWireSphere | 30분 |
| 3-5 | DrawWireCone (Spot light) | 30분 |
| 3-6 | DrawFrustum (Camera) | 30분 |
| 3-7 | ScenePanel 통합 | 30분 |
| 3-8 | 빌드 및 테스트 | 30분 |

---

## 4. 프리미티브 생성

### 목표
- Hierarchy 패널에서 우클릭 → Create 메뉴
- Cube, Sphere, Plane, Cylinder 등 기본 메시 생성
- Empty Entity, Light, Camera 생성도 포함

### 현재 상태
- `ObjectManager`: GetFallbackMeshCube/Sphere/Plane 제공
- `HierarchyPanel`: 우클릭 컨텍스트 메뉴 없음

### 구현 계획

#### 4-1. PrimitiveFactory 클래스

```cpp
// Source/Editor/Factory/PrimitiveFactory.h

class PrimitiveFactory
{
public:
    // 프리미티브 메시를 가진 엔티티 생성
    static Entity CreateCube(Scene* scene, const std::string& name = "Cube");
    static Entity CreateSphere(Scene* scene, const std::string& name = "Sphere");
    static Entity CreatePlane(Scene* scene, const std::string& name = "Plane");
    static Entity CreateCylinder(Scene* scene, const std::string& name = "Cylinder");

    // 컴포넌트 엔티티 생성
    static Entity CreateEmpty(Scene* scene, const std::string& name = "Empty");
    static Entity CreateCamera(Scene* scene, const std::string& name = "Camera");
    static Entity CreateDirectionalLight(Scene* scene, const std::string& name = "Directional Light");
    static Entity CreatePointLight(Scene* scene, const std::string& name = "Point Light");
    static Entity CreateSpotLight(Scene* scene, const std::string& name = "Spot Light");

private:
    static Material* getDefaultMaterial();
};
```

#### 4-2. HierarchyPanel 컨텍스트 메뉴

```cpp
// HierarchyPanel.cpp 수정

void HierarchyPanel::drawContextMenu()
{
    if (ImGui::BeginPopupContextWindow("HierarchyContext"))
    {
        Scene* scene = EditorContext::Get().GetActiveScene();

        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Empty"))
            {
                Entity e = PrimitiveFactory::CreateEmpty(scene);
                EditorContext::Get().SetSelectedEntity(e);
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("3D Object"))
            {
                if (ImGui::MenuItem("Cube"))
                {
                    Entity e = PrimitiveFactory::CreateCube(scene);
                    EditorContext::Get().SetSelectedEntity(e);
                }
                if (ImGui::MenuItem("Sphere"))
                {
                    Entity e = PrimitiveFactory::CreateSphere(scene);
                    EditorContext::Get().SetSelectedEntity(e);
                }
                if (ImGui::MenuItem("Plane"))
                {
                    Entity e = PrimitiveFactory::CreatePlane(scene);
                    EditorContext::Get().SetSelectedEntity(e);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Light"))
            {
                if (ImGui::MenuItem("Directional Light"))
                {
                    Entity e = PrimitiveFactory::CreateDirectionalLight(scene);
                    EditorContext::Get().SetSelectedEntity(e);
                }
                if (ImGui::MenuItem("Point Light"))
                {
                    Entity e = PrimitiveFactory::CreatePointLight(scene);
                    EditorContext::Get().SetSelectedEntity(e);
                }
                if (ImGui::MenuItem("Spot Light"))
                {
                    Entity e = PrimitiveFactory::CreateSpotLight(scene);
                    EditorContext::Get().SetSelectedEntity(e);
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Camera"))
            {
                Entity e = PrimitiveFactory::CreateCamera(scene);
                EditorContext::Get().SetSelectedEntity(e);
            }

            ImGui::EndMenu();
        }

        // 선택된 엔티티가 있을 때만 표시
        Entity selected = EditorContext::Get().GetSelectedEntity();
        if (selected.IsValid())
        {
            ImGui::Separator();

            if (ImGui::MenuItem("Duplicate", "Ctrl+D"))
            {
                duplicateEntity(selected);
            }

            if (ImGui::MenuItem("Delete", "Delete"))
            {
                scene->DestroyEntity(selected);
                EditorContext::Get().ClearSelection();
            }
        }

        ImGui::EndPopup();
    }
}
```

### 파일 구조

```
Source/Editor/
├── Factory/
│   ├── PrimitiveFactory.h    (신규)
│   └── PrimitiveFactory.cpp  (신규)
├── Panel/
│   └── HierarchyPanel.cpp    (수정: 컨텍스트 메뉴)
```

### 작업 항목

| # | 작업 | 예상 시간 |
|---|------|----------|
| 4-1 | PrimitiveFactory 헤더 | 15분 |
| 4-2 | CreateCube/Sphere/Plane | 30분 |
| 4-3 | CreateLight (3종) | 20분 |
| 4-4 | CreateCamera | 10분 |
| 4-5 | HierarchyPanel 컨텍스트 메뉴 | 30분 |
| 4-6 | Duplicate 기능 | 30분 |
| 4-7 | Delete 기능 | 10분 |
| 4-8 | 빌드 및 테스트 | 20분 |

---

## 구현 순서 및 의존성

```
┌─────────────────────────────────────────────────────────────┐
│                       Phase 1                                │
│  ┌──────────────────┐     ┌──────────────────┐              │
│  │ 4. 프리미티브 생성│     │ 2. 씬 직렬화     │              │
│  │    (독립적)      │     │    (독립적)      │              │
│  └──────────────────┘     └──────────────────┘              │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                       Phase 2                                │
│  ┌──────────────────┐     ┌──────────────────┐              │
│  │ 1. 리소스 연동   │────▶│ 3. Gizmo 개선    │              │
│  │  (AssetDatabase) │     │ (Bounds 시각화)  │              │
│  └──────────────────┘     └──────────────────┘              │
└─────────────────────────────────────────────────────────────┘
```

### 권장 순서

1. **프리미티브 생성** (독립적, 가장 간단)
2. **씬 직렬화** (독립적, nlohmann/json 이미 있음)
3. **리소스 연동** (AssetDatabase 필요)
4. **Gizmo 개선** (리소스 연동과 함께 테스트하기 좋음)

---

## 예상 총 작업 시간

| 기능 | 예상 시간 |
|------|----------|
| 1. 리소스 시스템 연동 | 5시간 |
| 2. 씬 직렬화 | 4.5시간 |
| 3. Gizmo 개선 | 4시간 |
| 4. 프리미티브 생성 | 2.5시간 |
| **총합** | **약 16시간** |

---

## 신규 파일 목록

```
Source/Editor/
├── Asset/
│   ├── AssetDatabase.h
│   ├── AssetDatabase.cpp
│   └── AssetTypes.h
├── Factory/
│   ├── PrimitiveFactory.h
│   └── PrimitiveFactory.cpp
├── Gizmo/
│   ├── GizmoRenderer.h
│   └── GizmoRenderer.cpp

Source/Engine/Scene/
├── SceneSerializer.h
└── Private/
    └── SceneSerializer.cpp
```

## 수정 파일 목록

```
Source/Editor/Panel/
├── ResourcePanel.h
├── ResourcePanel.cpp
├── InspectorPanel.cpp
├── HierarchyPanel.cpp
├── MenuPanel.cpp
└── ScenePanel.cpp

Source/Engine/CMakeLists.txt
Source/Editor/CMakeLists.txt
```
