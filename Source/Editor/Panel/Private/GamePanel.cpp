#include "Editor/Panel/GamePanel.h"

#include "Editor/Core/EditorContext.h"
#include "Editor/GUI/EditorFeedbackWidgets.h"
#include "Editor/GUI/EditorIcons.h"
#include "Editor/GUI/ImGuiExtension.h"
#include "Editor/Panel/EditorPanelFrame.h"

#include "RHI/Swapchain.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

#include "ImGui/imgui.h"

#include <vector>

HS_NS_EDITOR_BEGIN

namespace
{
std::vector<Entity> collectSceneCameras(Scene* scene)
{
    std::vector<Entity> cameras;
    if (!scene)
    {
        return cameras;
    }

    auto view = scene->GetRegistry().view<CameraComponent>();
    for (auto [entity, camera] : view.each())
    {
        cameras.emplace_back(entity, scene);
    }

    std::stable_sort(
        cameras.begin(),
        cameras.end(),
        [](const Entity& lhs, const Entity& rhs) -> bool
        {
            const CameraComponent& lhsCamera = lhs.GetComponent<CameraComponent>();
            const CameraComponent& rhsCamera = rhs.GetComponent<CameraComponent>();

            if (lhsCamera.isActive != rhsCamera.isActive)
            {
                return lhsCamera.isActive && !rhsCamera.isActive;
            }

            if (lhsCamera.priority != rhsCamera.priority)
            {
                return lhsCamera.priority > rhsCamera.priority;
            }

            if (lhsCamera.isPrimary != rhsCamera.isPrimary)
            {
                return lhsCamera.isPrimary && !rhsCamera.isPrimary;
            }

            return entt::to_integral(lhs.GetHandle()) < entt::to_integral(rhs.GetHandle());
        });

    return cameras;
}
}

GamePanel::GamePanel(Window* window)
    : Panel(window, "Game")
    , _resolution(800, 600)
{
}

GamePanel::~GamePanel()
{
}

bool GamePanel::Setup()
{
    Swapchain* swapchain = _window->GetSwapchain();
    HS_CHECK(swapchain != nullptr, "GamePanel::Setup requires a window with a valid swapchain");

    const uint8 frameCount = swapchain->GetMaxFrameCount();
    _panelRenderTargets.resize(frameCount);

    RenderTargetInfo info{};
    info.width  = _resolution.width;
    info.height = _resolution.height;
    info.colorTextureCount = 1;
    info.colorTextureInfo.resize(1);
    info.colorTextureInfo[0].arrayLength   = 1;
    info.colorTextureInfo[0].extent.width  = _resolution.width;
    info.colorTextureInfo[0].extent.height = _resolution.height;
    info.colorTextureInfo[0].extent.depth  = 1;
    info.colorTextureInfo[0].format        = EPixelFormat::R8G8B8A8Srgb;
    info.colorTextureInfo[0].usage         = ETextureUsage::ColorAttachment | ETextureUsage::Sampled;
    info.colorTextureInfo[0].isCompressed  = false;
    info.colorTextureInfo[0].byteSize      = 4 * _resolution.width * _resolution.height;

    info.useDepthStencilTexture                = true;
    info.depthStencilInfo.arrayLength          = 1;
    info.depthStencilInfo.extent.width         = _resolution.width;
    info.depthStencilInfo.extent.height        = _resolution.height;
    info.depthStencilInfo.extent.depth         = 1;
    info.depthStencilInfo.format               = EPixelFormat::Depth32;
    info.depthStencilInfo.usage                = ETextureUsage::DepthStencilAttachment;
    info.depthStencilInfo.isDepthStencilBuffer = true;
    info.depthStencilInfo.isCompressed         = false;

    info.isSwapchainTarget = false;
    info.swapchain         = nullptr;

    for (RenderTarget& rt : _panelRenderTargets)
    {
        rt.Create(info);
    }

    return true;
}

void GamePanel::Cleanup()
{
    for (RenderTarget& rt : _panelRenderTargets)
    {
        rt.Clear();
    }
    _panelRenderTargets.clear();
}

void GamePanel::Update(float /*deltaTime*/)
{
    if (_resolution.width == 0 || _resolution.height == 0)
    {
        return;
    }

    for (RenderTarget& rt : _panelRenderTargets)
    {
        rt.Update(_resolution.width, _resolution.height);
    }
}

RenderTarget* GamePanel::GetRenderTarget(uint32 imageIndex)
{
    if (_panelRenderTargets.empty() || imageIndex >= _panelRenderTargets.size())
    {
        return nullptr;
    }
    return &_panelRenderTargets[imageIndex];
}

Entity GamePanel::ResolveCamera(Scene* scene) const
{
    if (!scene)
    {
        return Entity();
    }

    if (!_useAutoCamera &&
        _selectedCamera.IsValid() &&
        _selectedCamera.GetScene() == scene &&
        _selectedCamera.HasComponent<CameraComponent>())
    {
        return _selectedCamera;
    }

    return scene->GetBestGameCamera();
}

std::string GamePanel::getCameraLabel(Entity camera) const
{
    if (!camera.IsValid() || !camera.HasComponent<TagComponent>())
    {
        return "No Camera";
    }

    std::string label = camera.GetComponent<TagComponent>().name;
    if (camera.HasComponent<CameraComponent>())
    {
        const CameraComponent& cameraComponent = camera.GetComponent<CameraComponent>();
        label += " [P=" + std::to_string(cameraComponent.priority) + "]";
    }

    return label;
}

void GamePanel::drawMenuBar(Scene* scene)
{
    if (!EditorPanelFrame::BeginPanelMenuBar())
    {
        return;
    }

    ImGui::TextUnformatted("Game");
    ImGui::SameLine();

    bool useAutoCamera = _useAutoCamera;
    if (ImGui::Checkbox("Auto", &useAutoCamera))
    {
        _useAutoCamera = useAutoCamera;
        if (_useAutoCamera)
        {
            _selectedCamera = Entity();
        }
    }

    ImGui::SameLine();

    Entity resolvedCamera = ResolveCamera(scene);
    std::string previewLabel = _useAutoCamera
        ? ("Auto: " + getCameraLabel(resolvedCamera))
        : getCameraLabel(_selectedCamera);

    ImGui::SetNextItemWidth(220.0f);
    if (ImGui::BeginCombo("##GameCamera", previewLabel.c_str()))
    {
        if (scene)
        {
            std::vector<Entity> cameras = collectSceneCameras(scene);
            for (const Entity& camera : cameras)
            {
                bool isSelected = !_useAutoCamera && camera == _selectedCamera;
                std::string label = getCameraLabel(camera);

                if (camera.HasComponent<CameraComponent>() && !camera.GetComponent<CameraComponent>().isActive)
                {
                    label += " (Inactive)";
                }

                if (ImGui::Selectable(label.c_str(), isSelected))
                {
                    _selectedCamera = camera;
                    _useAutoCamera = false;
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }

        ImGui::EndCombo();
    }

    EditorPanelFrame::EndPanelMenuBar();
}

void GamePanel::Draw()
{
    if (!IsVisible())
    {
        return;
    }

    EditorPanelWindowOptions panelOptions{};
    panelOptions.pOpen = GetVisibilityBinding();
    panelOptions.useMenuBar = true;
    panelOptions.noTitleBar = true;
    panelOptions.noScrollbar = true;
    panelOptions.noScrollWithMouse = true;
    EditorPanelFrame::BeginStandardPanel("Game", panelOptions);

    Scene* scene = EditorContext::Get().GetActiveScene();
    drawMenuBar(scene);

    EditorPanelContentOptions contentOptions{};
    contentOptions.id = "GameViewport";
    contentOptions.padding = ImVec2(0.0f, 0.0f);
    contentOptions.extraFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    EditorPanelFrame::BeginPanelContent(contentOptions);

    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    _resolution.width = static_cast<uint32>(availableSize.x > 1.0f ? availableSize.x : 1.0f);
    _resolution.height = static_cast<uint32>(availableSize.y > 1.0f ? availableSize.y : 1.0f);

    Swapchain* swapchain = _window->GetSwapchain();
    const uint8 imageIndex = swapchain ? swapchain->GetCurrentImageIndex() : 0;
    RenderTarget* currentRT = GetRenderTarget(imageIndex);
    if (currentRT)
    {
        ImVec2 viewportSize(
            static_cast<float>(currentRT->GetWidth()),
            static_cast<float>(currentRT->GetHeight()));
        ImGuiExtension::ImageOffscreen(currentRT->GetColorTexture(0), viewportSize);
    }
    else
    {
        ImGui::Dummy(availableSize);
    }

    if (!ResolveCamera(scene).IsValid())
    {
        ImVec2 overlayPos = ImGui::GetWindowPos();
        overlayPos.x += 12.0f;
        overlayPos.y += ImGui::GetFrameHeight() + 12.0f;
        ImGui::SetCursorScreenPos(overlayPos);
        const std::string message = std::string(EditorIcons::Camera) + " No active scene camera";
        EditorFeedbackWidgets::SecondaryText(message.c_str());
    }

    EditorPanelFrame::EndPanelContent();
    EditorPanelFrame::EndStandardPanel();
}

HS_NS_EDITOR_END
