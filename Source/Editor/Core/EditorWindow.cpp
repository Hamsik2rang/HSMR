#include "Editor/Core/EditorWindow.h"

#include "Engine/RendererPass/ForwardPass/ForwardOpaquePass.h"
#include "Engine/Renderer/Swapchain.h"
#include "Engine/Renderer/Framebuffer.h"
#include "Engine/Renderer/RenderPass.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/GUI/GUIRenderer.h"

#include "Editor/Panel/Panel.h"
#include "Editor/Panel/DockspacePanel.h"
#include "Editor/Panel/MenuPanel.h"
#include "Editor/Panel/ScenePanel.h"


HS_NS_EDITOR_BEGIN

EditorWindow::EditorWindow(const char* name, uint32 width, uint32 height, uint64 flags)
    : Window(name, width, height, flags)
{
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
    _guiRenderer = new GUIRenderer();
    _guiRenderer->Initialize(&_nativeHandle);

    _guiRenderer->AddPass(new ForwardOpaquePass("Opaque Pass", _guiRenderer, ERenderingOrder::OPAQUE));
    
    for (int i = 0; i < (sizeof(_renderTextures) / sizeof(Framebuffer*)); i++)
    {
        RenderPassInfo rpInfo{};
        rpInfo.colorAttachments.resize(1);
        rpInfo.colorAttachmentCount = 1;
        rpInfo.useDepthStencilAttachment = false;
        
        RenderPass* renderPass = new RenderPass(rpInfo);
        
        FramebufferInfo fbInfo;
        fbInfo.width = _width;
        fbInfo.height = _height;
        fbInfo.renderPass = renderPass;
        fbInfo.resolveBuffer = nullptr;
        fbInfo.depthStencilBuffer = nullptr;
        fbInfo.colorBuffers.emplace_back(new Texture(nullptr, _width, _height, EPixelFormat::R8G8B8A8_UNORM, ETextureType::TEX_2D, ETextureUsage::SHADER_WRITE));
        fbInfo.isSwapchainFramebuffer = false;
       
        _renderTextures[i] = new Framebuffer(fbInfo);
    }

    setupPanels();

    //    _renderer->AddPass(new ForwardOpaquePass("Opaque Pass", _renderer, 0));
}

void EditorWindow::onNextFrame()
{
    _guiRenderer->NextFrame(_swapchain);
    
}

void EditorWindow::onUpdate()
{
    //....
}

void EditorWindow::onRender()
{
    Framebuffer* currentFramebuffer = _renderTextures[_swapchain->GetCurrentFrameCount()];
    // 1. Render Scene to Scene Panel
    _guiRenderer->Render({}, currentFramebuffer);

    static_cast<ScenePanel*>(_scenePanel)->SetSceneRenderFramebuffer(currentFramebuffer);
    
    // 2. Render GUI
    onRenderGUI();
}

void EditorWindow::onPresent()
{
    _guiRenderer->Present(_swapchain);
}
void EditorWindow::onShutdown()
{
    if (nullptr != _guiRenderer)
    {
        _guiRenderer->Shutdown();
        delete _guiRenderer;
        _guiRenderer = nullptr;
    }
}

void EditorWindow::onRenderGUI()
{
    //    _guiRenderer->Render({}, nullptr);

    _basePanel->Draw(); // Draw panel tree.

    _guiRenderer->RenderGUI();
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

HS_NS_EDITOR_END
