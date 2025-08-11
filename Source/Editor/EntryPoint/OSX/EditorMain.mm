#include "Editor/EntryPoint/EditorMain.h"

#include "Core/EngineContext.h"
#include "Core/Renderer/Renderer.h"

#include "Editor/Core/EditorApplication.h"

#include <sys/stat.h>
#include <unistd.h>

#import <Cocoa/Cocoa.h>

using namespace HS;
using namespace HS::Editor;

int hs_editor_main(int argc, char* argv[])
{
    // TODO: Parse command arguments
    @autoreleasepool
    {
        EditorApplication* hsApp = new EditorApplication("HSMR");

        hs_platform_init(hsApp);

        EngineContext* engineContext = hs_engine_create_context("HSMR", ERHIPlatform::METAL);

        if ([NSApp delegate] == nil)
        {
            auto bp = true;
        }

        hsApp->Initialize(engineContext);

        hsApp->Run();
    }
    
    return 0;
}
