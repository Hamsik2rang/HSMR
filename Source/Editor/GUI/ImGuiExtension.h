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
class Texture;
class CommandBuffer;
class Swapchain;
}


namespace ImGuiExt
{
void ImageOffscreen(HS::Texture* use_texture, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));

void InitializeBackend(HS::Swapchain* swapchain);
void ProcessEvent();
void BeginRender(HS::Swapchain* swapchain);
void EndRender(HS::Swapchain* swapchain);

void FinalizeBackend();


};

#endif /* __HS_IMGUI_EXTENSION_H__ */
