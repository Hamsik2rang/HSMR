#include "Editor/GUI/ImGuiExtension.h"

#include "Engine/Core/Log.h"
#include "Engine/RHI/ResourceHandle.h"

using namespace HS;

namespace ImGuiExt
{

HS_EDITOR_API uint64 s_test = 0;

void ImageOffscreen(HS::Texture* use_texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	static uint64 i = 0;
	HS_LOG(debug, "IMAGE OFFSCREEN!!");

	s_test = i++;
}

void InitializeBackend(HS::Swapchain* swapchain)
{

}

void ProcessEvent()
{

}

void BeginRender(HS::Swapchain* swapchain)
{

}

void EndRender(HS::Swapchain* swapchain)
{

}

void FinalizeBackend()
{

}

}