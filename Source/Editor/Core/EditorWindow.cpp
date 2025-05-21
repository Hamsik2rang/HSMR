#include "Editor/Core/EditorWindow.h"

#include "Engine/RendererPass/Forward/ForwardOpaquePass.h"
#include "Engine/RHI/Swapchain.h"
#include "Engine/RHI/RenderHandle.h"
#include "Engine/Renderer/ForwardRenderer.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/GUI/ImGuiExtension.h"

#include "Editor/Panel/Panel.h"
#include "Editor/Panel/DockspacePanel.h"
#include "Editor/Panel/MenuPanel.h"
#include "Editor/Panel/ScenePanel.h"

HS_NS_EDITOR_BEGIN

EditorWindow::EditorWindow(const char* name, uint32 width, uint32 height, EWindowFlags flags)
    : Window(name, width, height, flags)
{
    onInitialize();
}

EditorWindow::~EditorWindow()
{
}

void EditorWindow::Render()
{
    onRender();
}

bool EditorWindow::onInitialize()
{
    _renderer = new ForwardRenderer(_rhiContext);
    _renderer->Initialize();

    ImGuiExt::InitializeBackend(_swapchain);

    _renderer->AddPass(new ForwardOpaquePass("Opaque Pass", _renderer, ERenderingOrder::OPAQUE));

    _renderTargets.resize(_swapchain->GetMaxFrameCount());

    uint32 width  = _swapchain->GetWidth();
    uint32 height = _swapchain->GetHeight();
    for (size_t i = 0; i < _renderTargets.size(); i++)
    {
        RenderTargetInfo info = _renderer->GetBareboneRenderTargetInfo();
        for (size_t j = 0; j < info.colorTextureCount; j++)
        {
            info.colorTextureInfos[j].extent.width  = width;
            info.colorTextureInfos[j].extent.height = height;
            info.colorTextureInfos[j].format        = EPixelFormat::R8G8B8A8_SRGB;
            info.colorTextureInfos[j].usage         = ETextureUsage::RENDER_TARGET | ETextureUsage::SHADER_READ;
            info.colorTextureInfos[j].isCompressed  = false;
            info.colorTextureInfos[j].byteSize      = 4 * width * height * 1 /*depth*/;
        }

        info.useDepthStencilTexture                = true;
        info.depthStencilInfo.extent.width         = width;
        info.depthStencilInfo.extent.height        = height;
        info.depthStencilInfo.extent.depth         = 1;
        info.depthStencilInfo.format               = EPixelFormat::DEPTH32;
        info.depthStencilInfo.usage                = ETextureUsage::RENDER_TARGET | ETextureUsage::SHADER_READ;
        info.depthStencilInfo.isDepthStencilBuffer = true;
        info.depthStencilInfo.isCompressed         = false;

        _renderTargets[i].Create(info);
    }

    setupPanels();

    return true;
}

void EditorWindow::onNextFrame()
{
    if (_nativeWindow.isMinimized)
    {
        return;
    }

    _renderer->NextFrame(_swapchain);

    Resolution resolution = static_cast<ScenePanel*>(_scenePanel)->GetResolution();
    uint32     width      = static_cast<uint32>(resolution.width / _nativeWindow.scale);
    uint32     height     = static_cast<uint32>(resolution.height / _nativeWindow.scale);

    for (auto& renderTarget : _renderTargets)
    {
        renderTarget.Update(resolution.width, resolution.height);
    }
}

void EditorWindow::onUpdate()
{
}

void EditorWindow::onRender()
{
    if (_nativeWindow.isMinimized)
    {
        return;
    }
    CommandBuffer* cmdBuffer = _swapchain->GetCommandBufferForCurrentFrame();
    cmdBuffer->Begin();

    uint8         frameIndex = _swapchain->GetCurrentFrameIndex();
    RenderTarget* curRT      = &_renderTargets[frameIndex];

    //     1. Render Scene to Scene Panel
    _renderer->Render({}, curRT);

    static_cast<ScenePanel*>(_scenePanel)->SetSceneRenderTarget(&_renderTargets[frameIndex]);

    // 2. Render GUI
    onRenderGUI();

    cmdBuffer->End();
}

void EditorWindow::onPresent()
{
    if (_nativeWindow.isMinimized)
    {
        return;
    }
    hs_engine_get_rhi_context()->Present(_swapchain);
}

void EditorWindow::onShutdown()
{
    ImGuiExt::FinalizeBackend();

    if (nullptr != _renderer)
    {
        _renderer->Shutdown();
        delete _renderer;
        _renderer = nullptr;
    }
}

void EditorWindow::onRenderGUI()
{
    // TODO: 어차피 필요하니 스왑체인이 렌더패스 핸들을 들고있도록 하고 이 함수가 인자로 렌더패스 핸들을 받도록 하기
    ImGuiExt::BeginRender(_swapchain);

    _basePanel->Draw(); // Draw panel tree.

    ImGuiExt::EndRender(_swapchain);
}

void EditorWindow::setupPanels()
{
    _basePanel = new DockspacePanel(this);
    _basePanel->Setup();

    _menuPanel = new MenuPanel(this);
    _menuPanel->Setup();
    _basePanel->InsertPanel(_menuPanel);

    _scenePanel = new ScenePanel(this);
    _scenePanel->Setup();
    _basePanel->InsertPanel(_scenePanel);
}

bool EditorWindow::dispatchEvent(EWindowEvent event, uint32 windowId)
{
    if (event == EWindowEvent::CLOSE)
    {
        _shouldClose = true;
        return false;
    }

    switch (event)
    {
        case EWindowEvent::MINIMIZE:
        {
            // handle._isMinimize;
        }
        break;
        case EWindowEvent::FOCUS_IN:
        {
        }
        break;
        case EWindowEvent::FOCUS_OUT:
        {
        }
        break;
        case EWindowEvent::RESTORE:
        {
            //...
        }
        break;
        case EWindowEvent::RESIZE_ENTER:
        {
            
        }
            break;
        case EWindowEvent::RESIZE_EXIT:
        {
//            int w, h;
//            SDL_GetWindowSize(window, &w, &h);
//            float scale = SDL_GetWindowDisplayScale(window);
//            _width      = static_cast<uint32>(w * scale);
//            _height     = static_cast<uint32>(h * scale);
//            _swapchain->SetWidth(_width);
//            _swapchain->SetHeight(_height);
//            ImGuiIO& io    = ImGui::GetIO();
//            io.DisplaySize = ImVec2(static_cast<float>(_width), static_cast<float>(_height));
        }
        break;
        //...
        default:
            break;
    }

    return true;
}

HS_NS_EDITOR_END
