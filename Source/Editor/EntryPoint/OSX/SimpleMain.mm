#include "Editor/EntryPoint/SimpleMain.h"

#include "Editor/Core/SimpleApplication.h"

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

    return 0;
}
