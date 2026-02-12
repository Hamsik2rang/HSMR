//
//  SDLFileDialog_Mac.mm
//  Platform
//
//  Native macOS folder dialog with "New Folder" support.
//  SDL3's SDL_ShowOpenFolderDialog does not expose canCreateDirectories.
//

#include "Precompile.h"

#import <AppKit/AppKit.h>

std::string HSOpenFolderDialog_Native(const char* defaultLocation)
{
    @autoreleasepool
    {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:NO];
        [panel setCanChooseDirectories:YES];
        [panel setCanCreateDirectories:YES];
        [panel setAllowsMultipleSelection:NO];

        if (defaultLocation)
        {
            NSString* defaultDir = [NSString stringWithUTF8String:defaultLocation];
            [panel setDirectoryURL:[NSURL fileURLWithPath:defaultDir]];
        }

        if ([panel runModal] == NSModalResponseOK)
        {
            NSURL* url = [[panel URLs] firstObject];
            if (url)
            {
                return std::string([[url path] UTF8String]);
            }
        }
    }
    return "";
}
