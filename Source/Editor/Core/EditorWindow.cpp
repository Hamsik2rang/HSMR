#include "Editor/Core/EditorWindow.h"
#include "Editor/Core/EditorContext.h"

#include "Core/HAL/FileSystem.h"
#include "Core/HAL/Input.h"
#include "Core/Profiler/Profiler.h"

#include "RHI/Swapchain.h"
#include "RHI/RenderHandle.h"
#include "RHI/CommandHandle.h"

#include "Renderer/ForwardRenderer.h"
#include "Renderer/RenderPass/ForwardOpaquePass.h"
#include "Resource/ObjectManager.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

#include "Editor/GUI/ImGuiExtension.h"
#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/EditorApplication.h"

#include "Editor/Panel/Panel.h"
#include "Editor/Panel/DockspacePanel.h"
#include "Editor/Panel/MenuPanel.h"
#include "Editor/Panel/ScenePanel.h"
#include "Editor/Panel/SceneStatusPanel.h"
#include "Editor/Panel/HierarchyPanel.h"
#include "Editor/Panel/InspectorPanel.h"
#include "Editor/Panel/ResourcePanel.h"
#include "Editor/Panel/ProfilerPanel.h"

#include "Core/Profiler/ProfileDataCollector.h"

#include <limits>
#include <vector>

HS_NS_EDITOR_BEGIN

namespace
{
RenderViewSnapshot buildEditorViewSnapshot(EditorCamera* editorCamera)
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
    perView.cameraPosition = editorCamera->GetPosition();

    viewSnapshot.perView = perView;
    return viewSnapshot;
}
}

EditorWindow::EditorWindow(Application* ownerApp, const char* name, uint32 width, uint32 height, EWindowFlags flags)
    : Window(ownerApp, name, width, height, flags)
{
    onInitialize();
}

EditorWindow::~EditorWindow()
{}

bool EditorWindow::onInitialize()
{
    _renderer = MakeScoped<ForwardRenderer>(_rhiContext);
    _renderer->Initialize();

    ImGuiExtension::InitializeBackend(_swapchain);

    // Apply DPI scaling for high-resolution displays (e.g., 4K monitors)
    GUIContext* guiContext = static_cast<EditorApplication*>(_ownerApp)->GetGUIContext();
    float dpiScale         = _nativeWindow.scale;
    if (dpiScale > 1.0f)
    {
        guiContext->ApplyDPIScale(dpiScale);
    }

    auto* opaquePass = new ForwardOpaquePass("Forward Opaque Pass", _renderer.get(), ERenderingOrder::Opaque);
    _renderer->AddPass(std::move(opaquePass));

    setupResources();
    setupDefaultScene();
    setupPanels();

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

    Resolution resolution = static_cast<ScenePanel*>(_scenePanel.get())->GetResolution();
    uint32 width          = static_cast<uint32>(resolution.width / _nativeWindow.scale);
    uint32 height         = static_cast<uint32>(resolution.height / _nativeWindow.scale);

    for (auto& renderTarget : _renderTargets)
    {
        renderTarget.Update(resolution.width, resolution.height);
    }
}

void EditorWindow::onUpdate(float deltaTime)
{
    HS_COLLECT_ZONE_NC("Update", HS::Profile::ColorScene);
    processShortcuts();
    updateSceneCamera(deltaTime);

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
    RenderTarget* curRT = &_renderTargets[imageIndex];

    // 1. Render Scene to Scene Panel
    {
        HS_COLLECT_ZONE_NC("Scene Render", HS::Profile::ColorRender);
        ScenePanel* scenePanel = static_cast<ScenePanel*>(_scenePanel.get());
        RenderSceneSnapshot sceneSnapshot = _renderer->GetResourceManager()->BuildRenderSceneSnapshot(
            _scene.get(), _renderer->GetShaderLibrary());
        RenderViewSnapshot editorViewSnapshot = buildEditorViewSnapshot(scenePanel->GetEditorCamera());

        if (sceneSnapshot.views.empty())
        {
            sceneSnapshot.views.push_back(editorViewSnapshot);
        }
        else
        {
            sceneSnapshot.views[0] = editorViewSnapshot;
        }

        _renderer->Render(sceneSnapshot, curRT);
    }

    static_cast<ScenePanel*>(_scenePanel.get())->SetSceneRenderTarget(&_renderTargets[imageIndex]);

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

    // Clear editor context
    EditorContext::Get().SetActiveScene(nullptr);
    EditorContext::Get().RemoveAllSelectionListeners();

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

    _basePanel->Draw(); // Draw panel tree.

    guiContext->EndRender();
}

void EditorWindow::setupPanels()
{
    _basePanel = MakeScoped<DockspacePanel>(this);
    _basePanel->Setup();

    _menuPanel = MakeScoped<MenuPanel>(this);
    _menuPanel->Setup();
    _basePanel->InsertPanel(_menuPanel.get());

    _scenePanel = MakeScoped<ScenePanel>(this);
    _scenePanel->Setup();
    _basePanel->InsertPanel(_scenePanel.get());

    _sceneStatusPanel = MakeScoped<SceneStatusPanel>(this);
    _sceneStatusPanel->Setup();
    _basePanel->InsertPanel(_sceneStatusPanel.get());

    _hierarchyPanel = MakeScoped<HierarchyPanel>(this);
    _hierarchyPanel->Setup();
    _basePanel->InsertPanel(_hierarchyPanel.get());

    _inspectorPanel = MakeScoped<InspectorPanel>(this);
    _inspectorPanel->Setup();
    _basePanel->InsertPanel(_inspectorPanel.get());

    _resourcePanel = MakeScoped<ResourcePanel>(this);
    _resourcePanel->Setup();
    _basePanel->InsertPanel(_resourcePanel.get());

    _profilerPanel = MakeScoped<ProfilerPanel>(this);
    _profilerPanel->Setup();
    _basePanel->InsertPanel(_profilerPanel.get());
}

void EditorWindow::setupDefaultScene()
{
    _scene = MakeScoped<Scene>("Default Scene");
    EditorContext::Get().SetActiveScene(_scene.get());

    // Editor camera entity
    Entity cameraEntity = _scene->CreateEntity("Editor Camera");
    auto& camera = cameraEntity.AddComponent<CameraComponent>();
    camera.isPrimary = true;

    // Test mesh entity
    Entity entity = _scene->CreateEntity("Damaged Helmet");
    entity.GetComponent<TransformComponent>().SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    entity.AddComponent<MeshRendererComponent>();

    _scene->Update(0.0f);
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
        ScenePanel* scenePanel = static_cast<ScenePanel*>(_scenePanel.get());
        SceneStatusPanel* sceneStatusPanel = static_cast<SceneStatusPanel*>(_sceneStatusPanel.get());

        sceneStatusPanel->SetSceneCamera(scenePanel->GetEditorCamera());
        sceneStatusPanel->SetSceneBounds(scenePanel->GetViewportMin(), scenePanel->GetViewportMax());
    }
}

void EditorWindow::processShortcuts()
{
    // Ctrl+S (Windows) or Cmd+S (Mac) to save layout
#if defined(__APPLE__)
    bool modifierPressed = Input::IsPressed(Input::Button::LwinOrCommand);
#else
    bool modifierPressed = Input::IsPressed(Input::Button::Control);
#endif

    static bool sKeyWasPressed = false;

    if (modifierPressed && Input::IsPressed(Input::Button::S))
    {
        if (!sKeyWasPressed)
        {
            auto* guiContext = static_cast<EditorApplication*>(_ownerApp)->GetGUIContext();
            if (guiContext)
            {
                guiContext->SaveLayout("");
            }
            sKeyWasPressed = true;
        }
    }
    else
    {
        sKeyWasPressed = false;
    }

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
