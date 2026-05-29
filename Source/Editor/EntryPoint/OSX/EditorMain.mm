#include "Editor/EntryPoint/EditorMain.h"

#include "Renderer/Renderer.h"

#include "Editor/Core/EditorApplication.h"
#include "Core/HAL/CommandLine.h"
#include "Core/HAL/Timer.h"
#include "Core/Native/NativeWindow.h"
#include "Core/Profiler/ProfileDataCollector.h"
#include "Core/SystemContext.h"
#include "RHI/RHIContext.h"

#include <sys/stat.h>
#include <unistd.h>

#import <Cocoa/Cocoa.h>

using namespace hs;
using namespace hs::editor;

int hs_editor_main(int argc, char* argv[])
{
    SystemContext::Init();
    
    @autoreleasepool
    {
        EditorApplication* app = new EditorApplication("HSMR");
        
        HS_ASSERT([NSApp delegate] != nil, "NSApplication's Delegate is nil");
        
        app->Run();
        
        app->Shutdown();
        delete app;
    }

    FinalizeNativeWindowSystem();
    RHIContext::Destroy();
    SystemContext::Destroy();
    CommandLine::Finalize();
    ProfileDataCollector::Get().Finalize();
    Timer::Finalize();
    
    return 0;
}
