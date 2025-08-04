#include "Editor/GUI/ImGuiExtension.h"

#include "Engine/Core/Log.h"
#include "Engine/RHI/ResourceHandle.h"
#include "Engine/RHI/Vulkan/SwapchainVulkan.h"
#include "Engine/RHI/Vulkan/RHIContextVulkan.h"
#include "Engine/RHI/Vulkan/RenderHandleVulkan.h"
#include "Engine/RHI/Vulkan/CommandHandleVulkan.h"
#include "Engine/RHI/Vulkan/ResourceHandleVulkan.h"

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

#include "Engine/Platform/Windows/PlatformWindowWindows.h"

#include <unordered_map>

using namespace HS;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


HS_NS_EDITOR_BEGIN

static VkPipelineCache s_pipelineCacheVk = VK_NULL_HANDLE;
static VkDescriptorPool s_descriptorPool = VK_NULL_HANDLE;
static SamplerVulkan* s_samplerVK;
static SwapchainVulkan* s_currentSwapchainVK;
static Swapchain* s_currentSwapchain = nullptr;

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

void ImGuiExtension::ImageOffscreen(HS::Texture* use_texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	TextureVulkan* textureVK = static_cast<TextureVulkan*>(use_texture);
	VkDescriptorSet dSet = ImGui_ImplVulkan_AddTexture(s_samplerVK->handle, textureVK->imageViewVk, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	ImGui::Image(reinterpret_cast<ImTextureID>(dSet), image_size, uv0, uv1, tint_col, border_col);


}

void ImGuiExtension::InitializeBackend(HS::Swapchain* swapchain)
{
	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	const NativeWindow* nativeWindow = swapchainVK->GetInfo().nativeWindow;
	HWND hWnd = (HWND)nativeWindow->handle;

	ImGui_ImplWin32_Init(hWnd);

	RHIContextVulkan* rhiContextVK = static_cast<RHIContextVulkan*>(hs_engine_get_rhi_context());
	const RHIDeviceVulkan* rhiDeviceVK = rhiContextVK->GetDevice();
	RenderPassVulkan* renderPassVK = static_cast<RenderPassVulkan*>(swapchainVK->GetRenderPass());
	HS::SamplerInfo samplerInfo{};
	samplerInfo.addressU = HS::EAddressMode::CLAMP_TO_BORDER;
	samplerInfo.addressV = HS::EAddressMode::CLAMP_TO_BORDER;
	samplerInfo.addressW = HS::EAddressMode::CLAMP_TO_BORDER;
	samplerInfo.isPixelCoordinate = false;
	samplerInfo.type = HS::ETextureType::TEX_2D;
	samplerInfo.magFilter = HS::EFilterMode::NEAREST;
	samplerInfo.minFilter = HS::EFilterMode::NEAREST;
	samplerInfo.mipmapMode = HS::EFilterMode::NEAREST;
	s_samplerVK = static_cast<SamplerVulkan*>(rhiContextVK->CreateSampler("ImGui Default Sampler", samplerInfo));

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = rhiContextVK->GetInstance();
	initInfo.Device = rhiDeviceVK->logicalDevice;
	initInfo.PhysicalDevice = rhiDeviceVK->physicalDevice;
	initInfo.Queue = rhiDeviceVK->graphicsQueue;
	initInfo.QueueFamily = rhiDeviceVK->queueFamilyIndices.graphics;
	initInfo.RenderPass = renderPassVK->handle;
	initInfo.PipelineCache = s_pipelineCacheVk;
	initInfo.CheckVkResultFn = check_vk_result;
	initInfo.ImageCount = static_cast<uint32>(swapchainVK->GetMaxFrameCount());
	initInfo.MinImageCount = 2;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT; // No MSAA for now
	{
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024},
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1024},
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 0;
		for (VkDescriptorPoolSize& pool_size : pool_sizes)
			pool_info.maxSets += pool_size.descriptorCount;
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		VkResult err = vkCreateDescriptorPool(rhiDeviceVK->logicalDevice, &pool_info, nullptr, &s_descriptorPool);
		check_vk_result(err);

		initInfo.DescriptorPool = s_descriptorPool;
		initInfo.DescriptorPoolSize = 0; // Use the created descriptor pool
	}


	ImGui_ImplVulkan_Init(&initInfo);

	// Is it work?
	HS::hs_platform_window_set_pre_event_handler(ImGui_ImplWin32_WndProcHandler);
}

void ImGuiExtension::BeginRender(HS::Swapchain* swapchain)
{
	RHIContextVulkan* rhiContextVK = static_cast<RHIContextVulkan*>(hs_engine_get_rhi_context());
	VkDevice logicalDevice = rhiContextVK->GetDevice()->logicalDevice;
	vkDeviceWaitIdle(logicalDevice);
	vkResetDescriptorPool(logicalDevice, s_descriptorPool, 0);

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Begin render pass for swapchain
	s_currentSwapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	CommandBuffer* cmdBuffer = swapchain->GetCommandBufferForCurrentFrame();
	RenderPass* renderPass = s_currentSwapchainVK->GetRenderPass();
	Framebuffer* framebuffer = s_currentSwapchainVK->GetFramebufferForCurrentFrame();

	Area area{ 0, 0, swapchain->GetWidth(), swapchain->GetHeight() };

	cmdBuffer->BeginRenderPass(renderPass, framebuffer, area);
}

void ImGuiExtension::EndRender(HS::Swapchain* swapchain)
{
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();

	// Actually render the ImGui draw data to the command buffer
	//SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	CommandBufferVulkan* cmdBufferVK = static_cast<CommandBufferVulkan*>(s_currentSwapchainVK->GetCommandBufferForCurrentFrame());

	ImGui_ImplVulkan_RenderDrawData(draw_data, cmdBufferVK->handle);

	// End the render pass
	cmdBufferVK->EndRenderPass();
}

void ImGuiExtension::FinalizeBackend()
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

HS_NS_EDITOR_END