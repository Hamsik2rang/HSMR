#include "Editor/EntryPoint/EditorMain.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Renderer/Renderer.h"

#include "Editor/Core/EditorApplication.h"

#include <sys/stat.h>
#include <unistd.h>

#import <Cocoa/Cocoa.h>

using namespace HS;
using namespace HS::Editor;

int hs_editor_main(int argc, char* argv[])
{
    EditorApplication* hsApp = new EditorApplication("HSMR");
    
    hs_platform_init(hsApp);
    
    EngineContext* engineContext = hs_engine_create_context("HSMR", ERHIPlatform::METAL);
    
    if([NSApp delegate] == nil)
    {
        auto bp = true;
    }
    
    hsApp->Initialize(engineContext);
    
    [NSApp run];

//    // TODO: Parse command arguments
//
//    // https://developer.apple.com/forums/thread/765445
//    // to Prevent macOS bug. => apple is xxx.
//
////    usleep(777777);
//
//    ERHIPlatform   rhiPlatform   = HS::ERHIPlatform::METAL;
//    EngineContext* engineContext = HS::hs_engine_create_context("HSMR", rhiPlatform);
//
//    Editor::EditorApplication* editorApp = new Editor::EditorApplication(engineContext->name);
//
//    editorApp->Initialize(engineContext);
//
//#if _DEBUG
//    try
//    {
//#endif
//        editorApp->Run();
//#if _DEBUG
//    }
//    catch (std::exception& e)
//    {
//        HS_LOG(crash, "Exception throwed", e.what());
//    }
//#endif
//
//    editorApp->Finalize();
//    hs_engine_destroy_context();

    return 0;
}
