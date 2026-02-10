#include "Editor/Core/EditorWindow.h"

#include "Core/HAL/FileSystem.h"
#include "Core/HAL/Input.h"
#include "Core/Profiler/Profiler.h"

#include "RHI/Swapchain.h"
#include "RHI/RenderHandle.h"
#include "RHI/CommandHandle.h"

#include "Engine/Renderer/ForwardPath.h"
#include "Engine/Renderer/RenderPass/ForwardOpaquePass.h"
#include "Engine/Resource/ObjectManager.h"

#include "Editor/GUI/ImGuiExtension.h"
#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/EditorApplication.h"

#include "Editor/Panel/Panel.h"
#include "Editor/Panel/DockspacePanel.h"
#include "Editor/Panel/MenuPanel.h"
#include "Editor/Panel/ScenePanel.h"
#include "Editor/Panel/ProfilerPanel.h"
#include "Editor/Panel/HierarchyPanel.h"

#include "Editor/Core/EditorCamera.h"

#include <vector>

HS_NS_EDITOR_BEGIN

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

    auto* opaquePass = new ForwardOpaquePass("Forward Opaque Pass", _renderer.get(), ERenderingOrder::OPAQUE);
    _renderer->AddPass(std::move(opaquePass));

    setupPanels();

    void* handler = nullptr;
    ImGuiExtension::SetProcessEventHandler(&handler);
    SetPreEventHandler(handler);

    _editorCamera = MakeScoped<EditorCamera>();

    setupResources();

    return true;
}

void EditorWindow::onNextFrame()
{
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
    HS_PROFILE_FUNCTION();
    processShortcuts();
    updateSceneCamera(deltaTime);
}

void EditorWindow::onResize()
{
}

void EditorWindow::onRender()
{
    HS_PROFILE_ZONE_NC("EditorWindow::onRender", HS::Profile::ColorRender);

    if (false == _shouldPresent)
    {
        return;
    }
    RHICommandBuffer* cmdBuffer = _swapchain->GetCommandBufferForCurrentFrame();
    cmdBuffer->Begin();

    uint8 imageIndex    = _swapchain->GetCurrentImageIndex();
    RenderTarget* curRT = &_renderTargets[imageIndex];

    RenderParameter param{};

    param.models.push_back(_model.get());

    Camera* sceneCamera = static_cast<ScenePanel*>(_scenePanel.get())->GetCamera();
    if (sceneCamera)
    {
        param.cameras.push_back(sceneCamera);
    }

    // 1. Render Scene to Scene Panel
    {
        HS_PROFILE_ZONE_NC("Scene Render", HS::Profile::ColorRender);
        _renderer->Render(param, curRT);
    }

    static_cast<ScenePanel*>(_scenePanel.get())->SetSceneRenderTarget(&_renderTargets[imageIndex]);

    // 2. Render GUI
    {
        HS_PROFILE_ZONE_NC("GUI Render", HS::Profile::ColorUI);
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

    if (_renderer)
    {
        _renderer->Shutdown();
        _renderer.reset(); // Automatic cleanup with Scoped<>
    }
}

void EditorWindow::onRenderGUI()
{
    GUIContext* guiContext = static_cast<EditorApplication*>(GetApplication())->GetGUIContext();

    //	guiContext->SetScaleFactor(_nativeWindow.scale);

    // TODO: 어차피 필요하니 스왑체인이 렌더패스 핸들을 들고있도록 하고 이 함수가 인자로 렌더패스 핸들을 받도록 하기
    guiContext->BeginRender(_swapchain);

    _basePanel->Draw(); // Draw panel tree.

    guiContext->EndRender();

    //	guiContext->SetScaleFactor(1.0f / _nativeWindow.scale);
}

void EditorWindow::onSuspend()
{
    _rhiContext->Suspend(_swapchain);
}

void EditorWindow::onRestore()
{
    _rhiContext->Restore(_swapchain);
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

    _profilerPanel = MakeScoped<ProfilerPanel>(this);
    _profilerPanel->Setup();
    _basePanel->InsertPanel(_profilerPanel.get());

    //_hierarchyPanel = MakeScoped<HierarchyPanel>(this);
    //_hierarchyPanel->Setup();
    //_basePanel->InsertPanel(_hierarchyPanel.get());
}

void EditorWindow::updateSceneCamera(float deltaTime)
{
    if (_scenePanel)
    {
        _scenePanel->Update(deltaTime);
        Camera* camera = static_cast<ScenePanel*>(_scenePanel.get())->GetCamera();
        static_cast<ProfilerPanel*>(_profilerPanel.get())->SetSceneCamera(camera);
    }
}

void EditorWindow::processShortcuts()
{
    // Ctrl+S (Windows) or Cmd+S (Mac) to save layout
#if defined(__APPLE__)
    bool modifierPressed = Input::IsPressed(Input::Button::LWIN_OR_COMMAND);
#else
    bool modifierPressed = Input::IsPressed(Input::Button::CONTROL);
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
}

void EditorWindow::setupResources()
{
    std::string gltfPath = std::string("GLTF") +
                           HS_DIR_SEPERATOR +
                           "DamagedHelmet" +
                           HS_DIR_SEPERATOR +
                           "DamagedHelmet.gltf";

    ObjectManager::LoadModel(gltfPath, _model);

    //_model->SetRotation(glm::vec3(0.0f, glm::radians(240.0f), 0.0f));
}

HS_NS_EDITOR_END
