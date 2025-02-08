//
//  main.mm
//  HSMR
//
//  Created by Yongsik Im on 1/29/25.
//
#include "Engine/Core/EngineContext.h"
#include "Engine/Renderer/Renderer.h"


#include "Editor/Core/EditorApplication.h"

#include "ImGui/imgui.h"
#include "SDL3/SDL.h"

#include <sys/stat.h>
#include <stdio.h>
#include <string>

//void initialize_imgui_context()
//{
//    IMGUI_CHECKVERSION();
//    ImGuiContext* context = ImGui::CreateContext();
//    
//    HS::Editor::hs_editor_set_main_gui_context(context);
//}

int main(int, char**)
{
    // TODO: Parse command arguments

//    initialize_imgui_context();
    
    
    HS::EngineContext* engineContext = HS::hs_engine_create_context(std::string("HSMR"), HS::ERHIPlatform::METAL);

    HS::Editor::EditorApplication* editorApp = new HS::Editor::EditorApplication(engineContext->name);
    
    editorApp->Initialize(engineContext);

#if _DEBUG
    try
    {
#endif
        editorApp->Run();
#if _DEBUG
    }
    catch (std::exception& e)
    {
        HS_LOG(crash, "Exception throwed", e.what());
    }
#endif

    editorApp->Finalize();
    HS::hs_engine_destroy_context();

    return 0;
}
