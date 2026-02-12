//
//  WinFileDialog.cpp
//  Platform
//
//  Created by Claude on 2/12/26.
//

#include "Platform/Win/WinFileDialog.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>
#include <ShlObj.h>

HS_NS_BEGIN

std::string FileDialog::OpenFile(const FileDialogFilter* filters, uint32 filterCount,
                                 const char* defaultLocation)
{
    char fileName[MAX_PATH] = {0};

    // Build filter string: "Name\0Pattern\0Name\0Pattern\0\0"
    std::string filterStr;
    if (filters && filterCount > 0)
    {
        for (uint32 i = 0; i < filterCount; ++i)
        {
            filterStr.append(filters[i].name);
            filterStr.push_back('\0');
            filterStr.append(filters[i].pattern);
            filterStr.push_back('\0');
        }
        filterStr.push_back('\0');
    }

    OPENFILENAMEA ofn    = {0};
    ofn.lStructSize      = sizeof(ofn);
    ofn.hwndOwner        = nullptr;
    ofn.lpstrFilter      = filterStr.empty() ? nullptr : filterStr.c_str();
    ofn.lpstrFile        = fileName;
    ofn.nMaxFile         = MAX_PATH;
    ofn.lpstrInitialDir  = defaultLocation;
    ofn.Flags            = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn))
    {
        return std::string(fileName);
    }
    return "";
}

std::string FileDialog::SaveFile(const FileDialogFilter* filters, uint32 filterCount,
                                 const char* defaultLocation)
{
    char fileName[MAX_PATH] = {0};

    std::string filterStr;
    if (filters && filterCount > 0)
    {
        for (uint32 i = 0; i < filterCount; ++i)
        {
            filterStr.append(filters[i].name);
            filterStr.push_back('\0');
            filterStr.append(filters[i].pattern);
            filterStr.push_back('\0');
        }
        filterStr.push_back('\0');
    }

    OPENFILENAMEA ofn    = {0};
    ofn.lStructSize      = sizeof(ofn);
    ofn.hwndOwner        = nullptr;
    ofn.lpstrFilter      = filterStr.empty() ? nullptr : filterStr.c_str();
    ofn.lpstrFile        = fileName;
    ofn.nMaxFile         = MAX_PATH;
    ofn.lpstrInitialDir  = defaultLocation;
    ofn.Flags            = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileNameA(&ofn))
    {
        return std::string(fileName);
    }
    return "";
}

std::string FileDialog::OpenFolder(const char* defaultLocation)
{
    BROWSEINFOA bi = {0};
    bi.lpszTitle   = "Select Folder";
    bi.ulFlags     = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl != nullptr)
    {
        char path[MAX_PATH];
        if (SHGetPathFromIDListA(pidl, path))
        {
            CoTaskMemFree(pidl);
            return std::string(path);
        }
        CoTaskMemFree(pidl);
    }
    return "";
}

HS_NS_END

#endif // _WIN32
