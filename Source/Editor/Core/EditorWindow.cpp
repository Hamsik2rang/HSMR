#include "Editor/Core/EditorWindow.h"

#include "Renderer/RenderPass/Forward/ForwardOpaquePass.h"
#include "RHI/Swapchain.h"
#include "RHI/RenderHandle.h"
#include "Renderer/ForwardPath.h"

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
{}

void EditorWindow::NextFrame()
{
	onNextFrame();
}

void EditorWindow::Render()
{
	onRender();
}

void EditorWindow::ProcessEvent()
{
	EWindowEvent event;
	while (PeekNativeEvent(&_nativeWindow, event))
	{
		switch (event)
		{
		case EWindowEvent::OPEN:
		{
			_shouldClose = false;

			break;
		}
		case EWindowEvent::CLOSE:
		{
			_shouldClose = true;
			_shouldUpdate = false;
			_shouldPresent = false;

			break;
		}
		case EWindowEvent::MAXIMIZE:
		{
			_shouldUpdate = true;
			_shouldPresent = true;
			onRestore();
			
			break;
		}
		case EWindowEvent::MINIMIZE:
		{
			_shouldUpdate = false;
			_shouldPresent = false;
			
			break;
		}
		case EWindowEvent::RESIZE:
		{
			onSuspend();
			onRestore();
			break;
		}
		case EWindowEvent::MOVE_ENTER:
		{
			_shouldUpdate = false;
			_shouldPresent = false;
			onSuspend();

			break;
		}
		case EWindowEvent::MOVE_EXIT:
		case EWindowEvent::RESTORE:
		{
			_shouldUpdate = true;
			_shouldPresent = true;
			onRestore();
			
			break;
		}
		case EWindowEvent::MOVE:
		{

			break;
		}
		case EWindowEvent::FOCUS_IN:
		{

			break;
		}
		case EWindowEvent::FOCUS_OUT:
		{

			break;
		}
		default:
			break;
		}
	}

	if (_shouldClose)
	{
		Flush();
		return;
	}

	for (auto* child : _childs)
	{
		child->ProcessEvent();
	}
}

bool EditorWindow::onInitialize()
{
	SwapchainInfo scInfo{};
	scInfo.nativeWindow = &_nativeWindow;
	scInfo.useDepth = false;
	scInfo.useMSAA = false;
	scInfo.useStencil = false;
	
	_rhiContext = RHIContext::Get();

	_swapchain = _rhiContext->CreateSwapchain(scInfo);

	_renderer = MakeScoped<ForwardRenderer>(_rhiContext);
	_renderer->Initialize();

	ImGuiExtension::InitializeBackend(_swapchain);

	_renderer->AddPass(new ForwardOpaquePass("Opaque Pass", _renderer.get(), ERenderingOrder::OPAQUE));

	_renderTargets.resize(_swapchain->GetMaxFrameCount());

	uint32 width = _swapchain->GetWidth();
	uint32 height = _swapchain->GetHeight();
	for (size_t i = 0; i < _renderTargets.size(); i++)
	{
		RenderTargetInfo info = _renderer->GetBareboneRenderTargetInfo();
		for (size_t j = 0; j < info.colorTextureCount; j++)
		{
			info.colorTextureInfos[j].extent.width = width;
			info.colorTextureInfos[j].extent.height = height;
			info.colorTextureInfos[j].format = EPixelFormat::R8G8B8A8_SRGB;
			info.colorTextureInfos[j].usage = ETextureUsage::COLOR_ATTACHMENT | ETextureUsage::STAGING | ETextureUsage::SAMPLED;
			info.colorTextureInfos[j].isCompressed = false;
			info.colorTextureInfos[j].byteSize = 4 * width * height * 1 /*depth*/;
		}

		info.useDepthStencilTexture = true;
		info.depthStencilInfo.extent.width = width;
		info.depthStencilInfo.extent.height = height;
		info.depthStencilInfo.extent.depth = 1;
		info.depthStencilInfo.format = EPixelFormat::DEPTH32;
		info.depthStencilInfo.usage = ETextureUsage::DEPTH_STENCIL_ATTACHMENT | ETextureUsage::STAGING;
		info.depthStencilInfo.isDepthStencilBuffer = true;
		info.depthStencilInfo.isCompressed = false;

		_renderTargets[i].Create(info);
	}

	setupPanels();

	return true;
}

void EditorWindow::onNextFrame()
{
	if (false == _nativeWindow.shouldRender)
	{
		return;
	}

	_renderer->NextFrame(_swapchain);

	Resolution resolution = static_cast<ScenePanel*>(_scenePanel.get())->GetResolution();
	uint32     width = static_cast<uint32>(resolution.width / _nativeWindow.scale);
	uint32     height = static_cast<uint32>(resolution.height / _nativeWindow.scale);

	for (auto& renderTarget : _renderTargets)
	{
		renderTarget.Update(resolution.width, resolution.height);
	}
}

void EditorWindow::onUpdate()
{

}

void EditorWindow::onResize()
{

}

void EditorWindow::onRender()
{
	if (false == _nativeWindow.shouldRender)
	{
		return;
	}
	RHICommandBuffer* cmdBuffer = _swapchain->GetCommandBufferForCurrentFrame();
	cmdBuffer->Begin();

	uint8         frameIndex = _swapchain->GetCurrentFrameIndex();
	RenderTarget* curRT = &_renderTargets[frameIndex];

	//     1. Render Scene to Scene Panel
	_renderer->Render({}, curRT);

	static_cast<ScenePanel*>(_scenePanel.get())->SetSceneRenderTarget(&_renderTargets[frameIndex]);

	// 2. Render GUI
	onRenderGUI();

	cmdBuffer->End();

	_rhiContext->Submit(_swapchain, &cmdBuffer, 1);
}

void EditorWindow::onPresent()
{
	if (!_nativeWindow.shouldRender)
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
		_renderer.reset();  // Automatic cleanup with Scoped<>
	}
}

void EditorWindow::onRenderGUI()
{
	// TODO: 어차피 필요하니 스왑체인이 렌더패스 핸들을 들고있도록 하고 이 함수가 인자로 렌더패스 핸들을 받도록 하기
	ImGuiExtension::BeginRender(_swapchain);

	_basePanel->Draw(); // Draw panel tree.

	ImGuiExtension::EndRender();
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
}


HS_NS_EDITOR_END
