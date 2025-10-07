#include "Editor/EntryPoint/EditorMain.h"

#include "Engine/EngineContext.h"
#include "Renderer/RenderPath.h"

#include "Editor/Core/EditorApplication.h"

#include <sys/stat.h>
#include <unistd.h>

#import <Cocoa/Cocoa.h>

using namespace HS;
using namespace HS::Editor;

int hs_editor_main(int argc, char* argv[])
{
    EngineContext* engineContext = new EngineContext("HSMR", ERHIPlatform::METAL);
    // TODO: Parse command arguments
    @autoreleasepool
    {
        EditorApplication* hsApp = new EditorApplication("HSMR", engineContext);

        if ([NSApp delegate] == nil)
        {
            auto bp = true;
        }

        hsApp->Run();
    }
    
    delete engineContext;
    
    return 0;
}
