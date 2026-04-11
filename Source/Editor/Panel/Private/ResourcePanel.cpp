//
//  ResourcePanel.cpp
//  Editor
//
//  Asset browser panel implementation
//

#include "Editor/Panel/ResourcePanel.h"
#include "Editor/Asset/AssetDatabase.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/GUI/EditorIcons.h"
#include "Editor/GUI/EditorListWidgets.h"
#include "Editor/GUI/EditorTreeWidgets.h"
#include "Editor/Panel/EditorPanelFrame.h"
#include "Core/SystemContext.h"
#include "Editor/Project/ProjectContext.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

#include "Resource/Model.h"
#include "Resource/Material.h"
#include "Resource/Mesh.h"
#include "Resource/ObjectManager.h"

#include "ImGui/imgui.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

HS_NS_EDITOR_BEGIN

namespace
{
std::string buildUniqueMaterialRelativePathInFolder(const std::string& folderPath)
{
    const std::string prefix = folderPath.empty() ? "" : folderPath + "/";
    std::string relativePath = prefix + "New Material.mat";
    int suffix = 1;
    while (AssetDatabase::Get().FindAsset(relativePath))
    {
        relativePath = prefix + "New Material " + std::to_string(suffix++) + ".mat";
    }
    return relativePath;
}
}

ResourcePanel::ResourcePanel(Window* window)
    : Panel(window, "Assets")
{
}

ResourcePanel::~ResourcePanel()
{
}

bool ResourcePanel::Setup()
{
    // Initialize AssetDatabase with project's Assets folder
    hs::SystemContext* sysContext = hs::SystemContext::Get();
    if (sysContext && !sysContext->assetDirectory.empty())
    {
        AssetDatabase::Get().Initialize(sysContext->assetDirectory);
    }

    // Start at root
    navigateToFolder("");
    EditorContext::Get().SetCurrentAssetFolderPath(_currentPath);

    return true;
}

void ResourcePanel::Cleanup()
{
    AssetDatabase::Get().Shutdown();
}

void ResourcePanel::Draw()
{
    if (!IsVisible())
    {
        return;
    }

    _selectedAssetPath = EditorContext::Get().GetSelectedAssetPath();

    // ImGui는 immediate-mode GUI입니다. 매 프레임 "현재 상태를 보고 UI를 다시 선언"합니다.
    // 그래서 이 함수 안의 Begin/End, BeginChild/EndChild 호출 순서가 곧 이번 프레임의 패널 구조입니다.
    // 새 패널을 만들 때도 이 패턴을 따라가면 됩니다.
    //
    // 1. Panel visibility state를 읽어서 창을 열지 결정합니다.
    // 2. ImGui::Begin()으로 top-level window를 시작합니다.
    // 3. 내부를 toolbar/pathbar/content child/context menu 순서로 그립니다.
    // 4. 반드시 ImGui::End()로 닫습니다.
    //
    // 주의: BeginChild(), PushStyleVar(), PushID()처럼 stack을 쓰는 API는 반드시 대응되는 End/Pop이 있어야 합니다.
    // 예외적으로 ImGui::Begin()이 false를 반환하는 경우에도 End()는 호출해야 하지만, 이 패널은 반환값을 쓰지 않고
    // 항상 내용을 그리는 단순한 패턴을 사용합니다.
    EditorPanelWindowOptions panelOptions{};
    panelOptions.pOpen = GetVisibilityBinding();
    EditorPanelFrame::BeginStandardPanel("Assets", panelOptions);

    // Path bar at top
    drawPathBar();

    ImGui::Separator();

    // Main content area
    float contentWidth = ImGui::GetContentRegionAvail().x;

    // 이 패널은 좌측 FolderTree와 우측 AssetList를 나누는 전형적인 split-view 레이아웃입니다.
    // ImGui에는 별도의 splitter 위젯이 없으므로, 작은 Button을 splitter처럼 그리고 드래그 중일 때 폭을 직접 갱신합니다.
    //
    // 중요한 점:
    // - 좌측 트리가 항상 고정 폭을 차지하면 도킹 창이 좁아질 때 우측 리스트 폭이 0에 가까워질 수 있습니다.
    // - 그래서 AssetList가 최소 폭(minAssetListWidth)을 확보할 수 있을 때만 FolderTree를 표시합니다.
    // - 폴더 트리 폭은 매 프레임 clamp해서 사용자가 splitter를 과하게 당겨도 레이아웃이 깨지지 않게 합니다.
    //
    // 새 split-view 패널을 만들 때도 "좌측/상단 패널이 먼저 공간을 먹고, 주 콘텐츠가 남은 공간을 쓴다"는 점을
    // 항상 의식해야 합니다. 주 콘텐츠가 0폭이 되면 텍스트가 한 글자처럼 잘리는 현상이 쉽게 생깁니다.
    const float splitterWidth = 4.0f;
    const float minFolderTreeWidth = 120.0f;
    const float minAssetListWidth = 240.0f;
    const float itemSpacingX = ImGui::GetStyle().ItemSpacing.x;
    const bool showFolderTree = _showFolderTree &&
                                contentWidth > minFolderTreeWidth + minAssetListWidth + splitterWidth + itemSpacingX * 2.0f;

    if (showFolderTree)
    {
        const float maxFolderTreeWidth = std::max(
            minFolderTreeWidth,
            contentWidth - minAssetListWidth - splitterWidth - itemSpacingX * 2.0f);
        _folderTreeWidth = glm::clamp(_folderTreeWidth, minFolderTreeWidth, maxFolderTreeWidth);

        // Left: Folder tree
        //
        // BeginChild()는 현재 창 내부에 독립적인 scroll/clipping 영역을 만듭니다.
        // 여기서는 폴더 트리가 독립적으로 스크롤되고, 오른쪽 AssetList와 clipping 영역이 섞이지 않도록 child로 분리합니다.
        // 두 번째 인자 ImVec2(width, height)에서 height가 0이면 남은 세로 공간을 모두 사용합니다.
        ImGui::BeginChild("FolderTree", ImVec2(_folderTreeWidth, 0), true);
        drawFolderTree();
        ImGui::EndChild();

        // Splitter
        //
        // ImGui::SameLine()은 다음 위젯을 같은 행에 배치합니다. 여기서는 FolderTree 바로 오른쪽에 얇은 Button을 둡니다.
        // Button 자체는 "##Splitter"라는 ID-only label을 씁니다. "##" 뒤 문자열은 ImGui ID로만 쓰이고 화면에는 보이지 않습니다.
        // 드래그 중이면 MouseDelta.x를 누적해서 좌측 트리 폭을 조절합니다.
        ImGui::SameLine();
        _folderTreeWidth = EditorWidgets::DrawVerticalSplitter(
            "##Splitter",
            _folderTreeWidth,
            minFolderTreeWidth,
            maxFolderTreeWidth,
            splitterWidth);

        ImGui::SameLine();
    }

    // Right: Asset list
    //
    // showFolderTree가 true이면 이미 왼쪽 child + splitter + SameLine spacing이 공간을 사용했습니다.
    // 따라서 오른쪽 child에는 우리가 계산한 폭을 명시적으로 넣어 줍니다.
    // showFolderTree가 false일 때 width 0은 "남은 가로 공간 전체"라는 ImGui 관례입니다.
    float assetListWidth = showFolderTree
                               ? std::max(1.0f, contentWidth - _folderTreeWidth - splitterWidth - itemSpacingX * 2.0f)
                               : 0.0f;
    ImGui::BeginChild("AssetList", ImVec2(assetListWidth, 0), true);
    drawAssetList();
    ImGui::EndChild();

    // Context menu
    drawContextMenu();

    EditorPanelFrame::EndStandardPanel();
}

void ResourcePanel::drawPathBar()
{
    // Path bar는 현재 폴더 이동에 필요한 작은 toolbar입니다.
    // Button들을 SameLine으로 이어 붙인 뒤, 남는 공간의 오른쪽 끝에 검색창을 배치합니다.
    //
    // ImGui에서 흔히 하는 실수:
    //     ImGui::SameLine(ImGui::GetContentRegionAvail().x - 200);
    // 처럼 "남은 폭"을 절대 위치처럼 쓰는 것입니다. GetContentRegionAvail().x는 현재 커서 위치 기준 남은 폭이라,
    // 창이 좁아지면 음수가 될 수 있고 검색창이 왼쪽으로 튀거나 다음 위젯의 clip rect를 이상하게 만들 수 있습니다.
    // 그래서 아래에서는 남은 폭이 충분할 때만 SameLine + SetCursorPosX를 쓰고, 부족하면 검색창을 다음 줄 전체 폭으로 둡니다.

    // Navigation buttons
    bool canGoBack = _historyIndex > 0;
    bool canGoForward = _historyIndex < static_cast<int>(_pathHistory.size()) - 1;

    ImGui::BeginDisabled(!canGoBack);
    if (EditorWidgets::IconButton(EditorIcons::Back, "Back"))
    {
        navigateBack();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(!canGoForward);
    if (EditorWidgets::IconButton(EditorIcons::Forward, "Forward"))
    {
        navigateForward();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    if (EditorWidgets::IconButton(EditorIcons::Home, "Home"))
    {
        navigateToFolder("");
    }

    ImGui::SameLine();

    if (EditorWidgets::IconButton(EditorIcons::Refresh, "Refresh"))
    {
        AssetDatabase::Get().Refresh();
    }

    ImGui::SameLine();

    // Breadcrumb path
    ImGui::Text("Assets");

    if (!_currentPath.empty())
    {
        std::string path = _currentPath;
        size_t start = 0;
        size_t end = 0;

        while ((end = path.find('/', start)) != std::string::npos || start < path.size())
        {
            if (end == std::string::npos)
                end = path.size();

            std::string segment = path.substr(start, end - start);
            std::string fullPath = path.substr(0, end);

            if (!segment.empty())
            {
                ImGui::SameLine();
                ImGui::Text("/");
                ImGui::SameLine();

                if (ImGui::SmallButton(segment.c_str()))
                {
                    navigateToFolder(fullPath);
                }
            }

            start = end + 1;
            if (start >= path.size())
                break;
        }
    }

    // Search box
    EditorWidgets::SearchFieldRightAligned("##Search", "Search...", _searchBuffer, sizeof(_searchBuffer));
}

void ResourcePanel::drawFolderTree()
{
    const FolderEntry& root = AssetDatabase::Get().GetFolderTree();

    // Tree depth spacing is an editor-wide style decision. GUIContext keeps ImGuiStyle::IndentSpacing small
    // so ResourcePanel, HierarchyPanel, and InspectorPanel all present shallow Unity/Unreal-like nesting.
    drawFolderTreeNode(root);
}

void ResourcePanel::drawFolderTreeNode(const FolderEntry& folder)
{
    // ImGui tree는 재귀로 그리는 것이 가장 단순합니다.
    // TreeNodeEx()가 true를 반환하면 해당 노드가 열려 있다는 뜻이고, 이때만 자식 폴더들을 그린 뒤 TreePop()합니다.
    //
    // TreeNodeEx(label, flags)는 label 문자열을 ID로도 사용합니다. 같은 label이 형제 노드에 중복될 수 있다면 PushID를
    // 추가해야 합니다. 현재 폴더 트리는 path 구조상 같은 레벨에서 동일 이름 폴더가 생기기 어렵지만, 더 견고하게 하려면
    // folder.relativePath를 PushID로 감싸는 패턴을 고려할 수 있습니다.
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (folder.subFolders.empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (folder.relativePath == _currentPath)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Root folder is always open
    if (folder.relativePath.empty())
    {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    const char* icon = (folder.relativePath == _currentPath) ? EditorIcons::FolderOpen : EditorIcons::Folder;
    std::string displayName = folder.name.empty() ? "Assets" : folder.name;
    std::string label = std::string(icon) + " " + displayName;

    bool isOpen = EditorTreeWidgets::BeginNode(
        label.c_str(),
        folder.relativePath == _currentPath,
        folder.subFolders.empty(),
        folder.relativePath.empty());

    // TreeNode는 화살표 클릭으로 open/close되고, row 클릭으로 selection도 할 수 있습니다.
    // IsItemToggledOpen()을 확인해서 "화살표를 눌러 펼친 경우"에는 폴더 이동까지 같이 발생하지 않게 막습니다.
    if (EditorTreeWidgets::IsSelectionClick())
    {
        navigateToFolder(folder.relativePath);
    }

    if (isOpen)
    {
        for (const auto& subFolder : folder.subFolders)
        {
            drawFolderTreeNode(subFolder);
        }
        ImGui::TreePop();
    }
}

void ResourcePanel::drawAssetList()
{
    // 오른쪽 AssetList는 현재 경로의 하위 폴더와 파일 asset을 순서대로 보여줍니다.
    // 데이터는 AssetDatabase가 소유하고, 이 함수는 매 프레임 현재 필터/선택 상태에 맞춰 UI만 선언합니다.
    //
    // 폴더와 파일 모두 ImGui::Selectable()을 사용합니다.
    // 중요한 점: Selectable의 size 인자에 ImVec2(-1, 0)을 넣지 않습니다.
    // 일부 ImGui 버전/상황에서 음수 width가 기대처럼 "남은 폭 전체"가 아니라 지나치게 작은 텍스트 클리핑 폭으로
    // 처리될 수 있습니다. 이 패널에서 이름이 "[" 한 글자처럼 보였던 문제가 그 패턴에서 나왔습니다.
    // 따라서 기본 size를 쓰고, full-row selection이 꼭 필요하면 테이블/커스텀 hit box를 별도로 설계하는 편이 안전합니다.
    auto assets = AssetDatabase::Get().GetAssetsInFolder(_currentPath);
    auto subFolders = AssetDatabase::Get().GetSubFolders(_currentPath);

    // Filter by search
    std::string searchStr(_searchBuffer);
    for (auto& c : searchStr) c = static_cast<char>(std::tolower(c));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f));

    for (const auto& folderPath : subFolders)
    {
        // AssetDatabase는 folder path를 "Images/Foo" 같은 상대 경로로 돌려줍니다.
        // 리스트에는 마지막 segment만 보여주고, 클릭 시 전체 상대 경로로 navigate합니다.
        size_t lastSlash = folderPath.rfind('/');
        std::string folderName = (lastSlash != std::string::npos)
            ? folderPath.substr(lastSlash + 1)
            : folderPath;
        if (folderName.empty())
        {
            folderName = folderPath;
        }

        // Apply search filter
        if (!searchStr.empty())
        {
            std::string lowerName = folderName;
            for (auto& c : lowerName) c = static_cast<char>(std::tolower(c));
            if (lowerName.find(searchStr) == std::string::npos)
                continue;
        }

        ImGui::PushID(folderPath.c_str());
        std::string label = std::string(EditorIcons::Folder) + " " + folderName;
        bool isSelected = (folderPath == _currentPath);

        // PushID(folderPath) + visible label 조합입니다.
        // 같은 이름의 폴더가 다른 경로에 있더라도 ImGui ID가 충돌하지 않게 folderPath를 ID stack에 넣습니다.
        if (EditorListWidgets::SelectableRow("##FolderRow", label.c_str(), isSelected))
        {
            navigateToFolder(folderPath);
        }
        ImGui::PopID();
    }

    for (const auto* asset : assets)
    {
        // Apply search filter
        if (!searchStr.empty())
        {
            std::string lowerName = asset->name;
            for (auto& c : lowerName) c = static_cast<char>(std::tolower(c));
            if (lowerName.find(searchStr) == std::string::npos)
                continue;
        }

        drawAssetItem(asset);
    }

    ImGui::PopStyleVar();

    drawContextMenu();
}

void ResourcePanel::drawAssetItem(const AssetEntry* asset)
{
    // 파일 asset 하나를 한 줄로 그립니다.
    // asset->relativePath를 PushID로 사용해서 동일한 파일 이름이 다른 폴더에 있어도 ImGui ID가 충돌하지 않게 합니다.
    //
    // 이 함수가 담당하는 interaction:
    // - 클릭: 선택 상태 갱신
    // - 더블 클릭: model asset이면 현재 scene에 entity 생성
    // - drag source: 다른 패널이 asset path/model/texture payload를 받을 수 있게 함
    // - tooltip: 전체 이름, 타입, 파일 크기 표시
    ImGui::PushID(asset->relativePath.c_str());

    bool isSelected = (_selectedAssetPath == asset->relativePath);

    const char* icon = GetAssetTypeIcon(asset->type);
    const std::string& displayName = asset->name.empty() ? asset->relativePath : asset->name;
    std::string label = std::string(icon) + " " + displayName;
    if (EditorListWidgets::SelectableRow("##AssetRow", label.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
    {
        _selectedAssetPath = asset->relativePath;
        EditorContext::Get().SetSelectedAssetPath(asset->relativePath);
    }
    const bool rowHovered = ImGui::IsItemHovered();

    // IsItemHovered()는 "직전에 제출한 item"에 대한 상태입니다.
    // 이 함수 아래쪽에서 tooltip/drag source 등 추가 위젯이 생길 수 있으므로, 더블 클릭 판정에 쓸 hover 값은
    // Selectable 직후 rowHovered에 저장해 둡니다.
    if (rowHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        if (asset->type == EAssetType::Model)
        {
            Scene* scene = EditorContext::Get().GetActiveScene();
            if (scene)
            {
                hs::Model* model = AssetDatabase::Get().LoadModel(asset->relativePath);
                if (model)
                {
                    // Extract display name from asset name
                    std::string entityName = asset->name;
                    size_t dot = entityName.rfind('.');
                    if (dot != std::string::npos)
                        entityName = entityName.substr(0, dot);

                    Entity entity = scene->CreateEntity(entityName);
                    auto& meshRenderer = entity.AddComponent<MeshRendererComponent>();
                    meshRenderer.mesh = model->GetMesh();
                    if (model->GetMaterial())
                    {
                        meshRenderer.materials.push_back(model->GetMaterial());
                    }
                    if (meshRenderer.mesh)
                    {
                        const auto& bound = meshRenderer.mesh->GetBound();
                        meshRenderer.localBounds = AABB(glm::vec3(bound.min), glm::vec3(bound.max));
                        meshRenderer.boundsDirty = true;
                    }

                    EditorContext::Get().SetSelectedEntity(entity);
                }
            }
        }
    }

    // Drag source for drag & drop
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        // Payload type은 drop target이 어떤 asset인지 빠르게 구분하기 위한 문자열입니다.
        // 실제 payload data는 asset의 relativePath 문자열입니다. Drop target은 이 경로로 AssetDatabase에서 다시 로드하면 됩니다.
        const char* payloadType = nullptr;
        switch (asset->type)
        {
            case EAssetType::Model:   payloadType = "ASSET_MODEL"; break;
            case EAssetType::Texture: payloadType = "ASSET_TEXTURE"; break;
            case EAssetType::Material: payloadType = "ASSET_MATERIAL"; break;
            default:                  payloadType = "ASSET_PATH"; break;
        }

        ImGui::SetDragDropPayload(payloadType, asset->relativePath.c_str(),
                                   asset->relativePath.size() + 1);

        // Drag preview
        ImGui::Text("%s %s", icon, displayName.c_str());

        ImGui::EndDragDropSource();
    }

    // Tooltip
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s", displayName.c_str());
        ImGui::TextDisabled("Type: %s", GetAssetTypeName(asset->type));
        ImGui::TextDisabled("Size: %llu bytes", asset->fileSize);
        ImGui::EndTooltip();
    }

    ImGui::PopID();
}

void ResourcePanel::drawContextMenu()
{
    // AssetList child 영역 어디서 우클릭하든 현재 폴더 기준 생성 메뉴를 표시합니다.
    if (ImGui::BeginPopupContextWindow("AssetContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Material"))
            {
                createMaterialAssetInFolder(_currentPath);
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Refresh"))
        {
            AssetDatabase::Get().Refresh();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Show in Explorer"))
        {
            std::string folderPath = AssetDatabase::Get().GetRootPath();
            if (!_currentPath.empty())
            {
                folderPath += "/" + _currentPath;
            }
#if defined(__APPLE__)
            std::string cmd = "open \"" + folderPath + "\"";
            system(cmd.c_str());
#elif defined(_WIN32)
            std::string cmd = "explorer \"" + folderPath + "\"";
            system(cmd.c_str());
#endif
        }

        ImGui::EndPopup();
    }
}

void ResourcePanel::navigateToFolder(const std::string& path)
{
    // 폴더 이동은 UI 클릭에서 여러 경로로 호출됩니다.
    // 현재 경로와 같으면 history를 중복해서 쌓지 않기 위해 early return합니다.
    if (_currentPath == path)
        return;

    // Add to history
    if (_historyIndex < static_cast<int>(_pathHistory.size()) - 1)
    {
        // Remove forward history
        _pathHistory.resize(_historyIndex + 1);
    }

    _pathHistory.push_back(path);
    _historyIndex = static_cast<int>(_pathHistory.size()) - 1;

    _currentPath = path;
    _selectedAssetPath.clear();
    EditorContext::Get().SetCurrentAssetFolderPath(_currentPath);
    EditorContext::Get().ClearSelectedAssetPath();
}

void ResourcePanel::navigateBack()
{
    // Back/Forward는 _pathHistory와 _historyIndex만 조작합니다.
    // 실제 AssetDatabase scan은 경로 이동 때마다 하지 않습니다. 이미 scan된 asset map에서 현재 경로 기준 목록만 다시 꺼냅니다.
    if (_historyIndex > 0)
    {
        _historyIndex--;
        _currentPath = _pathHistory[_historyIndex];
        _selectedAssetPath.clear();
        EditorContext::Get().SetCurrentAssetFolderPath(_currentPath);
        EditorContext::Get().ClearSelectedAssetPath();
    }
}

void ResourcePanel::navigateForward()
{
    if (_historyIndex < static_cast<int>(_pathHistory.size()) - 1)
    {
        _historyIndex++;
        _currentPath = _pathHistory[_historyIndex];
        _selectedAssetPath.clear();
        EditorContext::Get().SetCurrentAssetFolderPath(_currentPath);
        EditorContext::Get().ClearSelectedAssetPath();
    }
}

bool ResourcePanel::createMaterialAssetInFolder(const std::string& folderPath)
{
    const std::string relativePath = buildUniqueMaterialRelativePathInFolder(folderPath);

    hs::Scoped<hs::Material> material = hs::MakeScoped<hs::Material>();
    material->SetDisplayName(std::filesystem::path(relativePath).stem().string());
    material->SetTexture(hs::EMaterialTextureType::Diffuse, const_cast<hs::Image*>(hs::ObjectManager::GetFallbackImage2DWhite()));

    if (!AssetDatabase::Get().SaveMaterial(relativePath, material.get()))
    {
        return false;
    }

    AssetDatabase::Get().Refresh();
    _selectedAssetPath = relativePath;
    EditorContext::Get().SetSelectedAssetPath(relativePath);
    return true;
}

HS_NS_EDITOR_END
