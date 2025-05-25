#include "Engine/Platform/Mac/PlatformApplicationMac.h"

#include "Engine/Core/Application.h"
#include "Engine/Core/EngineContext.h"

#include <unistd.h>
#import <Cocoa/Cocoa.h>

@interface HSApplicationDelegate : NSObject <NSApplicationDelegate>

@end

@implementation HSApplicationDelegate
{
    HS::Application* _app;
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    //...
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    _app->Finalize();
    HS::hs_engine_destroy_context();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return YES;
}

- (instancetype)initWithHSApplication:(HS::Application*)hsApp
{
    if (self)
    {
        _app = hsApp;
    }

    return self;
}

@end

HS_NS_BEGIN

void hs_platform_init(Application* hsApp)
{
    // https://developer.apple.com/forums/thread/765445
    // to Prevent macOS bug. => apple is xxx.
    usleep(777777);

    NSApplication* app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];

    HSApplicationDelegate* appDelegate = [[HSApplicationDelegate alloc] initWithHSApplication:hsApp];
    [app setDelegate:appDelegate];
}

void hs_platform_shutdown(Application* hsApp)
{
}

HS_NS_END
