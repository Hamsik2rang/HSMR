//
//  FileSystem.cpp
//  Engine
//
//  Created by Yongsik Im on 2/7/25.
//
#include "Engine/Core/FileSystem.h"
#include "Engine/Core/EngineContext.h"

#include <SDL3/SDL.h>

HS_NS_BEGIN

bool hs_file_copy(const char* src, const char* dst)
{
    SDL_Window* window = hs_engine_get_context()->sdlHandle.window;
    
    bool result = SDL_CopyFile(src, dst);
}
FileHandle hs_file_open(const char* absolutePath, EFileAccess access, FileHandle& outFileHandle)
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
bool hs_file_get_directory(const char* absolutePath, const char** outDirectory)
{
}
bool hs_file_get_resource_path(const char* relativePath, const char** outPath)
{
}
bool hs_file_get_relative_path(const char* absolutePath, const char** outRelativePath)
{
}
bool hs_file_get_absolute_path(const char* relativePath, const char** outAbsolutePath)
{
}

HS_NS_END
