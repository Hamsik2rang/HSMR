#include "Engine/Core/FileSystem.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Core/Log.h"

#include <SDL3/SDL.h>

#define HS_DIRECTORY_SEPERATOR '/'

HS_NS_BEGIN

bool hs_file_copy(const std::string& src, const std::string& dst)
{
    bool result = SDL_CopyFile(src.c_str(), dst.c_str());
}
FileHandle hs_file_open(const std::string& absolutePath, EFileAccess access, FileHandle& outFileHandle)
{
}
bool hs_file_close(FileHandle fileHandle)
{
}
int64 hs_file_read(FileHandle fileHandle, void* buffer, size_t byteSize)
{
}

int64 hs_file_wrtie(FileHandle fileHandle, void* buffer, size_t byteSize)
{
}

bool hs_file_set_pos(FileHandle fileHandle, const int64 pos)
{
}

bool hs_file_flush(FileHandle fileHandle)
{
}

bool hs_file_is_eof(FileHandle fileHandle)
{
}

std::string hs_file_get_directory(const std::string& absolutePath)
{
    size_t delimIndex = absolutePath.find_last_of("/");
    std::string absoluteDirectory =  absolutePath.substr(0, delimIndex+1);
    
    return absoluteDirectory;
}

std::string hs_file_get_extension(const std::string& fileName)
{
    size_t delimIndex = fileName.find_last_of(".");
    //...

    return "";
}
std::string hs_file_get_resource_path(const std::string& relativePath)
{
    std::string absolutePath = std::string(hs_engine_get_context()->resourceDirectory) + relativePath;
    
    return absolutePath;
}

bool hs_file_is_absolute_path(const std::string& path)
{
    HS_ASSERT(path.length() > 0, "Invalid path");

    return (path[0] != '.');
}

std::string hs_file_get_relative_path(const std::string& absolutePath)
{
}

std::string hs_file_get_absolute_path(const std::string& relativePath)
{
}

HS_NS_END
