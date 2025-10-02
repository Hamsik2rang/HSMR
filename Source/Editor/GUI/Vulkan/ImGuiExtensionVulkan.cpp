#include "Editor/GUI/ImGuiExtension.h"

#include "Core/Log.h"
#include "RHI/ResourceHandle.h"
#include "RHI/Vulkan/VulkanSwapchain.h"
#include "RHI/Vulkan/VulkanContext.h"
#include "RHI/Vulkan/VulkanRenderHandle.h"
#include "RHI/Vulkan/VulkanCommandHandle.h"
#include "RHI/Vulkan/VulkanResourceHandle.h"

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

#include "Engine/Window.h"
#include "HAL/Win/WinWindow.h"

#include <unordered_map>

using namespace HS;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


HS_NS_EDITOR_BEGIN

static VkPipelineCache s_pipelineCacheVk = VK_NULL_HANDLE;
static VkDescriptorPool s_descriptorPool = VK_NULL_HANDLE;
static SamplerVulkan* s_samplerVK;

Swapchain* ImGuiExtension::s_currentSwapchain = nullptr;
uint8 ImGuiExtension::s_currentFrameIndex = 0;
std::vector<std::vector<void*>> ImGuiExtension::s_AddedTexturesPerFrame;

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

void ImGuiExtension::ImageOffscreen(HS::RHITexture* use_texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	TextureVulkan* textureVK = static_cast<TextureVulkan*>(use_texture);
	VkDescriptorSet dSet = ImGui_ImplVulkan_AddTexture(s_samplerVK->handle, textureVK->imageViewVk, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	s_AddedTexturesPerFrame[s_currentFrameIndex].push_back(reinterpret_cast<void*>(dSet));

	ImGui::Image(reinterpret_cast<ImTextureID>(dSet), image_size, uv0, uv1, tint_col, border_col);
}

void ImGuiExtension::InitializeBackend(HS::Swapchain* swapchain)
{
	s_currentSwapchain = swapchain;
	s_currentFrameIndex = 0;
	s_AddedTexturesPerFrame.resize(s_currentSwapchain->GetMaxFrameCount());

	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	const NativeWindow* nativeWindow = swapchainVK->GetInfo().nativeWindow;
	HWND hWnd = (HWND)nativeWindow->handle;

	ImGui_ImplWin32_Init(hWnd);

	VulkanContext* rhiContextVK = static_cast<VulkanContext*>(RHIContext::Get());
	const VulkanDevice* rhiDeviceVK = rhiContextVK->GetDevice();
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
			// TODO: 지금은 대충 큰 사이즈의 풀을 할당한 후 이용하는데, 이러면 ImGui::AddTexture() 호출이 예측 불가능한 경우 크래시 발생 가능
			// 이상적으로는 RHI 백엔드의 ResourceSet을 Custom Allocator로 동적 할당해 제공해주고 Release(Lazy-Destroy)하는 방식으로 구현해야 함.
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024},
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1024},
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1024},
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
	}

	ImGui_ImplVulkan_Init(&initInfo);
}

void ImGuiExtension::SetProcessEventHandler(void** fnHandler)
{
	*fnHandler = static_cast<void*>(ImGui_ImplWin32_WndProcHandler);
}

void ImGuiExtension::BeginRender(HS::Swapchain* swapchain)
{
	VulkanContext* rhiContextVK = static_cast<VulkanContext*>(RHIContext::Get());

	if (swapchain != s_currentSwapchain)
	{
		// If the swapchain has changed, we need to clear the previous ImGui data
		clearDeletedSwapchainData();
		s_currentSwapchain = swapchain;
	}
	s_currentFrameIndex = swapchain->GetCurrentFrameIndex();
	HS_ASSERT(s_currentFrameIndex < s_AddedTexturesPerFrame.size(), "ImGuiExtension::BeginRender: Current frame index out of bounds");

	size_t rSetCount = s_AddedTexturesPerFrame[s_currentFrameIndex].size();
	if (rSetCount > 0)
	{
		for (size_t i = 0; i < rSetCount; i++)
		{
			void* rSet = s_AddedTexturesPerFrame[s_currentFrameIndex][i];
			if (rSet)
			{
				ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(rSet));
				s_AddedTexturesPerFrame[s_currentFrameIndex][i] = nullptr; // Clear the pointer after destruction
			}
		}
		s_AddedTexturesPerFrame[s_currentFrameIndex].clear();
	}

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiExtension::EndRender()
{
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();

	// Begin render pass for swapchain
	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(s_currentSwapchain);
	CommandBufferVulkan* cmdBufferVK = static_cast<CommandBufferVulkan*>(s_currentSwapchain->GetCommandBufferForCurrentFrame());
	RHIRenderPass* renderPass = swapchainVK->GetRenderPass();
	RHIFramebuffer* framebuffer = swapchainVK->GetFramebufferForCurrentFrame();

	Area area{ 0, 0, s_currentSwapchain->GetWidth(), s_currentSwapchain->GetHeight() };

	cmdBufferVK->BeginRenderPass(renderPass, framebuffer, area);

	ImGui_ImplVulkan_RenderDrawData(draw_data, cmdBufferVK->handle);

	cmdBufferVK->EndRenderPass();
}

void ImGuiExtension::FinalizeBackend()
{
	VulkanContext* rhiContextVK = static_cast<VulkanContext*>(RHIContext::Get());
	rhiContextVK->WaitForIdle();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();

	// Destroy the pipeline cache
	if (s_pipelineCacheVk != VK_NULL_HANDLE)
	{
		VulkanContext* rhiContextVK = static_cast<VulkanContext*>(RHIContext::Get());
		vkDestroyPipelineCache(rhiContextVK->GetDevice()->logicalDevice, s_pipelineCacheVk, nullptr);
		s_pipelineCacheVk = VK_NULL_HANDLE;
	}

	if (s_descriptorPool)
	{
		vkDestroyDescriptorPool(rhiContextVK->GetDevice()->logicalDevice, s_descriptorPool, nullptr);
		s_descriptorPool = VK_NULL_HANDLE;
	}
}

// TODO: 지금은 매 프레임 이 안에서 N프레임을 시작하기 전에 (N - MaxFrameCount) 프레임의 ImGui::AddTexture() 호출로 추가된 ResourceSet들을 제거함.
// N-1프레임을 제거하지 않는 이유는 Host-GPU 커맨드 정합성이 보장되지 않기 때문.
// InitializeBackend() 의 주석과 마찬가지로, 이상적으로는 RHI 백엔드의 ResourceSet을 Custom Allocator로 동적 할당해 제공해주고 
// Release(Lazy-Destroy)하는 방식으로 구현해야 함.
void ImGuiExtension::clearDeletedSwapchainData()
{
	for (auto& rSetList : s_AddedTexturesPerFrame)
	{
		for (void* rSet : rSetList)
		{
			if (rSet)
			{
				ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(rSet));
			}
		}
		rSetList.clear();
	}

	if (s_AddedTexturesPerFrame.size() < s_currentSwapchain->GetMaxFrameCount())
	{
		s_AddedTexturesPerFrame.resize(s_currentSwapchain->GetMaxFrameCount());
	}
}

HS_NS_EDITOR_END