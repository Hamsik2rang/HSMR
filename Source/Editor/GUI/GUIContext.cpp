#include "Editor/GUI/GUIContext.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/FileSystem.h"

#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_sdl3.h"
#include "ImGui/backends/imgui_impl_metal.h"

#include "Engine/Renderer/Renderer.h"

#include <string>

HS_NS_EDITOR_BEGIN

GUIContext* s_guiContext = nullptr;

GUIContext::GUIContext()
    : layoutFileName(std::string("imgui.ini"))
    , _font{nullptr}
    , _context(nullptr)
{}

GUIContext::~GUIContext()
{
    Finalize();
}
void GUIContext::Initialize()
{
    IMGUI_CHECKVERSION();
    _context = ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    
    layoutFileName = "imgui.ini";

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows -> Metal+SDL3에서 지원 안함
    HS_LOG(debug, "%s", layoutFileName.c_str());
    io.IniFilename = hs_file_get_resource_path(layoutFileName).c_str();
    chmod(io.IniFilename, S_IRWXU);

    // Setup style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding              = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}
void GUIContext::NextFrame()
{
    ImGui::NewFrame();
}

void GUIContext::Finalize()
{
    ImGui::DestroyContext();
}

void GUIContext::SetColorTheme(bool useWhite)
{
}

void GUIContext::SetFont(std::string& fontPath, float fontSize)
{
    ImGui::PopFont();
    ImGuiIO& io = ImGui::GetIO();
    _font       = io.Fonts->AddFontFromFileTTF(hs_file_get_resource_path(fontPath).c_str(), 18.0f);

    io.Fonts->Build();

    ImGui::PushFont(_font);
}

HS_NS_EDITOR_END
