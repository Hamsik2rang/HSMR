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

namespace HS
{
	struct Texture;

	struct CommandBuffer;
	class Swapchain;
}


namespace ImGuiExt
{
HS_EDITOR_API void ImageOffscreen(HS::Texture* use_texture, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));

HS_EDITOR_API void InitializeBackend(HS::Swapchain* swapchain);
//void SetDisplaySize(uint32 width, uint32 height);
HS_EDITOR_API void ProcessEvent();
HS_EDITOR_API void BeginRender(HS::Swapchain* swapchain);
HS_EDITOR_API void EndRender(HS::Swapchain* swapchain);

HS_EDITOR_API void FinalizeBackend();


};

#endif /* __HS_IMGUI_EXTENSION_H__ */
