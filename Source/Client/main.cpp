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

#ifdef __APPLE__
#include <unistd.h>
#endif

int main(int, char**)
{
    // https://developer.apple.com/forums/thread/765445
    // to Prevent macOS bug. => apple is xxx.
#ifdef __APPLE__
    usleep(777777);
#endif
    // TODO: Parse command arguments
#ifdef __APPLE__
	HS::ERHIPlatform rhiPlatform = HS::ERHIPlatform::METAL;
#else
    HS::ERHIPlatform rhiPlatform = HS::ERHIPlatform::VULKAN;
#endif
    HS::EngineContext* engineContext = HS::hs_engine_create_context(std::string("HSMR"), rhiPlatform);

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
