#include "Editor/GUI/GUIContext.h"

#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_sdl3.h"
#include "ImGui/backends/imgui_impl_metal.h"

HS_NS_EDITOR_BEGIN

GUIContext* s_guiContext;

GUIContext::~GUIContext()
{
     Finalize();
}

void GUIContext::Initialize()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows -> Metal+SDL3에서 지원 안함
    
    

}

void GUIContext::Finalize()
{
    
}

GUIContext* hs_editor_get_gui_context()
{
}

HS_NS_EDITOR_END
