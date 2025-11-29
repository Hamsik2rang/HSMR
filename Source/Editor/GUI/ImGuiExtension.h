//
//  ImGuiExtension.h
//  Editor
//
//  Created by Yongsik Im on 2/15/25.
//

#ifndef __HS_IMGUI_EXTENSION_H__
#define __HS_IMGUI_EXTENSION_H__

#include "Precompile.h"

#include "ImGui/imgui.h"

/* #include "Core/Swapchain.h" */ namespace hs { struct Swapchain; }
/* #include "RHI/ResourceHandle.h" */ namespace hs { struct RHITexture; }
/* #include "RHI/CommandHandle.h" */ namespace hs { struct RHICommandBuffer; }

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API ImGuiExtension
{
public:
	static void ImageOffscreen(hs::RHITexture* use_texture, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));

	static void InitializeBackend(hs::Swapchain* swapchain);
	//void SetDisplaySize(uint32 width, uint32 height);
	static void BeginRender(hs::Swapchain* swapchain);
	static void EndRender();

	static void FinalizeBackend();
	
	static void SetProcessEventHandler(void** fnHandler);

private:
	static void clearDeletedSwapchainData();

	static std::vector<std::vector<void*>> s_AddedTexturesPerFrame;
	static uint8 s_currentImageIndex;
	static Swapchain* s_currentSwapchain;
};

HS_NS_EDITOR_END
#endif /* __HS_IMGUI_EXTENSION_H__ */
