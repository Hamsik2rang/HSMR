//
//  MacFileDialog.mm
//  Platform
//
//  Created by Claude on 2/12/26.
//

#include "Platform/Mac/MacFileDialog.h"

#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

HS_NS_BEGIN

std::string FileDialog::OpenFile(const FileDialogFilter* filters, uint32 filterCount,
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

        // Set file type filters
        if (filters && filterCount > 0)
        {
            NSMutableArray<UTType*>* contentTypes = [NSMutableArray array];
            for (uint32 i = 0; i < filterCount; ++i)
            {
                // Parse pattern like "*.scene" -> "scene"
                std::string pattern(filters[i].pattern);
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

std::string FileDialog::SaveFile(const FileDialogFilter* filters, uint32 filterCount,
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

        // Set file type filters
        if (filters && filterCount > 0)
        {
            NSMutableArray<UTType*>* contentTypes = [NSMutableArray array];
            for (uint32 i = 0; i < filterCount; ++i)
            {
                std::string pattern(filters[i].pattern);
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

std::string FileDialog::OpenFolder(const char* defaultLocation)
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

HS_NS_END
