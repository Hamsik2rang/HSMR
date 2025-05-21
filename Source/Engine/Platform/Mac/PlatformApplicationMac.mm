#include "Engine/Platform/Mac/PlatformApplicationMac.h"

#include "Engine/Core/Application.h"

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
    @autoreleasepool
    {
        _app->Run();
    }
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

- (void)applicationWillTerminate:(NSNotification*)notification
{
    _app->Finalize();
}

@end

HS_NS_BEGIN

void hs_platform_init(Application* hsApp)
{
    usleep(777777);
    
//    @autoreleasepool
//    {
        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        HSApplicationDelegate* appDelegate = [[HSApplicationDelegate alloc] initWithHSApplication:hsApp];
        [app setDelegate:appDelegate];
//    }
}

void hs_platform_shutdown(Application* hsApp)
{
}

HS_NS_END
