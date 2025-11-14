#include "Platform/Win/WinFileSystem.h"

#include <cstddef>
#include <string>
#include <algorithm>

// Windows API includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <direct.h>
#include <io.h>

// Link required libraries
#pragma comment(lib, "shlwapi.lib")

#include "Core/SystemContext.h"

HS_NS_BEGIN

// UTF-8 to UTF-16 conversion helper
std::wstring FileSystem::Utf8ToUtf16(const std::string& utf8)
{
    if (utf8.empty()) return std::wstring();

    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (size <= 0) return std::wstring();

    std::wstring result(size - 1, 0); // size includes null terminator
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], size);
    return result;
}

// UTF-16 to UTF-8 conversion helper
std::string FileSystem::Utf16ToUtf8(const std::wstring& utf16)
{
    if (utf16.empty()) return std::string();

    int size = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return std::string();

    std::string result(size - 1, 0); // size includes null terminator
    WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
}

// Convert EFileAccess to Windows access flags
DWORD get_access_flag(EFileAccess access)
{
    switch (access)
    {
    case EFileAccess::READ_ONLY:
        return GENERIC_READ;
    case EFileAccess::WRITE_ONLY:
        return GENERIC_WRITE;
    case EFileAccess::READ_WRITE:
        return GENERIC_READ | GENERIC_WRITE;
    default:
        return GENERIC_READ;
    }
}

// Convert EFileAccess to Windows creation disposition
DWORD get_createion_disposition(EFileAccess access)
{
    switch (access)
    {
    case EFileAccess::READ_ONLY:
        return OPEN_EXISTING;
    case EFileAccess::WRITE_ONLY:
    case EFileAccess::READ_WRITE:
        return OPEN_ALWAYS; // Open existing or create new
    default:
        return OPEN_EXISTING;
    }
}

bool FileSystem::Exist(const std::string& absolutePath)
{
    std::wstring pathW = FileSystem::Utf8ToUtf16(absolutePath);
    if (pathW.empty())
    {
        // HS_LOG(error, "Failed to convert file path to UTF-16: %s", absolutePath.c_str());
        return false;
    }
    DWORD attributes = GetFileAttributesW(pathW.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false; // File does not exist or error occurred
    }
    return true; // File exists
}

// 파일 복사 함수
bool FileSystem::Copy(const std::string& src, const std::string& dst)
{
    std::wstring srcW = FileSystem::Utf8ToUtf16(src);
    std::wstring dstW = FileSystem::Utf8ToUtf16(dst);

    if (srcW.empty() || dstW.empty())
    {
        // HS_LOG(error, "Failed to convert file paths to UTF-16");
        return false;
    }

    // CopyFileW automatically overwrites destination if it exists
    BOOL success = CopyFileW(srcW.c_str(), dstW.c_str(), FALSE);
    if (!success)
    {
        DWORD error = GetLastError();
        // HS_LOG(error, "Failed to copy file from %s to %s. Error: %lu", src.c_str(), dst.c_str(), error);
        return false;
    }

    return true;
}

// 파일 열기 함수
bool FileSystem::Open(const std::string& absolutePath, EFileAccess access, FileHandle& outFileHandle)
{
    std::wstring pathW = FileSystem::Utf8ToUtf16(absolutePath);
    if (pathW.empty())
    {
        // HS_LOG(error, "Failed to convert file path to UTF-16: %s", absolutePath.c_str());
        return false;
    }

    DWORD accessFlags         = get_access_flag(access);
    DWORD shareMode           = FILE_SHARE_READ; // Allow other processes to read
    DWORD creationDisposition = get_createion_disposition(access);
    DWORD flagsAndAttributes  = FILE_ATTRIBUTE_NORMAL;

    HANDLE handle = CreateFileW(
        pathW.c_str(),
        accessFlags,
        shareMode,
        nullptr, // Security attributes
        creationDisposition,
        flagsAndAttributes,
        nullptr // Template file
    );

    if (handle == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        // HS_LOG(error, "Failed to open file: %s. Error: %lu", absolutePath.c_str(), error);
        return false;
    }

    outFileHandle = static_cast<FileHandle>(handle);
    return true;
}

// 파일 닫기 함수
bool FileSystem::Close(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return false;
    }

    HANDLE handle = static_cast<HANDLE>(fileHandle);
    BOOL success  = CloseHandle(handle);

    if (!success)
    {
        DWORD error = GetLastError();
        // HS_LOG(error, "Failed to close file handle. Error: %lu", error);
        return false;
    }

    return true;
}

// 파일 읽기 함수
size_t FileSystem::Read(FileHandle fileHandle, void* buffer, size_t byteSize)
{
    if (!fileHandle || !buffer || byteSize == 0)
    {
        return 0;
    }

    HANDLE handle   = static_cast<HANDLE>(fileHandle);
    DWORD bytesRead = 0;

    BOOL success = ReadFile(handle, buffer, static_cast<DWORD>(byteSize), &bytesRead, nullptr);
    if (!success)
    {
        DWORD error = GetLastError();
        if (error != ERROR_HANDLE_EOF) // EOF is not an error
        {
            // HS_LOG(error, "Failed to read from file. Error: %lu", error);
        }
        return 0;
    }

    return static_cast<size_t>(bytesRead);
}

// 파일 쓰기 함수
size_t FileSystem::Write(FileHandle fileHandle, void* buffer, size_t byteSize)
{
    if (!fileHandle || !buffer || byteSize == 0)
    {
        return 0;
    }

    HANDLE handle      = static_cast<HANDLE>(fileHandle);
    DWORD bytesWritten = 0;

    BOOL success = WriteFile(handle, buffer, static_cast<DWORD>(byteSize), &bytesWritten, nullptr);
    if (!success)
    {
        DWORD error = GetLastError();
        // HS_LOG(error, "Failed to write to file. Error: %lu", error);
        return 0;
    }

    return static_cast<size_t>(bytesWritten);
}

// 파일 위치 설정 함수
bool FileSystem::SetPos(FileHandle fileHandle, const int64 pos)
{
    if (!fileHandle)
    {
        return false;
    }

    HANDLE handle = static_cast<HANDLE>(fileHandle);
    LARGE_INTEGER distance;
    distance.QuadPart = pos;

    BOOL success = SetFilePointerEx(handle, distance, nullptr, FILE_BEGIN);
    if (!success)
    {
        DWORD error = GetLastError();
        // HS_LOG(error, "Failed to set file position. Error: %lu", error);
        return false;
    }

    return true;
}

// 파일 버퍼 비우기 함수
bool FileSystem::Flush(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return false;
    }

    HANDLE handle = static_cast<HANDLE>(fileHandle);
    BOOL success  = FlushFileBuffers(handle);

    if (!success)
    {
        DWORD error = GetLastError();
        // HS_LOG(error, "Failed to flush file buffers. Error: %lu", error);
        return false;
    }

    return true;
}

// 파일 끝 확인 함수
bool FileSystem::IsEOF(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return true; // 유효하지 않은 핸들은 EOF로 간주
    }

    HANDLE handle = static_cast<HANDLE>(fileHandle);

    // Get current position
    LARGE_INTEGER currentPos;
    currentPos.QuadPart = 0;
    if (!SetFilePointerEx(handle, currentPos, &currentPos, FILE_CURRENT))
    {
        return true;
    }

    // Get file size
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(handle, &fileSize))
    {
        return true;
    }

    // Check if current position is at or beyond end of file
    return (currentPos.QuadPart >= fileSize.QuadPart);
}

// 파일 크기 확인 함수
size_t FileSystem::GetSize(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return 0;
    }

    HANDLE handle = static_cast<HANDLE>(fileHandle);
    LARGE_INTEGER fileSize;

    if (!GetFileSizeEx(handle, &fileSize))
    {
        DWORD error = GetLastError();
        // HS_LOG(error, "Failed to get file size. Error: %lu", error);
        return 0;
    }

    // For compatibility, return size_t (might truncate on 32-bit systems)
    return static_cast<size_t>(fileSize.QuadPart);
}

// 디렉토리 경로 얻기 함수
std::string FileSystem::GetDirectory(const std::string& absolutePath)
{
    if (absolutePath.empty())
    {
        return "";
    }

    std::string path = absolutePath;

    // Find last directory separator
    size_t lastSeparator = path.find_last_of("\\/");
    if (lastSeparator == std::string::npos)
    {
        return ""; // No directory separator found
    }

    std::string directory = path.substr(0, lastSeparator + 1);

    // Ensure trailing separator
    if (!directory.empty() && directory.back() != HS_DIR_SEPERATOR)
    {
        directory.push_back(HS_DIR_SEPERATOR);
    }

    return directory;
}

// 파일 확장자 얻기 함수
std::string FileSystem::GetExtension(const std::string& fileName)
{
    if (fileName.empty())
    {
        return "";
    }

    // Find last dot
    size_t lastDot = fileName.find_last_of('.');
    if (lastDot == std::string::npos)
    {
        return ""; // No extension found
    }

    // Make sure the dot is after the last directory separator
    size_t lastSeparator = fileName.find_last_of("\\/");
    if (lastSeparator != std::string::npos && lastDot < lastSeparator)
    {
        return ""; // Dot is in directory name, not extension
    }

    return fileName.substr(lastDot + 1);
}

// 절대 경로 확인 함수
bool FileSystem::IsAbsolutePath(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }

    // Check for Windows absolute paths:
    // 1. Drive letter (C:\, D:\, etc.)
    // 2. UNC path (\\server\share)
    // 3. Device path (\\?\, \\.\)

    if (path.length() >= 3 && path[1] == ':' && (path[2] == '\\' || path[2] == '/'))
    {
        return true; // Drive letter path
    }

    if (path.length() >= 2 && path[0] == '\\' && path[1] == '\\')
    {
        return true; // UNC or device path
    }

    return false;
}

// 실행 파일 기준 상대 경로 얻기 함수
std::string FileSystem::GetRelativePath(const std::string& absolutePath)
{
    std::string baseDir = SystemContext::Get()->executableDirectory;
    if (baseDir.empty())
    {
        return absolutePath; // If base directory is empty, return the absolute path
    }
    // Convert to lowercase for case-insensitive comparison (Windows is case-insensitive)
    std::string lowerAbsolute = absolutePath;
    std::string lowerBase     = baseDir;
    std::transform(lowerAbsolute.begin(), lowerAbsolute.end(), lowerAbsolute.begin(), ::tolower);
    std::transform(lowerBase.begin(), lowerBase.end(), lowerBase.begin(), ::tolower);

    if (lowerAbsolute.find(lowerBase) == 0)
    {
        std::string relative = absolutePath.substr(baseDir.length());
        if (!relative.empty() && (relative[0] == HS_DIR_SEPERATOR))
        {
            relative = relative.substr(1);
        }
        return relative;
    }

    return absolutePath;
}

// 실행 파일 기준 절대 경로 얻기 함수
std::string FileSystem::GetAbsolutePath(const std::string& relativePath)
{
    if (FileSystem::IsAbsolutePath(relativePath))
    {
        return relativePath;
    }

    std::string baseDir = SystemContext::Get()->executableDirectory;
    if (baseDir.empty())
    {
        return relativePath;
    }
    return baseDir + relativePath;
}

HS_NS_END