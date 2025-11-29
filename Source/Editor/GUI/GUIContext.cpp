#include "Editor/GUI/GUIContext.h"

#include "Core/Log.h"
#include "Core/SystemContext.h"

#include "RHI/Swapchain.h"

#include "Editor/GUI/ImGuiExtension.h"

#include <string>

HS_NS_EDITOR_BEGIN

GUIContext::GUIContext()
    : _assetDirectory(SystemContext::Get()->assetDirectory)
    , _font{nullptr}
    , _context(nullptr)
{
    Initialize();
}

GUIContext::~GUIContext()
{}

void GUIContext::Initialize()
{
    IMGUI_CHECKVERSION();
    _context = ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
#if defined(__WINDOWS__)
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport
#endif
    std::string layoutPath = _assetDirectory + "imgui.ini";
    const char* filePath   = layoutPath.c_str();

    io.IniFilename = filePath;
    ImGui::LoadIniSettingsFromDisk(io.IniFilename);
    // Setup style
    SetColorTheme(false);

    ImGuiStyle& style = ImGui::GetStyle();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    std::string fontName = "Fonts/Malgun-Gothic.ttf";
    SetFont(fontName);
}
void GUIContext::NextFrame()
{
    ImGui::NewFrame();
}

void GUIContext::Finalize()
{
    std::string filePath = std::string(_assetDirectory + "imgui.ini");
    ImGui::SaveIniSettingsToDisk(filePath.c_str());
    ImGui::DestroyContext();
}

void GUIContext::SetColorTheme(bool useWhite)
{
    auto& styles = ImGui::GetStyle();

    styles.WindowRounding   = 5.0f;
    styles.TabRounding      = 0.0f;
    styles.FrameRounding    = 5.0f;
    styles.PopupRounding    = 5.0f;
    styles.WindowBorderSize = 0.0f;

    auto& colors = styles.Colors;
    if (useWhite)
    {
        ImGui::StyleColorsLight();

        colors[ImGuiCol_WindowBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
        colors[ImGuiCol_PopupBg]  = ImVec4(0.80f, 0.80f, 0.80f, 1.0f);

        colors[ImGuiCol_Header]        = ImVec4(0.76f, 0.76f, 0.76f, 1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4{0.63, 0.63f, 0.63f, 1.0f};
        colors[ImGuiCol_HeaderActive]  = ImVec4(0.59f, 0.59f, 0.59f, 1.0f);

        // Buttons
        colors[ImGuiCol_Button]        = ImVec4(0.72f, 0.72f, 0.72f, 1.0f);
        colors[ImGuiCol_ButtonHovered] = ImVec4{0.63, 0.63f, 0.63f, 1.0f};
        colors[ImGuiCol_ButtonActive]  = ImVec4(0.59f, 0.59f, 0.59f, 1.0f);

        // Frame BG
        colors[ImGuiCol_FrameBg]        = ImVec4{0.85f, 0.85f, 0.85f, 1.0f};
        colors[ImGuiCol_FrameBgHovered] = ImVec4{0.9f, 0.9f, 0.9f, 1.0f};
        colors[ImGuiCol_FrameBgActive]  = ImVec4{0.75f, 0.75f, 0.75f, 1.0f};

        // Tabs
        colors[ImGuiCol_Tab]                 = ImVec4{0.588f, 0.588f, 0.588f, 1.0f};
        colors[ImGuiCol_TabHovered]          = ImVec4{0.75f, 0.745f, 0.75f, 1.0f};
        colors[ImGuiCol_TabActive]           = ImVec4{0.9f, 0.9f, 0.9f, 1.0f};
        colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        colors[ImGuiCol_TabDimmedSelected]   = ImVec4(0.785f, 0.785f, 0.785f, 1.0f);
        colors[ImGuiCol_TabDimmedSelected]   = ImVec4(0.81f, 0.81f, 0.81f, 1.0f);
        //        colors[ImGuiCol_TabUnfocused]       = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        //        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

        colors[ImGuiCol_TitleBg]          = ImVec4{0.88f, 0.875f, 0.88f, 1.0f};
        colors[ImGuiCol_TitleBgActive]    = ImVec4{0.92f, 0.92f, 0.92f, 1.0f};
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        colors[ImGuiCol_TableHeaderBg]     = ImVec4(0.87f, 0.87f, 0.87f, 1.0f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.568f, 0.568f, 0.568f, 1.0f);
        colors[ImGuiCol_TableBorderLight]  = ImVec4(0.678f, 0.678f, 0.678f, 1.0f);

        colors[ImGuiCol_CheckMark]      = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
        colors[ImGuiCol_DockingPreview] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

        colors[ImGuiCol_SliderGrab]       = ImVec4(0.69f, 0.69f, 0.69f, 1.0f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.0f);

        colors[ImGuiCol_PlotHistogram] = ImVec4(0.26f, 0.59f, 1.00f, 0.00f);
    }
    else
    {
        ImGui::StyleColorsDark();

        colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

        // Headers
        colors[ImGuiCol_Header]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_HeaderActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Buttons
        colors[ImGuiCol_Button]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_ButtonActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Frame BG
        colors[ImGuiCol_FrameBg]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_FrameBgActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Tabs
        colors[ImGuiCol_Tab]                = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TabHovered]         = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
        colors[ImGuiCol_TabActive]          = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
        colors[ImGuiCol_TabUnfocused]       = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

        // Title
        colors[ImGuiCol_TitleBg]          = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TitleBgActive]    = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    }
}

void GUIContext::SetScaleFactor(float scaleFactor)
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.ScaleAllSizes(scaleFactor);

    ImGuiIO& io        = ImGui::GetIO();
    io.FontGlobalScale = scaleFactor;
}

// Font Push/Pop 어떻게?
void GUIContext::SetFont(const std::string& fontPath, float defaultFontSize)
{
    ImGuiIO& io = ImGui::GetIO();
    _font       = io.Fonts->AddFontFromFileTTF((_assetDirectory + fontPath).c_str(), defaultFontSize);

    io.Fonts->Build();
}

void GUIContext::LoadLayout(const std::string& layoutPath)
{
    std::string fullPath = layoutPath.empty() ? (_assetDirectory + "imgui.ini") : layoutPath;
    ImGui::LoadIniSettingsFromDisk(fullPath.c_str());

    #ifdef _DEBUG
    HS_LOG(debug, "GUI Layout info is loaded from %s", fullPath.c_str());
#endif
}

void GUIContext::SaveLayout(const std::string& layoutPath)
{
    std::string fullPath = layoutPath.empty() ? (_assetDirectory + "imgui.ini") : layoutPath;
    ImGui::SaveIniSettingsToDisk(fullPath.c_str());

#ifdef _DEBUG
    HS_LOG(debug, "GUI Layout info is saved into %s", fullPath.c_str());
#endif
}

void GUIContext::BeginRender(Swapchain* swapchain)
{
    _scaleFactor = swapchain->GetInfo().nativeWindow->scale;
    SetScaleFactor(_scaleFactor);

    ImGuiExtension::BeginRender(swapchain);
}

void GUIContext::EndRender()
{
    ImGuiExtension::EndRender();
    _scaleFactor = 1.0f / _scaleFactor;
    SetScaleFactor(_scaleFactor);
}

HS_NS_EDITOR_END
