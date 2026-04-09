#include "Editor/GUI/GUIContext.h"

#include "Core/Log.h"
#include "Core/SystemContext.h"
#include "Core/HAL/FileSystem.h"

#include "RHI/Swapchain.h"

#include "Editor/GUI/ImGuiExtension.h"

#include <string>

HS_NS_EDITOR_BEGIN

namespace
{
constexpr float s_editorIndentSpacingRatio = 0.25f;
constexpr float s_editorFramePaddingYRatio = 0.12f;
constexpr float s_editorItemSpacingYRatio = 0.20f;
constexpr float s_editorWindowPadding = 4.0f;
constexpr float s_fallbackEditorFontSize = 16.0f;
constexpr float s_materialSymbolFontScale = 1.25f;
constexpr const char* s_materialSymbolsFontPath = "Fonts/MaterialSymbolsRounded.ttf";

static const ImWchar s_materialSymbolsGlyphRanges[] =
{
    0xE000, 0xF8FF,
    0
};

void applyEditorStyleMetrics()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // ImGui's default metrics favor generic debug tools. They are comfortable, but editor panels such as
    // Hierarchy, Inspector, and Assets read more like Unity/Unreal when rows are compact and tree depth advances
    // only one or two spaces. Keep the policy centralized here so individual panels do not need to push the same
    // style overrides around every TreeNode, CollapsingHeader, Button, and Input widget.
    float fontSize = ImGui::GetFontSize();
    if (fontSize <= 0.0f)
    {
        fontSize = s_fallbackEditorFontSize;
    }

    style.IndentSpacing = fontSize * s_editorIndentSpacingRatio;
    style.FramePadding.y = std::max(2.0f, fontSize * s_editorFramePaddingYRatio);
    style.ItemSpacing.y = std::max(3.0f, fontSize * s_editorItemSpacingYRatio);
    style.WindowPadding = ImVec2(s_editorWindowPadding, s_editorWindowPadding);
    style.TabCloseButtonMinWidthSelected = 0.0f;
    style.TabCloseButtonMinWidthUnselected = 0.0f;
}

ImFont* addEditorFontWithIcons(const std::string& assetDirectory, const std::string& fontPath, float fontSize)
{
    ImGuiIO& io = ImGui::GetIO();

    ImFont* font = io.Fonts->AddFontFromFileTTF((assetDirectory + fontPath).c_str(), fontSize);
    if (!font)
    {
        HS_LOG(warning, "[GUI] Failed to load font: %s", fontPath.c_str());
    }

    std::string iconFontPath = assetDirectory + s_materialSymbolsFontPath;
    if (hs::FileSystem::Exist(iconFontPath))
    {
        ImFontConfig iconConfig;
        iconConfig.MergeMode = true;
        iconConfig.PixelSnapH = true;
        iconConfig.GlyphOffset.y = 2.0f;

        ImFont* iconFont = io.Fonts->AddFontFromFileTTF(
            iconFontPath.c_str(),
            fontSize * s_materialSymbolFontScale,
            &iconConfig,
            s_materialSymbolsGlyphRanges);

        if (!iconFont)
        {
            HS_LOG(warning, "[GUI] Failed to merge Material Symbols font: %s", iconFontPath.c_str());
        }
    }
    else
    {
        HS_LOG(warning, "[GUI] Material Symbols font is missing: %s", iconFontPath.c_str());
    }

    io.Fonts->Build();
    return font;
}
} // namespace

GUIContext::GUIContext()
    : _assetDirectory(SystemContext::Get()->assetDirectory)
    , _font{nullptr}
    , _context(nullptr)
    , _scaleFactor(1.0f)
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
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport (drag windows outside main window)
    // 기본 레이아웃 경로 (글로벌 — 프로젝트 열기 전까지 사용)
    _layoutPath = _assetDirectory + "imgui.ini";

    // ImGui 자동 저장 비활성화 (수동으로 제어)
    io.IniFilename = nullptr;

    // 글로벌 레이아웃 로드
    ImGui::LoadIniSettingsFromDisk(_layoutPath.c_str());
    // Setup style
    SetColorTheme(true);

    ImGuiStyle& style = ImGui::GetStyle();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
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
    ImGui::SaveIniSettingsToDisk(_layoutPath.c_str());
    ImGui::DestroyContext();
}

void GUIContext::SetColorTheme(bool useWhite)
{
    auto& styles = ImGui::GetStyle();

    styles.WindowRounding   = 0.0f;
    styles.TabRounding      = 0.0f;
    styles.FrameRounding    = 0.0f;
    styles.PopupRounding    = 0.0f;
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

    applyEditorStyleMetrics();
}

void GUIContext::SetScaleFactor(float scaleFactor)
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.ScaleAllSizes(scaleFactor);

    ImGuiIO& io        = ImGui::GetIO();
    io.FontGlobalScale = scaleFactor;

    applyEditorStyleMetrics();
}

void GUIContext::ApplyDPIScale(float dpiScale)
{
    if (dpiScale <= 0.0f || dpiScale == 1.0f)
    {
        _scaleFactor = 1.0f;
        return;
    }

    _scaleFactor = dpiScale;

    // Scale ImGui style sizes
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(dpiScale);

    // Reload font with scaled size
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    float scaledFontSize = 16.0f * dpiScale;
    std::string fontName = "Fonts/Malgun-Gothic.ttf";
    _font = addEditorFontWithIcons(_assetDirectory, fontName, scaledFontSize);

    applyEditorStyleMetrics();

    HS_LOG(info, "[GUI] Applied DPI scale: %.2f (font size: %.1f)", dpiScale, scaledFontSize);
}

// Font Push/Pop 어떻게?
void GUIContext::SetFont(const std::string& fontPath, float defaultFontSize)
{
    _font = addEditorFontWithIcons(_assetDirectory, fontPath, defaultFontSize);

    applyEditorStyleMetrics();
}

void GUIContext::LoadLayout(const std::string& layoutPath)
{
    std::string fullPath = layoutPath.empty() ? _layoutPath : layoutPath;
    ImGui::LoadIniSettingsFromDisk(fullPath.c_str());

#ifdef _DEBUG
    HS_LOG(debug, "GUI Layout info is loaded from %s", fullPath.c_str());
#endif
}

void GUIContext::SaveLayout(const std::string& layoutPath)
{
    std::string fullPath = layoutPath.empty() ? _layoutPath : layoutPath;
    ImGui::SaveIniSettingsToDisk(fullPath.c_str());

#ifdef _DEBUG
    HS_LOG(debug, "GUI Layout info is saved into %s", fullPath.c_str());
#endif
}

void GUIContext::SetLayoutPath(const std::string& layoutPath)
{
    _layoutPath = layoutPath;

    if (hs::FileSystem::Exist(layoutPath))
    {
        ImGui::LoadIniSettingsFromDisk(_layoutPath.c_str());
        HS_LOG(info, "[GUI] Loaded project layout: %s", _layoutPath.c_str());
    }
    else
    {
        HS_LOG(info, "[GUI] No project layout found, using current layout");
    }
}

void GUIContext::BeginRender(Swapchain* swapchain)
{
    ImGuiExtension::BeginRender(swapchain);
}

void GUIContext::EndRender()
{
    ImGuiExtension::EndRender();
}

HS_NS_EDITOR_END
