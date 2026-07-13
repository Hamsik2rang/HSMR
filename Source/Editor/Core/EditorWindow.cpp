#include "Editor/Core/EditorWindow.h"
#include "Editor/Core/EditorContext.h"

#include "Core/HAL/FileSystem.h"
#include "Core/HAL/Input.h"
#include "Core/Profiler/Profiler.h"

#include "RHI/Swapchain.h"
#include "RHI/RenderHandle.h"
#include "RHI/CommandHandle.h"

#include "Renderer/ForwardRenderer.h"
#include "Editor/Renderer/EditorRenderer.h"
#include "Renderer/CameraUtils.h"
#include "Resource/ObjectManager.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"
#include "Scene/SceneSerializer.h"

#include "Editor/GUI/ImGuiExtension.h"
#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/EditorApplication.h"
#include "Editor/Project/ProjectContext.h"

#include "Editor/Panel/Panel.h"
#include "Editor/Panel/DockspacePanel.h"
#include "Editor/Panel/MenuPanel.h"
#include "Editor/Panel/ScenePanel.h"
#include "Editor/Panel/GamePanel.h"
#include "Editor/Panel/HierarchyPanel.h"
#include "Editor/Panel/InspectorPanel.h"
#include "Editor/Panel/ProfilerPanel.h"

#include "Core/Profiler/ProfileDataCollector.h"

#include <limits>
#include <vector>

HS_NS_EDITOR_BEGIN

namespace
{
constexpr uint64 s_invalidGameViewId = std::numeric_limits<uint64>::max() - 1;

RenderViewSnapshot buildEditorViewSnapshot(EditorCamera* editorCamera, uint32 width, uint32 height)
{
    RenderViewSnapshot viewSnapshot{};
    viewSnapshot.viewId = std::numeric_limits<uint64>::max();

    if (!editorCamera)
    {
        return viewSnapshot;
    }

    editorCamera->Update();

    PerView perView{};
    perView.viewMatrix = editorCamera->GetViewMatrix();
    perView.projectionMatrix = editorCamera->GetProjectionMatrix();
    perView.viewProjectionMatrix = editorCamera->GetViewProjectionMatrix();
    perView.inverseViewMatrix = editorCamera->GetInverseViewMatrix();
    perView.inverseProjectionMatrix = editorCamera->GetInverseProjectionMatrix();
    perView.inverseViewProjectionMatrix = editorCamera->GetInverseViewProjectionMatrix();
    perView.cameraPositionTime = glm::vec4(editorCamera->GetPosition(), 0.0f);
    perView.resolution = glm::vec4(static_cast<float>(width), static_cast<float>(height), 0.0f, 0.0f);

    viewSnapshot.perView = perView;
    return viewSnapshot;
}

RenderViewSnapshot buildSceneCameraViewSnapshot(Entity cameraEntity, bool vulkanYFlip, uint32 width, uint32 height)
{
    RenderViewSnapshot viewSnapshot{};
    viewSnapshot.viewId = s_invalidGameViewId;
    if (!cameraEntity.IsValid() ||
        !cameraEntity.HasComponent<TransformComponent>() ||
        !cameraEntity.HasComponent<CameraComponent>())
    {
        return viewSnapshot;
    }

    TransformComponent transform = cameraEntity.GetComponent<TransformComponent>();
    CameraComponent camera = cameraEntity.GetComponent<CameraComponent>();
    camera.SetAspectRatio(static_cast<float>(width), static_cast<float>(height));

    viewSnapshot.viewId = static_cast<uint64>(entt::to_integral(cameraEntity.GetHandle()));
    viewSnapshot.perView = CameraUtils::BuildPerViewData(transform, camera, vulkanYFlip);
    viewSnapshot.perView.resolution = glm::vec4(static_cast<float>(width), static_cast<float>(height), 0.0f, 0.0f);
    return viewSnapshot;
}

void setSingleViewSnapshot(RenderSceneSnapshot& snapshot, const RenderViewSnapshot& viewSnapshot)
{
    snapshot.views.clear();
    if (viewSnapshot.viewId != s_invalidGameViewId)
    {
        snapshot.views.push_back(viewSnapshot);
    }
}

void populateStarterScene(Scene& scene)
{
    scene.SetName("Main");

    Entity cameraEntity = scene.CreateEntity("Main Camera");
    auto& camera = cameraEntity.AddComponent<CameraComponent>();
    camera.isPrimary = true;
    camera.isActive = true;
    camera.priority = 100;

    Entity lightEntity = scene.CreateEntity("Directional Light");
    auto& light = lightEntity.AddComponent<LightComponent>();
    light.type = ELightType::Directional;
    auto& lightTransform = lightEntity.GetComponent<TransformComponent>();
    lightTransform.SetPosition(glm::vec3(0.0f, 3.0f, 0.0f));
    lightTransform.SetEulerAngles(glm::vec3(-90.0f, 0.0f, 45.0f));

    scene.Update(0.0f);
}
}

EditorWindow::EditorWindow(Application* ownerApp, const char* name, uint32 width, uint32 height, EWindowFlags flags)
    : Window(ownerApp, name, width, height, flags)
{
    onInitialize();
}

EditorWindow::~EditorWindow()
{}

GUIContext* EditorWindow::GetGUIContext()
{
    return static_cast<EditorApplication*>(_ownerApp)->GetGUIContext();
}

bool EditorWindow::onInitialize()
{
    _renderer = MakeScoped<ForwardRenderer>(_rhiContext);
    _renderer->Initialize();
    _overlayRenderer = MakeScoped<EditorRenderer>(_rhiContext);
    _overlayRenderer->Initialize(_renderer->GetShaderLibrary());

    ImGuiExtension::InitializeBackend(_swapchain);

    // Apply DPI scaling for high-resolution displays (e.g., 4K monitors)
    GUIContext* guiContext = static_cast<EditorApplication*>(_ownerApp)->GetGUIContext();
    float dpiScale         = _nativeWindow.scale;
    if (dpiScale > 1.0f)
    {
        guiContext->ApplyDPIScale(dpiScale);
    }

    setupResources();
    loadInitialScene();
    setupPanels();

    if (_menuPanel && !_currentScenePath.empty())
    {
        static_cast<MenuPanel*>(_menuPanel.get())->SetCurrentScenePath(_currentScenePath);
    }

    void* handler = nullptr;
    ImGuiExtension::SetProcessEventHandler(&handler);
    SetPreEventHandler(handler);

    return true;
}

void EditorWindow::onNextFrame()
{
    ProfileDataCollector::Get().BeginFrame();

    if (false == _shouldPresent)
    {
        return;
    }

    _renderer->NextFrame(_swapchain);

    // Each panel resizes its own offscreen RTs in its Update() callback.
    // The Window's swapchain-backed _renderTargets follow swapchain dimensions automatically.
}

void EditorWindow::onUpdate(float deltaTime)
{
    HS_COLLECT_ZONE_NC("Update", HS::Profile::ColorScene);
    processShortcuts();
    updateSceneCamera(deltaTime);

    for (Panel* panel : _registeredPanels)
    {
        if (panel)
        {
            panel->Update(deltaTime);
        }
    }

    // Update scene transforms
    if (_scene)
    {
        _scene->Update(deltaTime);
    }
}

void EditorWindow::onRender()
{
    HS_COLLECT_ZONE_NC("Render", HS::Profile::ColorRender);

    if (false == _shouldPresent)
    {
        return;
    }
    RHICommandBuffer* cmdBuffer = _swapchain->GetCommandBufferForCurrentFrame();
    cmdBuffer->Begin();

    uint8 imageIndex    = _swapchain->GetCurrentImageIndex();
    ScenePanel* scenePanel = static_cast<ScenePanel*>(_scenePanel.get());
    GamePanel*  gamePanel  = _gamePanel ? static_cast<GamePanel*>(_gamePanel.get()) : nullptr;

    RenderTarget* sceneRT = scenePanel ? scenePanel->GetRenderTarget(imageIndex) : nullptr;
    RenderTarget* gameRT  = gamePanel  ? gamePanel->GetRenderTarget(imageIndex)  : nullptr;
    const bool vulkanYFlip = (_rhiContext->GetCurrentPlatform() == ERHIPlatform::Vulkan);

    // 1. Render Scene to Scene Panel
    if (sceneRT)
    {
        HS_COLLECT_ZONE_NC("Scene Render", HS::Profile::ColorRender);
        RenderSceneSnapshot baseSnapshot = _renderer->GetResourceManager()->BuildRenderSceneSnapshot(
            _scene.get(), _renderer->GetShaderLibrary());
        RenderViewSnapshot editorViewSnapshot = buildEditorViewSnapshot(
            scenePanel->GetEditorCamera(),
            sceneRT->GetWidth(),
            sceneRT->GetHeight());
        RenderSceneSnapshot sceneSnapshot = baseSnapshot;
        setSingleViewSnapshot(sceneSnapshot, editorViewSnapshot);

        RenderOptions sceneOptions{};
        sceneOptions.enableGrid = true;
        sceneOptions.enableDebug = EditorContext::Get().GetDebugDrawSettings().showDebugPass;
        _renderer->Render(sceneSnapshot, sceneRT, RenderOptions{});
        if (_overlayRenderer)
        {
            _overlayRenderer->Render(
                *cmdBuffer,
                *_renderer->GetResourceManager(),
                baseSnapshot,
                editorViewSnapshot,
                sceneRT,
                sceneOptions);
        }

        if (gamePanel && gameRT)
        {
            Entity gameCamera = gamePanel->ResolveCamera(_scene.get());
            RenderViewSnapshot gameViewSnapshot = buildSceneCameraViewSnapshot(
                gameCamera,
                vulkanYFlip,
                gameRT->GetWidth(),
                gameRT->GetHeight());

            if (gameViewSnapshot.viewId != s_invalidGameViewId)
            {
                RenderSceneSnapshot gameSnapshot = baseSnapshot;
                setSingleViewSnapshot(gameSnapshot, gameViewSnapshot);
                _renderer->Render(gameSnapshot, gameRT, RenderOptions{});
            }
        }
    }

    // 2. Render GUI
    {
        HS_COLLECT_ZONE_NC("GUI Render", HS::Profile::ColorUI);
        onRenderGUI();
    }

    cmdBuffer->End();

    _rhiContext->Submit(_swapchain, &cmdBuffer, 1);
}

void EditorWindow::onPresent()
{
    if (!_shouldPresent)
    {
        return;
    }
    RHIContext::Get()->Present(_swapchain);
}

void EditorWindow::onShutdown()
{
    ImGuiExtension::FinalizeBackend();

    cleanupPanels();

    // Clear editor context
    EditorContext::Get().SetActiveScene(nullptr);
    EditorContext::Get().RemoveAllSelectionListeners();

    if (_overlayRenderer)
    {
        _overlayRenderer->Shutdown();
        _overlayRenderer.reset();
    }

    if (_renderer)
    {
        _renderer->Shutdown();
        _renderer.reset();
    }

    _scene.reset();
}

void EditorWindow::onRenderGUI()
{
    GUIContext* guiContext = static_cast<EditorApplication*>(GetApplication())->GetGUIContext();

    guiContext->BeginRender(_swapchain);

    if (_basePanel)
    {
        _basePanel->Draw();
    }

    guiContext->EndRender();
}

void EditorWindow::registerPanel(Panel* panel, bool* visibilityBinding, Panel* parent)
{
    if (!panel)
    {
        return;
    }

    panel->BindVisibility(visibilityBinding);
    panel->Setup();
    _registeredPanels.push_back(panel);

    if (parent)
    {
        parent->InsertPanel(panel);
    }
}

void EditorWindow::cleanupPanels()
{
    for (auto it = _registeredPanels.rbegin(); it != _registeredPanels.rend(); ++it)
    {
        if (*it)
        {
            (*it)->Cleanup();
        }
    }
    _registeredPanels.clear();
}

void EditorWindow::setupPanels()
{
    PanelVisibility& visibility = EditorContext::Get().GetPanelVisibility();

    _basePanel = MakeScoped<DockspacePanel>(this);
    registerPanel(_basePanel.get());

    _menuPanel = MakeScoped<MenuPanel>(this);
    registerPanel(_menuPanel.get(), nullptr, _basePanel.get());

    _scenePanel = MakeScoped<ScenePanel>(this);
    registerPanel(_scenePanel.get(), &visibility.scene, _basePanel.get());

    _gamePanel = MakeScoped<GamePanel>(this);
    registerPanel(_gamePanel.get(), &visibility.game, _basePanel.get());

    _hierarchyPanel = MakeScoped<HierarchyPanel>(this);
    registerPanel(_hierarchyPanel.get(), &visibility.hierarchy, _basePanel.get());

    _inspectorPanel = MakeScoped<InspectorPanel>(this);
    registerPanel(_inspectorPanel.get(), &visibility.inspector, _basePanel.get());

    _profilerPanel = MakeScoped<ProfilerPanel>(this);
    registerPanel(_profilerPanel.get(), &visibility.profiler, _basePanel.get());
}

void EditorWindow::setupDefaultScene()
{
    _scene = MakeScoped<Scene>("Default Scene");
    EditorContext::Get().SetActiveScene(_scene.get());
    EditorContext::Get().ClearCurrentScenePath();
    populateStarterScene(*_scene);
}

bool EditorWindow::loadInitialScene()
{
    ProjectContext& projectContext = ProjectContext::Get();
    const std::string defaultScenePath = projectContext.GetResolvedDefaultScenePath();

    if (projectContext.IsProjectOpen() && !defaultScenePath.empty() && FileSystem::Exist(defaultScenePath))
    {
        _scene = MakeScoped<Scene>("Main");
        SceneSerializer serializer(_scene.get());
        if (serializer.LoadFromFile(defaultScenePath))
        {
            EditorContext::Get().SetActiveScene(_scene.get());
            _currentScenePath = defaultScenePath;
            EditorContext::Get().SetCurrentScenePath(defaultScenePath);
            return true;
        }

        HS_LOG(error, "[EditorWindow] Failed to load startup scene: %s", defaultScenePath.c_str());
        _scene.reset();
    }

    setupDefaultScene();
    persistDefaultSceneAsset();
    return true;
}

void EditorWindow::persistDefaultSceneAsset()
{
    ProjectContext& projectContext = ProjectContext::Get();
    if (!projectContext.IsProjectOpen() || !_scene)
    {
        return;
    }

    const std::string defaultScenePath = projectContext.GetResolvedDefaultScenePath().empty()
        ? projectContext.GetScenePath() + "Main.scene"
        : projectContext.GetResolvedDefaultScenePath();

    SceneSerializer serializer(_scene.get());
    if (!serializer.SaveToFile(defaultScenePath))
    {
        HS_LOG(error, "[EditorWindow] Failed to persist fallback startup scene: %s", defaultScenePath.c_str());
        return;
    }

    projectContext.SetDefaultScene(defaultScenePath);
    _currentScenePath = defaultScenePath;
    EditorContext::Get().SetCurrentScenePath(defaultScenePath);
}

void EditorWindow::syncEditorCameraToScene()
{
    if (!_scene || !_scenePanel) return;

    EditorCamera* editorCamera = static_cast<ScenePanel*>(_scenePanel.get())->GetEditorCamera();
    if (!editorCamera) return;

    editorCamera->Update();

    Entity cameraEntity = _scene->GetPrimaryCamera();
    if (!cameraEntity.IsValid()) return;

    auto& transform = cameraEntity.GetComponent<TransformComponent>();
    auto& camera = cameraEntity.GetComponent<CameraComponent>();

    const auto& editorTransform = editorCamera->GetTransform();
    const auto& editorCameraComp = editorCamera->GetCameraComponent();

    transform.SetPosition(editorTransform.position);
    transform.SetRotation(editorTransform.rotation);
    camera = editorCameraComp;
}

void EditorWindow::updateSceneCamera(float deltaTime)
{
    if (_scenePanel)
    {
        _scenePanel->Update(deltaTime);
    }
}

void EditorWindow::processShortcuts()
{
    // Scene shortcuts use Ctrl on Windows/Linux and Cmd on macOS.
#if defined(__APPLE__)
    bool modifierPressed = Input::IsPressed(Input::Button::LwinOrCommand);
#else
    bool modifierPressed = Input::IsPressed(Input::Button::Control);
#endif
    bool shiftPressed = Input::IsPressed(Input::Button::Shift);
    MenuPanel* menuPanel = static_cast<MenuPanel*>(_menuPanel.get());

    static bool shortcutWasPressed = false;
    bool handledShortcut = false;

    if (modifierPressed && menuPanel)
    {
        if (Input::IsPressed(Input::Button::N))
        {
            handledShortcut = true;
            if (!shortcutWasPressed)
            {
                menuPanel->ExecuteNewScene();
            }
        }
        else if (Input::IsPressed(Input::Button::O))
        {
            handledShortcut = true;
            if (!shortcutWasPressed)
            {
                menuPanel->ExecuteOpenScene();
            }
        }
        else if (Input::IsPressed(Input::Button::S) && shiftPressed)
        {
            handledShortcut = true;
            if (!shortcutWasPressed)
            {
                menuPanel->ExecuteSaveSceneAs();
            }
        }
        else if (Input::IsPressed(Input::Button::S))
        {
            handledShortcut = true;
            if (!shortcutWasPressed)
            {
                menuPanel->ExecuteSaveScene();
            }
        }
    }

    shortcutWasPressed = handledShortcut;

    // Gizmo operation shortcuts (W/E/R for Translate/Rotate/Scale)
    auto& context = EditorContext::Get();

    if (Input::IsPressed(Input::Button::W) && !Input::IsPressed(Input::Button::MouseRight))
    {
        context.SetGizmoOperation(EditorContext::GizmoOperation::Translate);
    }
    if (Input::IsPressed(Input::Button::E) && !Input::IsPressed(Input::Button::MouseRight))
    {
        context.SetGizmoOperation(EditorContext::GizmoOperation::Rotate);
    }
    if (Input::IsPressed(Input::Button::R))
    {
        context.SetGizmoOperation(EditorContext::GizmoOperation::Scale);
    }

    // Toggle local/world space with Q (when not moving camera)
    static bool qKeyWasPressed = false;
    if (Input::IsPressed(Input::Button::Q) && !Input::IsPressed(Input::Button::MouseRight))
    {
        if (!qKeyWasPressed)
        {
            auto currentSpace = context.GetGizmoSpace();
            context.SetGizmoSpace(currentSpace == EditorContext::GizmoSpace::Local
                ? EditorContext::GizmoSpace::World
                : EditorContext::GizmoSpace::Local);
            qKeyWasPressed = true;
        }
    }
    else
    {
        qKeyWasPressed = false;
    }
}

void EditorWindow::setupResources()
{
    std::string gltfPath = std::string("GLTF") +
                           HS_DIR_SEPERATOR +
                           "DamagedHelmet" +
                           HS_DIR_SEPERATOR +
                           "DamagedHelmet.gltf";

    //ObjectManager::LoadModel(gltfPath, _model);
}

HS_NS_EDITOR_END
