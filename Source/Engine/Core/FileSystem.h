//
//  FileSystem.h
//  Engine
//
//  Created by Yongsik Im on 2/7/25.
//

#ifndef __HS_FILE_SYSTEM_H__
#define __HS_FILE_SYSTEM_H__

#include "Precompile.h"
#include <stddef.h>
#include <string>

#include <sys/stat.h>

HS_NS_BEGIN

enum class EFileAccess
{
    READ_ONLY,
    WRITE_ONLY,
    READ_WRITE
};

struct FileHandle
{
    void* ioStream;
};

bool hs_file_copy(const std::string& src, const std::string& dst);
FileHandle hs_file_open(const std::string& absolutePath, EFileAccess access, FileHandle& outFileHandle);
bool hs_file_close(FileHandle fileHandle);
int64 hs_file_read(FileHandle fileHandle, void* buffer, size_t byteSize);
int64 hs_file_wrtie(FileHandle fileHandle, void* buffer, size_t byteSize);
bool hs_file_set_pos(FileHandle fileHandle, const int64 pos);
bool hs_file_flush(FileHandle fileHandle);
bool hs_file_is_eof(FileHandle fileHandle);

std::string hs_file_get_directory(const std::string& absolutePath);
std::string hs_file_get_extension(const std::string& fileNmae);

std::string hs_file_get_resource_path(const std::string& relativePath);

bool hs_file_is_absolute_path(const std::string& path);
std::string hs_file_get_relative_path(const std::string& absolutePath);
std::string hs_file_get_absolute_path(const std::string& relativePath);

HS_NS_END

#endif /* FileSystem_h */
