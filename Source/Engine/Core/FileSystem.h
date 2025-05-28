//
//  FileSystem.h
//  Engine
//
//  Created by Yongsik Im on 2025.2.7
//

#ifndef __HS_FILE_SYSTEM_H__
#define __HS_FILE_SYSTEM_H__

#include "Precompile.h"


HS_NS_BEGIN

enum class EFileAccess
{
    READ_ONLY,
    WRITE_ONLY,
    READ_WRITE
};

typedef void* FileHandle;

bool hs_file_copy(const std::string& src, const std::string& dst);
bool hs_file_open(const std::string& absolutePath, EFileAccess access, FileHandle& outFileHandle);
bool hs_file_close(FileHandle fileHandle);
size_t hs_file_read(FileHandle fileHandle, void* buffer, size_t byteSize);
size_t hs_file_wrtie(FileHandle fileHandle, void* buffer, size_t byteSize);
bool hs_file_set_pos(FileHandle fileHandle, const int64 pos);
bool hs_file_flush(FileHandle fileHandle);
bool hs_file_is_eof(FileHandle fileHandle);
size_t hs_file_get_size(FileHandle fileHandle);

std::string hs_file_get_directory(const std::string& absolutePath);
std::string hs_file_get_extension(const std::string& fileNmae);

std::string hs_file_get_executable_path();
std::string hs_file_get_default_resource_directory();
std::string hs_file_get_default_resource_path(const std::string& relativePath);


bool hs_file_is_absolute_path(const std::string& path);
std::string hs_file_get_relative_path(const std::string& absolutePath);
std::string hs_file_get_absolute_path(const std::string& relativePath);

HS_NS_END

#endif /* FileSystem_h */
