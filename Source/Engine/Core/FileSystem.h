//
//  FileSystem.h
//  Engine
//
//  Created by Yongsik Im on 2/7/25.
//

#ifndef __HS_FILE_SYSTEM_H__
#define __HS_FILE_SYSTEM_H__

#include "Precompile.h"

#Include < cstdio>

HS_NS_BEGIN

enum class EFileAccess
{
    READ_ONLY,
    WRITE_ONLY,
    READ_WRITE
};

struct FileHandle
{
#define HS_INVALID_HANDLE -1

    int nativeHandle = HS_INVALID_HANDLE;
};

bool hs_file_copy(const char* src, const char* dst);
FileHandle hs_file_open(const char* absolutePath, EFileAccess access, FileHandle& outFileHandle);
bool hs_file_close(FileHandle fileHandle);
int64 hs_file_read(FileHandle fileHandle, void* buffer, size_t byteSize);
int64 hs_file_wrtie(FileHandle fileHandle, void* buffer, size_t byteSize);
bool hs_file_set_pos(FileHandle fileHandle, const int64 pos);
bool hs_file_flush(FileHandle fileHandle);
bool hs_file_is_eof(FileHandle fileHandle);
bool hs_file_get_directory(const char* absolutePath, const char** outDirectory);
bool hs_file_get_resource_path(const char* relativePath, const char** outPath);
bool hs_file_get_relative_path(const char* absolutePath, const char** outRelativePath);
bool hs_file_get_absolute_path(const char* relativePath, const char** outAbsolutePath);

HS_NS_END

#endif /* FileSystem_h */
