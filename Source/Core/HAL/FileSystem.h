//
//  FileSystem.h
//  Core
//
//  Created by Yongsik Im on 2/7/2025
//

#ifndef __HS_FILE_SYSTEM_H__
#define __HS_FILE_SYSTEM_H__

#include "Precompile.h"

HS_NS_BEGIN

enum class HS_API EFileAccess
{
    ReadOnly,
    WriteOnly,
    ReadWrite
};

typedef void* FileHandle;

class HS_API FileSystem
{
public:
    enum class EAccessFlag
    {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };

    static bool Exist(const std::string& absolutePath);
    static bool Copy(const std::string& src, const std::string& dst);
    static bool Open(const std::string& absolutePath, EFileAccess access, FileHandle& outFileHandle);
    static bool Close(FileHandle fileHandle);
    static size_t Read(FileHandle fileHandle, void* buffer, size_t byteSize);
    static size_t Write(FileHandle fileHandle, void* buffer, size_t byteSize);
    static bool SetPos(FileHandle fileHandle, const int64 pos);
    static bool Flush(FileHandle fileHandle);
    static bool IsEOF(FileHandle fileHandle);
    static size_t GetSize(FileHandle fileHandle);

    static std::string GetDirectory(const std::string& absolutePath);
    static std::string GetExtension(const std::string& fileNmae);
    static std::string GetFileName(const std::string& absolutePath);
    static std::string GetFileNameWithoutExtension(const std::string& absolutePath);

    static bool IsAbsolutePath(const std::string& path);
    static std::string GetRelativePath(const std::string& absolutePath);
    static std::string GetAbsolutePath(const std::string& relativePath);

    static bool IsDirectory(const std::string& path);
    static bool CreateDirectoryRecursive(const std::string& path);
    static std::vector<std::string> GetFilesInDirectory(const std::string& directory, bool recursive = false);
    static std::vector<std::string> GetSubDirectories(const std::string& directory);

    static std::wstring Utf8ToUtf16(const std::string& utf8);
    static std::string Utf16ToUtf8(const std::wstring& utf16);
};

HS_NS_END

#endif /* FileSystem_h */
