#include "Editor/EntryPoint/EditorMain.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Renderer/Renderer.h"

#include "Editor/Core/EditorApplication.h"


#include <sys/stat.h>
#include <unistd.h>

#import <Cocoa/Cocoa.h>



int hs_editor_main(int argc, char* argv[])
{
//    NSApplication* app = [NSApplication sharedApplication];
//
//    //...
//
//    return NSApplicationMain(argc, (const char**)argv);

    // TODO: Parse command arguments

    // https://developer.apple.com/forums/thread/765445
    // to Prevent macOS bug. => apple is xxx.

    usleep(777777);

    HS::ERHIPlatform   rhiPlatform   = HS::ERHIPlatform::METAL;
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
