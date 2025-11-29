#include "Editor/EntryPoint/EditorMain.h"

#include "Engine/EngineContext.h"
#include "Engine/Renderer/RenderPath.h"

#include "Editor/Core/EditorApplication.h"

#include <sys/stat.h>
#include <unistd.h>

#import <Cocoa/Cocoa.h>

using namespace hs;
using namespace hs::editor;

int hs_editor_main(int argc, char* argv[])
{
    SystemContext::Init();
    
    // TODO: Parse command arguments
    @autoreleasepool
    {
        EditorApplication* app = new EditorApplication("HSMR");
        
        if ([NSApp delegate] == nil)
        {
            auto bp = true;
        }
        
        app->Run();
        
        app->Shutdown();
    }
    
    return 0;
}
