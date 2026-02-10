#include "Platform/Mac/MacSystemContext.h"

#include "Core/HAL/FileSystem.h"
#include "Core/Log.h"

#include <unistd.h>

#ifdef __SDL__
#include <SDL3/SDL.h>

#else
#import <Cocoa/Cocoa.h>
#import <mach-o/dyld.h>
#endif

#ifdef __SDL__
static bool s_sdlInitialized = false;
HS_NS_BEGIN

bool SystemContext::initializePlatform()
{
    if (!s_sdlInitialized)
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        {
            HS_LOG(crash, "Failed to initialize SDL: %s", SDL_GetError());
            return false;
        }
        s_sdlInitialized = true;
    }

    executableDirectory = std::string(SDL_GetBasePath());
    executablePath      = executableDirectory + "HSMR";
    assetDirectory      = executableDirectory + "Assets" + HS_DIR_SEPERATOR;

    // Initialize AppData directory (~/Library/Application Support/HSMR/)
    @autoreleasepool
    {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        if ([paths count] > 0)
        {
            NSString* appSupportDir = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"HSMR"];
            appDataDirectory = std::string([appSupportDir UTF8String]) + HS_DIR_SEPERATOR;

            // Create directory if not exists
            FileSystem::CreateDirectoryRecursive(appDataDirectory);
        }

        // Initialize Documents directory
        paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        if ([paths count] > 0)
        {
            NSString* documentsDir = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"HSMR Projects"];
            userDocumentsDir = std::string([documentsDir UTF8String]) + HS_DIR_SEPERATOR;
        }
    }

    return true;
}

void SystemContext::finalizePlatform()
{
    if (s_sdlInitialized)
    {
        SDL_Quit();
        s_sdlInitialized = false;
    }
}

HS_NS_END

#else
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

        // Initialize AppData directory (~/Library/Application Support/HSMR/)
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        if ([paths count] > 0)
        {
            NSString* appSupportDir = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"HSMR"];
            appDataDirectory = std::string([appSupportDir UTF8String]) + HS_DIR_SEPERATOR;

            // Create directory if not exists
            FileSystem::CreateDirectoryRecursive(appDataDirectory);
        }

        // Initialize Documents directory
        paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        if ([paths count] > 0)
        {
            NSString* documentsDir = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"HSMR Projects"];
            userDocumentsDir = std::string([documentsDir UTF8String]) + HS_DIR_SEPERATOR;
        }
    }

    return true;
}

void SystemContext::finalizePlatform()
{
}
HS_NS_END
#endif
