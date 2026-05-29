#include "Editor/EntryPoint/SimpleMain.h"

#include "Editor/Core/SimpleApplication.h"
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

int hs_simple_main(int argc, char* argv[])
{
    SystemContext::Init();

    @autoreleasepool
    {
        SimpleApplication* app = new SimpleApplication("HSMR Simple");

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
