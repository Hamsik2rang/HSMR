//
//  SDLFileDialog_Mac.mm
//  Platform
//
//  Native macOS file/folder dialogs.
//  SDL3's async dialog approach can deadlock when called during ImGui render frames.
//

#include "Precompile.h"

#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

struct HSFileDialogFilter
{
    const char* name;
    const char* pattern;
};

std::string HSOpenFileDialog_Native(const HSFileDialogFilter* filters, int filterCount,
                                    const char* defaultLocation)
{
    @autoreleasepool
    {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];

        if (defaultLocation)
        {
            NSString* defaultDir = [NSString stringWithUTF8String:defaultLocation];
            [panel setDirectoryURL:[NSURL fileURLWithPath:defaultDir]];
        }

        if (filters && filterCount > 0)
        {
            NSMutableArray<UTType*>* contentTypes = [NSMutableArray array];
            for (int i = 0; i < filterCount; ++i)
            {
                std::string pattern(filters[i].pattern);
                // Skip wildcard-all patterns like "*.*" or "*"
                if (pattern == "*.*" || pattern == "*")
                {
                    continue;
                }
                // Parse "*.ext" -> "ext"
                size_t dotPos = pattern.find('.');
                if (dotPos != std::string::npos)
                {
                    std::string ext = pattern.substr(dotPos + 1);
                    NSString* nsExt = [NSString stringWithUTF8String:ext.c_str()];
                    UTType* type = [UTType typeWithFilenameExtension:nsExt];
                    if (type)
                    {
                        [contentTypes addObject:type];
                    }
                }
            }
            if ([contentTypes count] > 0)
            {
                [panel setAllowedContentTypes:contentTypes];
            }
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

std::string HSSaveFileDialog_Native(const HSFileDialogFilter* filters, int filterCount,
                                    const char* defaultLocation)
{
    @autoreleasepool
    {
        NSSavePanel* panel = [NSSavePanel savePanel];

        if (defaultLocation)
        {
            NSString* defaultDir = [NSString stringWithUTF8String:defaultLocation];
            [panel setDirectoryURL:[NSURL fileURLWithPath:defaultDir]];
        }

        if (filters && filterCount > 0)
        {
            NSMutableArray<UTType*>* contentTypes = [NSMutableArray array];
            for (int i = 0; i < filterCount; ++i)
            {
                std::string pattern(filters[i].pattern);
                if (pattern == "*.*" || pattern == "*")
                {
                    continue;
                }
                size_t dotPos = pattern.find('.');
                if (dotPos != std::string::npos)
                {
                    std::string ext = pattern.substr(dotPos + 1);
                    NSString* nsExt = [NSString stringWithUTF8String:ext.c_str()];
                    UTType* type = [UTType typeWithFilenameExtension:nsExt];
                    if (type)
                    {
                        [contentTypes addObject:type];
                    }
                }
            }
            if ([contentTypes count] > 0)
            {
                [panel setAllowedContentTypes:contentTypes];
            }
        }

        if ([panel runModal] == NSModalResponseOK)
        {
            NSURL* url = [panel URL];
            if (url)
            {
                return std::string([[url path] UTF8String]);
            }
        }
    }
    return "";
}

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
