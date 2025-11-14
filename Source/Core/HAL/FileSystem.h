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

enum class EFileAccess
{
    READ_ONLY,
    WRITE_ONLY,
    READ_WRITE
};

typedef void* FileHandle;

class FileSystem
{
public:
    enum class EAccessFlag
    {
        READ_ONLY,
        WRITE_ONLY,
        READ_WRITE
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

    static bool IsAbsolutePath(const std::string& path);
    static std::string GetRelativePath(const std::string& absolutePath);
    static std::string GetAbsolutePath(const std::string& relativePath);

    static std::wstring Utf8ToUtf16(const std::string& utf8);
    static std::string Utf16ToUtf8(const std::wstring& utf16);
};

HS_NS_END

#endif /* FileSystem_h */
