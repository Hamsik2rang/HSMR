#include "Editor/Core/SimpleWindow.h"

#include "Core/HAL/Input.h"
#include "Core/Math/Common.h"
#include "ThirdParty/ImGuizmo/ImGuizmo.h"

#include "RHI/Swapchain.h"
#include "RHI/RenderHandle.h"
#include "RHI/CommandHandle.h"

#include "Renderer/ForwardRenderer.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

#include "Editor/GUI/ImGuiExtension.h"
#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/SimpleApplication.h"
#include "Editor/Core/EditorCamera.h"
#include "Editor/Core/EditorContext.h"

#include "Resource/ObjectManager.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"

HS_NS_EDITOR_BEGIN

SimpleWindow::SimpleWindow(Application* ownerApp, const char* name, uint32 width, uint32 height, EWindowFlags flags)
    : Window(ownerApp, name, static_cast<uint16>(width), static_cast<uint16>(height), flags)
{
    onInitialize();
}

SimpleWindow::~SimpleWindow()
{
}

bool SimpleWindow::onInitialize()
{
    _renderer = MakeScoped<ForwardRenderer>(_rhiContext);
    _renderer->Initialize();

    ImGuiExtension::InitializeBackend(_swapchain);

    // Apply DPI scaling
    GUIContext* guiContext = GetGUIContext();
    float dpiScale = _nativeWindow.scale;
    if (dpiScale > 1.0f)
    {
        guiContext->ApplyDPIScale(dpiScale);
    }

    _camera = MakeScoped<EditorCamera>();
    float aspect = static_cast<float>(_nativeWindow.surfaceWidth) / static_cast<float>(_nativeWindow.surfaceHeight);
    _camera->SetAspectRatio(aspect);
    // Sponza fits inside ~50 units; tighter bounds give us ~6x more depth
    // precision than the previous 0.5/300 (which still showed z-fighting).
    _camera->SetNearZ(0.1f);
    _camera->SetFarZ(1000.0f);

    _menuPanel = MakeScoped<MenuPanel>(this, MenuPanel::EMode::MainMenuBar);
    _menuPanel->Setup();

    _inspectorPanel = MakeScoped<SimpleInspectorPanel>(this);
    _inspectorPanel->Setup();

    setupDefaultScene();

    // ImGui event handler
    void* handler = nullptr;
    ImGuiExtension::SetProcessEventHandler(&handler);
    SetPreEventHandler(handler);
    
    return true;
}

void SimpleWindow::onNextFrame()
{
    if (!_shouldPresent)
    {
        return;
    }

    _renderer->NextFrame(_swapchain);

    uint32 width = _swapchain->GetWidth();
    uint32 height = _swapchain->GetHeight();

    for (auto& renderTarget : _swapchainRenderTargets)
    {
        renderTarget.Update(width, height);
    }
}

void SimpleWindow::onUpdate(float deltaTime)
{
    processCameraInput(deltaTime);

    _camera->Update();
    syncEditorCameraToScene();

    if (_scene)
    {
        _scene->Update(deltaTime);
    }
}

void SimpleWindow::onRender()
{
    if (!_shouldPresent)
    {
        return;
    }

    RHICommandBuffer* cmdBuffer = _swapchain->GetCommandBufferForCurrentFrame();
    cmdBuffer->Begin();

    uint8 imageIndex = _swapchain->GetCurrentImageIndex();
    RenderTarget* curRT = &_swapchainRenderTargets[imageIndex];

    // Scene primary camera is kept in sync with EditorCamera by syncEditorCameraToScene()
    // in onUpdate, so the standard scene render path produces the correct view.
    _renderer->Render(_scene.get(), curRT);

    // ImGui composites on top of the swapchain via Load action.
    onRenderGUI();

    cmdBuffer->End();
    _rhiContext->Submit(_swapchain, &cmdBuffer, 1);
}

void SimpleWindow::onPresent()
{
    if (!_shouldPresent)
    {
        return;
    }
    RHIContext::Get()->Present(_swapchain);
}

void SimpleWindow::onShutdown()
{
    ImGuiExtension::FinalizeBackend();

    if (_renderer)
    {
        _renderer->Shutdown();
        _renderer.reset();
    }

    if (_inspectorPanel)
    {
        _inspectorPanel->Cleanup();
        _inspectorPanel.reset();
    }

    EditorContext::Get().SetActiveScene(nullptr);
    _camera.reset();
    _scene.reset();
}

GUIContext* SimpleWindow::GetGUIContext()
{
    return static_cast<SimpleApplication*>(_ownerApp)->GetGUIContext();
}

void SimpleWindow::setupDefaultScene()
{
    _scene = MakeScoped<Scene>("Simple Scene");
    EditorContext::Get().SetActiveScene(_scene.get());

    // Camera entity
    Entity cameraEntity;
    {
        cameraEntity = _scene->CreateEntity("Camera");
        auto& camera = cameraEntity.AddComponent<CameraComponent>();
        camera.isPrimary = true;
    }

    // Main light (sun-style directional). Rotation is angle-based; the
    // renderer derives the light direction from transform.forward as long as
    // LightComponent.direction is left at the zero default. -60° pitch +
    // 30° yaw points the light roughly down with a slant into the atrium.
    {
        _directionalLightEntity = _scene->CreateEntity("Directional Light");
        auto& light     = _directionalLightEntity.AddComponent<LightComponent>();
        auto& transform = _directionalLightEntity.GetComponent<TransformComponent>();
        transform.SetPosition(glm::vec3(1.0f, 5.0f, 1.0f));
        transform.SetEulerAngles(glm::vec3(-60.0f, 30.0f, 0.0f));
        light.intensity = 5.0f;
    }

    // Load Sponza (multi-submesh GLTF). Meshes/materials are owned by SimpleWindow
    // because ObjectManager only caches single-(mesh,material) Models today.
    {
        std::string gltfPath = std::string("GLTF") + HS_DIR_SEPERATOR
                             + "Sponza" + HS_DIR_SEPERATOR
                             + "Sponza.gltf";

        if (ObjectManager::LoadModel(gltfPath, _ownedMeshes, _ownedMaterials))
        {
            for (size_t i = 0; i < _ownedMeshes.size(); ++i)
            {
                Mesh* mesh = _ownedMeshes[i].get();
                if (!mesh) continue;

                int32 matIdx    = mesh->GetMaterialIndex();
                Material* mat   = (matIdx >= 0 && static_cast<size_t>(matIdx) < _ownedMaterials.size())
                                    ? _ownedMaterials[matIdx].get()
                                    : nullptr;

                std::string entityName = "Sponza_" + std::to_string(i);
                Entity entity = _scene->CreateEntity(entityName.c_str());

                auto& meshRenderer = entity.AddComponent<MeshRendererComponent>();
                meshRenderer.mesh = mesh;
                if (mat)
                {
                    meshRenderer.materials.push_back(mat);
                }

                const auto& bound = mesh->GetBound();
                meshRenderer.localBounds = AABB(glm::vec3(bound.min), glm::vec3(bound.max));
            }
        }
        else
        {
            HS_LOG(error, "Failed to load Sponza GLTF: %s", gltfPath.c_str());
        }
    }

    _scene->Update(0.0f);

    // Wire the inspector now that the entities exist. The light is the default
    // gizmo target so the user can manipulate it visually.
    if (_inspectorPanel)
    {
        if (cameraEntity.IsValid())
        {
            _inspectorPanel->SetMainCamera(cameraEntity);
        }
        if (_directionalLightEntity.IsValid())
        {
            _inspectorPanel->SetMainLight(_directionalLightEntity);
            _inspectorPanel->SetTarget(_directionalLightEntity);
        }
    }
}

void SimpleWindow::syncEditorCameraToScene()
{
    if (!_scene || !_camera) return;

    Entity cameraEntity = _scene->GetPrimaryCamera();
    if (!cameraEntity.IsValid()) return;

    auto& transform = cameraEntity.GetComponent<TransformComponent>();
    auto& camera = cameraEntity.GetComponent<CameraComponent>();

    const auto& editorTransform = _camera->GetTransform();
    const auto& editorCameraComp = _camera->GetCameraComponent();

    transform.SetPosition(editorTransform.position);
    transform.SetRotation(editorTransform.rotation);
    bool wasPrimary = camera.isPrimary;
    bool wasActive = camera.isActive;
    camera = editorCameraComp;
    camera.isPrimary = wasPrimary;
    camera.isActive = wasActive;
}

void SimpleWindow::processCameraInput(float deltaTime)
{
    // Block camera input while gizmo is being manipulated
    if (ImGuizmo::IsUsing())
    {
        return;
    }

    static constexpr float moveSpeedDecelFactor = 0.5f;

    bool isMoveDirectionUpdated = false;

    if (Input::IsPressed(Input::Button::MouseRight))
    {
        _rightClickActive = true;

        // Mouse look
        uint16 mouseX, mouseY;
        Input::GetMousePosition(mouseX, mouseY);

        if (_isMouseTracking)
        {
            float dx = static_cast<float>(mouseX) - static_cast<float>(_lastMouseX);
            float dy = static_cast<float>(mouseY) - static_cast<float>(_lastMouseY);

            if (dx != 0.0f || dy != 0.0f)
            {
                float rotateSpeed = _camera->GetRotateSpeed();
                _camera->Rotate(dx * rotateSpeed, -dy * rotateSpeed);
            }
        }

        _lastMouseX = mouseX;
        _lastMouseY = mouseY;
        _isMouseTracking = true;

        // Keyboard movement (WASD + EQ)
        int front = 0, right = 0, up = 0;
        if (Input::IsPressed(Input::Button::W)) front++;
        if (Input::IsPressed(Input::Button::S)) front--;
        if (Input::IsPressed(Input::Button::D)) right++;
        if (Input::IsPressed(Input::Button::A)) right--;
        if (Input::IsPressed(Input::Button::E)) up++;
        if (Input::IsPressed(Input::Button::Q)) up--;

        if (front != 0 || right != 0 || up != 0)
        {
            _moveDir = _camera->GetForward() * static_cast<float>(front) +
                       _camera->GetRight() * static_cast<float>(right) +
                       glm::vec3(0.0f, 1.0f, 0.0f) * static_cast<float>(up);
            isMoveDirectionUpdated = true;
        }
    }
    else
    {
        _isMouseTracking = false;
        _rightClickActive = false;
    }

    if (isMoveDirectionUpdated)
    {
        _currentCameraSpeed = _camera->GetMoveSpeed();
    }
    else
    {
        _currentCameraSpeed = std::max(_currentCameraSpeed - moveSpeedDecelFactor, 0.0f);
    }

    if (Math::EpsilonEqual(_currentCameraSpeed, 0.0f))
    {
        return;
    }

    _camera->Move(_moveDir * deltaTime * _currentCameraSpeed);
}

void SimpleWindow::onRenderGUI()
{
    GUIContext* guiContext = GetGUIContext();
    guiContext->BeginRender(_swapchain);

    drawHelperOverlayGUI();
    if (_inspectorPanel)
    {
        _inspectorPanel->Draw();
    }
    _menuPanel->Draw();

    guiContext->EndRender();
}

void SimpleWindow::drawHelperOverlayGUI()
{
    // The scene is already rendered directly into the swapchain backbuffer by onRender().
    // ImGui composites on top via its Load action, so no fullscreen quad is needed here.
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    ImVec2 viewportPos  = mainViewport->Pos;

    // Stats overlay
    ImGui::SetNextWindowPos(ImVec2(viewportPos.x + 10, viewportPos.y + 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::Begin("Stats", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_AlwaysAutoResize |
                 ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoFocusOnAppearing |
                 ImGuiWindowFlags_NoNav);
    
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("FPS: %.1f (%.2f ms)", io.Framerate, 1000.0f / io.Framerate);
    ImGui::Text("Resolution: %ux%u", _swapchain->GetWidth(), _swapchain->GetHeight());
    ImGui::Separator();
    ImGui::Text("RMB + WASD: Move Camera");
    ImGui::Text("RMB + Mouse: Look Around");
    
    ImGui::End();
}

HS_NS_EDITOR_END
