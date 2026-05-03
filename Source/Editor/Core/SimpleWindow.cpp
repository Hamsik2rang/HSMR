#include "Editor/Core/SimpleWindow.h"

#include "Core/HAL/Input.h"
#include "Core/Math/Common.h"
#include "ThirdParty/ImGuizmo/ImGuizmo.h"

#include "RHI/Swapchain.h"
#include "RHI/RenderHandle.h"
#include "RHI/CommandHandle.h"

#include "Renderer/ForwardRenderer.h"
#include "Renderer/RenderPass/ForwardOpaquePass.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

#include "Editor/GUI/ImGuiExtension.h"
#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/SimpleApplication.h"
#include "Editor/Core/EditorCamera.h"


#include "Resource/ObjectManager.h"

HS_NS_EDITOR_BEGIN

SimpleWindow::SimpleWindow(Application* ownerApp, const char* name, uint32 width, uint32 height, EWindowFlags flags)
    : Window(ownerApp, name, static_cast<uint16>(width), static_cast<uint16>(height), flags)
{
    onInitialize();
}

SimpleWindow::~SimpleWindow()
{}

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

    auto* opaquePass = new ForwardOpaquePass("Forward Opaque Pass", _renderer.get(), ERenderingOrder::Opaque);
    _renderer->AddPass(std::move(opaquePass));

    // EditorCamera setup
    _camera = MakeScoped<EditorCamera>();
    float aspect = static_cast<float>(_nativeWindow.surfaceWidth) / static_cast<float>(_nativeWindow.surfaceHeight);
    _camera->SetAspectRatio(aspect);

    // NOTE: setupDefaultScene()에서 inspectorPanel을 사용하기 때문에 Initialize순서가 바뀌면 안된다.
    _inspectorPanel = MakeScoped<SimpleInspectorPanel>(this);
    _inspectorPanel->Setup();
    _inspectorPanel->SetEditorCamera(_camera.get());

    _menuPanel = MakeScoped<MenuPanel>(this);
    _menuPanel->Setup();

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

    // RT is swapchain-backed: scene draws directly into the swapchain backbuffer (Clear).
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

    // Camera entity
    {
        Entity cameraEntity = _scene->CreateEntity("Camera");
        auto& camera = cameraEntity.AddComponent<CameraComponent>();
        camera.isPrimary = true;
        
        _inspectorPanel->SetMainCamera(cameraEntity);
    }
    
    // Main light(Directional) entity
    {
        Entity lightEntity = _scene->CreateEntity("Directional Light");
        auto& light = lightEntity.AddComponent<LightComponent>();
        auto& transform = lightEntity.GetComponent<TransformComponent>();
        transform.SetPosition(glm::vec3(1.0f, 5.0f, 1.0f));
        
        _inspectorPanel->SetMainLight(lightEntity);
    }
    
    // Load DamagedHelmet model
    {
        std::string gltfPath = std::string("GLTF") + HS_DIR_SEPERATOR
                             + "DamagedHelmet" + HS_DIR_SEPERATOR
                             + "DamagedHelmet.gltf";
        auto [mesh, material] = ObjectManager::LoadModel(gltfPath);

        // Damaged Helmet entity
        Entity entity = _scene->CreateEntity("Damaged Helmet");
        if (mesh)
        {
            auto& meshRenderer = entity.AddComponent<MeshRendererComponent>();
            meshRenderer.mesh = mesh;
            if (material)
            {
                meshRenderer.materials.push_back(material);
            }

            // Set local bounds from mesh
            const auto& bound = mesh->GetBound();
            meshRenderer.localBounds = AABB(glm::vec3(bound.min), glm::vec3(bound.max));
        }
        
        _inspectorPanel->SetTarget(entity);
    }

    _scene->Update(0.0f);
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
//    _menuPanel->Draw();
    _inspectorPanel->Draw();
    
    guiContext->EndRender();
}

void SimpleWindow::drawHelperOverlayGUI()
{
    // The scene is already rendered directly into the swapchain backbuffer by onRender().
    // ImGui composites on top via its Load action, so no fullscreen quad is needed here.
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    ImVec2 viewportPos  = mainViewport->Pos;

    // Stats overlay
    ImGui::SetNextWindowPos(ImVec2(viewportPos.x + 10, viewportPos.y + 10), ImGuiCond_FirstUseEver);
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
