#include "Platform/Mac/MacSystemContext.h"

#include "Core/HAL/FileSystem.h"

#include <unistd.h>

#import <Cocoa/Cocoa.h>
#import <mach-o/dyld.h>

@interface HSApplicationDelegate : NSObject <NSApplicationDelegate>

@end

@implementation HSApplicationDelegate
{
    hs::SystemContext* _systemContext;
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    //...
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    //    hs::hs_engine_destroy_context();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return YES;
}

- (instancetype)initWithSystemContext:(hs::SystemContext*)context
{
    if (self)
    {
        _systemContext = context;
    }

    return self;
}

@end

HS_NS_BEGIN

static SystemContext* g_context;

bool SystemContext::initializePlatform()
{
    // https://developer.apple.com/forums/thread/765445
    // to Prevent macOS bug. => apple is xxxx.
    // usleep(777777);

    NSApplication* app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];

    HSApplicationDelegate* appDelegate = [[HSApplicationDelegate alloc] initWithSystemContext:g_context];
    [app setDelegate:appDelegate];

    @autoreleasepool
    {
        char path[PATH_MAX]{0};
        uint32 size = sizeof(path);
        if (0 != _NSGetExecutablePath(path, &size))
        {
            return false;
        }

        executablePath.resize(size);
        realpath(path, executablePath.data());
        executableDirectory = FileSystem::GetDirectory(executablePath);

        assetDirectory = executableDirectory + "Assets" + HS_DIR_SEPERATOR;
    }

    return true;
}

void SystemContext::finalizePlatform()
{
    
}
HS_NS_END
