//
//  FileDialog.h
//  Core
//
//  Created by Claude on 2/12/26.
//

#ifndef __HS_FILE_DIALOG_H__
#define __HS_FILE_DIALOG_H__

#include "Precompile.h"

HS_NS_BEGIN

struct HS_CORE_API FileDialogFilter
{
    const char* name;    // e.g. "Scene Files"
    const char* pattern; // e.g. "*.scene"
};

class HS_CORE_API FileDialog
{
public:
    static std::string OpenFile(const FileDialogFilter* filters = nullptr,
                                uint32 filterCount = 0,
                                const char* defaultLocation = nullptr);

    static std::string SaveFile(const FileDialogFilter* filters = nullptr,
                                uint32 filterCount = 0,
                                const char* defaultLocation = nullptr);

    static std::string OpenFolder(const char* defaultLocation = nullptr);

    static bool OpenInExplorer(const std::string& path);
};

HS_NS_END

#endif /* FileDialog_h */
