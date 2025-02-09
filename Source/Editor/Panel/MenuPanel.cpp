#include "Editor/Panel/MenuPanel.h"

#include "Engine/Core/Window.h"

#include "Editor/GUI/GUIContext.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

MenuPanel::~MenuPanel()
{
    
}

bool MenuPanel::Setup()
{
}

void MenuPanel::Cleanup()
{
}

void MenuPanel::Draw()
{
    static bool useWhite = true;
    
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            //  which we can't undo at the moment without finer window depth/z control.
            //             ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            //             ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if(ImGui::MenuItem("Show Demo", nullptr, false))
            {
                ImGui::ShowDemoWindow();
            }
            if(ImGui::MenuItem("Change Theme", nullptr, false))
            {
                hs_editor_get_gui_context()->SetColorTheme(useWhite);
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Close", nullptr, false))
            {
                _window->Close();
            }
            ImGui::EndMenu();
        }
        //        HelpMarker(
        //            "When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
        //            "- Drag from window title bar or their tab to dock/undock." "\n"
        //            "- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
        //            "- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)" "\n"
        //            "- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)" "\n"
        //            "This demo app has nothing to do with enabling docking!" "\n\n"
        //            "This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window." "\n\n"
        //            "Read comments in ShowExampleAppDockSpace() for more details.");

        if (ImGui::BeginMenu("Edit"))
        {
        }
        ImGui::EndMenuBar();
    }
}

HS_NS_EDITOR_END
