#include "Editor/GUI/ImGuiExtension.h"

#include "Engine/Core/Log.h"
#include "Engine/RHI/ResourceHandle.h"
#include "Engine/RHI/Vulkan/SwapchainVulkan.h"
#include "Engine/RHI/Vulkan/RHIContextVulkan.h"
#include "Engine/RHI/Vulkan/RenderHandleVulkan.h"

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

#include "Engine/Platform/Windows/PlatformWindowWindows.h"

using namespace HS;

namespace ImGuiExt
{

static VkPipelineCache s_pipelineCacheVk;

static void check_vk_result(VkResult result)
{
	if (result == VK_SUCCESS)
	{
		return;
	}

	if (result < 0)
	{
		switch (result)
		{
		case VK_NOT_READY:
			HS_LOG(crash, "ImGuiBackendVulkan: VK_NOT_READY");
			break;
		case VK_TIMEOUT:
			HS_LOG(crash, "ImGuiBackendVulkan: VK_TIMEOUT");
			break;
		case VK_EVENT_SET:
			HS_LOG(crash, "ImGuiBackendVulkan: VK_EVENT_SET");
			break;
		case VK_EVENT_RESET:
			HS_LOG(crash, "ImGuiBackendVulkan: VK_EVENT_RESET");
			break;
		case VK_INCOMPLETE:
			HS_LOG(crash, "ImGuiBackendVulkan: VK_INCOMPLETE");
			break;
		default:
			HS_LOG(crash, "ImGuiBackendVulkan: Unknown Vulkan error: %d", result);
			break;
		}
	}
}

HS_EDITOR_API uint64 s_test = 0;

void ImageOffscreen(HS::Texture* use_texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	static uint64 i = 0;
	HS_LOG(debug, "IMAGE OFFSCREEN!!");

	s_test = i++;
}

void InitializeBackend(HS::Swapchain* swapchain)
{
	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	const NativeWindow* nativeWindow = swapchainVK->GetInfo().nativeWindow;
	HWND hWnd = (HWND)nativeWindow->handle;

	ImGui_ImplWin32_Init(hWnd);

	RHIContextVulkan* rhiContextVK = static_cast<RHIContextVulkan*>(hs_engine_get_rhi_context());
	const RHIDeviceVulkan* rhiDeviceVK = rhiContextVK->GetDevice();
	RenderPassVulkan* renderPassVK = static_cast<RenderPassVulkan*>(swapchainVK->GetRenderPass());


	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = rhiContextVK->GetInstance();
	initInfo.Device = rhiDeviceVK->logicalDevice;
	initInfo.PhysicalDevice = rhiDeviceVK->physicalDevice;
	initInfo.Queue = rhiDeviceVK->graphicsQueue;
	initInfo.QueueFamily = rhiDeviceVK->queueFamilyIndices.graphics;
	initInfo.DescriptorPoolSize = 64;
	initInfo.RenderPass = renderPassVK->handle;
	initInfo.PipelineCache = s_pipelineCacheVk;
	initInfo.CheckVkResultFn = check_vk_result;
	initInfo.ImageCount = static_cast<uint32>(swapchainVK->GetMaxFrameCount());
	initInfo.MinImageCount = 2;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT; // No MSAA for now

	ImGui_ImplVulkan_Init(&initInfo);
}

void ProcessEvent()
{

}

void BeginRender(HS::Swapchain* swapchain)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}

void EndRender(HS::Swapchain* swapchain)
{
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
}

void FinalizeBackend()
{
	hs_engine_get_rhi_context()->WaitForIdle();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();

	// Destroy the pipeline cache
	if (s_pipelineCacheVk != VK_NULL_HANDLE)
	{
		RHIContextVulkan* rhiContextVK = static_cast<RHIContextVulkan*>(hs_engine_get_rhi_context());
		vkDestroyPipelineCache(rhiContextVK->GetDevice()->logicalDevice, s_pipelineCacheVk, nullptr);
		s_pipelineCacheVk = VK_NULL_HANDLE;
	}	
}

}