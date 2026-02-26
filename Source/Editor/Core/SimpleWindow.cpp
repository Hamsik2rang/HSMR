#include "Editor/Core/SimpleWindow.h"

#include "Core/HAL/Input.h"
#include "Core/Math/Common.h"

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

    for (auto& renderTarget : _renderTargets)
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
    RenderTarget* curRT = &_renderTargets[imageIndex];

    // Render scene to offscreen RT
    _renderer->Render(_scene.get(), curRT);

    // Render ImGui overlay (scene displayed fullscreen + overlay widgets)
    renderOverlay();

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
    Entity cameraEntity = _scene->CreateEntity("Camera");
    auto& camera = cameraEntity.AddComponent<CameraComponent>();
    camera.isPrimary = true;

    // Test mesh entity
    Entity entity = _scene->CreateEntity("Damaged Helmet");
    entity.GetComponent<TransformComponent>().SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    entity.AddComponent<MeshRendererComponent>();

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
    camera = editorCameraComp;
}

void SimpleWindow::processCameraInput(float deltaTime)
{
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

void SimpleWindow::renderOverlay()
{
    GUIContext* guiContext = GetGUIContext();
    guiContext->BeginRender(_swapchain);

    // Display scene as fullscreen background
    uint8 imageIndex = _swapchain->GetCurrentImageIndex();
    RenderTarget* curRT = &_renderTargets[imageIndex];
    RHITexture* sceneTexture = curRT->GetColorTexture(0);

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(displaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##SceneViewport", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);

    ImGuiExtension::ImageOffscreen(sceneTexture, displaySize);
    ImGui::End();
    ImGui::PopStyleVar();

    // Stats overlay
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
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

    guiContext->EndRender();
}

HS_NS_EDITOR_END
