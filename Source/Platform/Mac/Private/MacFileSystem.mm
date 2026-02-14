
#include "Platform/Mac/MacFileSystem.h"

#include "Core/SystemContext.h"

#include <cstddef>
#include <string>
#include <vector>

#include <sys/stat.h>

#include <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>
#include <mach-o/dyld.h>

HS_NS_BEGIN

bool FileSystem::Exist(const std::string &absolutePath)
{
    
    
    return true;
}

// 파일 복사 함수
bool FileSystem::Copy(const std::string& src, const std::string& dst)
{
    @autoreleasepool
    {
        NSString* sourcePath = [NSString stringWithUTF8String:src.c_str()];
        NSString* destPath   = [NSString stringWithUTF8String:dst.c_str()];

        NSFileManager* fileManager = [NSFileManager defaultManager];
        NSError* error             = nil;

        // 목적지 파일이 이미 존재하면 삭제
        if ([fileManager fileExistsAtPath:destPath])
        {
            [fileManager removeItemAtPath:destPath error:&error];
            if (error)
            {
                return false;
            }
        }

        // 파일 복사
        BOOL success = [fileManager copyItemAtPath:sourcePath toPath:destPath error:&error];
        return (success && !error);
    }
}

// 파일 열기 함수
bool FileSystem::Open(const std::string& absolutePath, EFileAccess access, FileHandle& outFileHandle)
{
    NSString* path = [NSString stringWithUTF8String:absolutePath.c_str()];

    // 파일 접근 모드 설정
    NSFileHandle* fileHandle = nil;

    switch (access)
    {
        case EFileAccess::ReadOnly:
            fileHandle = [NSFileHandle fileHandleForReadingAtPath:path];
            break;

        case EFileAccess::WriteOnly:
            // 파일이 없으면 생성
            if (![[NSFileManager defaultManager] fileExistsAtPath:path])
            {
                [[NSFileManager defaultManager] createFileAtPath:path contents:nil attributes:nil];
            }
            fileHandle = [NSFileHandle fileHandleForWritingAtPath:path];
            break;

        case EFileAccess::ReadWrite:
            // 파일이 없으면 생성
            if (![[NSFileManager defaultManager] fileExistsAtPath:path])
            {
                [[NSFileManager defaultManager] createFileAtPath:path contents:nil attributes:nil];
            }
            fileHandle = [NSFileHandle fileHandleForUpdatingAtPath:path];
            break;
    }

    if (fileHandle)
    {
        // 메모리 관리를 위해 retain 카운트 증가
        outFileHandle = (__bridge void*)fileHandle;
        return true;
    }

    return false;
}

// 파일 닫기 함수
bool FileSystem::Close(FileHandle fileHandle)
{
    @autoreleasepool
    {
        if (!fileHandle)
        {
            return false;
        }

        NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;

        @try
        {
            [handle closeFile];
            return true;
        }
        @catch (NSException* exception)
        {
            return false;
        }
    }
}

// 파일 읽기 함수
size_t FileSystem::Read(FileHandle fileHandle, void* buffer, size_t byteSize)
{
    if (!fileHandle || !buffer)
    {
        return 0;
    }

    NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;

    @try
    {
        NSData* data = [handle readDataOfLength:byteSize];
        if (!data || [data length] == 0)
        {
            return 0;
        }

        // 데이터를 버퍼에 복사
        memcpy(buffer, [data bytes], [data length]);
        return [data length];
    }
    @catch (NSException* exception)
    {
        return 0;
    }
}
// 파일 쓰기 함수
size_t FileSystem::Write(FileHandle fileHandle, void* buffer, size_t byteSize)
{
    if (!fileHandle || !buffer)
    {
        return 0;
    }

    NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;

    @try
    {
        NSData* data = [NSData dataWithBytes:buffer length:byteSize];
        [handle writeData:data];
        return byteSize; // 성공 시 전체 바이트 크기 반환
    }
    @catch (NSException* exception)
    {
        return 0;
    }
}

// 파일 위치 설정 함수
bool FileSystem::SetPos(FileHandle fileHandle, const int64 pos)
{
    if (!fileHandle)
    {
        return false;
    }

    NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;

    @try
    {
        [handle seekToFileOffset:pos];
        return true;
    }
    @catch (NSException* exception)
    {
        return false;
    }
}

// 파일 버퍼 비우기 함수
bool FileSystem::Flush(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return false;
    }

    NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;

    @try
    {
        [handle synchronizeFile];
        return true;
    }
    @catch (NSException* exception)
    {
        return false;
    }
}

// 파일 끝 확인 함수
bool FileSystem::IsEOF(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return true; // 유효하지 않은 핸들은 EOF로 간주
    }

    NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;

    @try
    {
        // 현재 위치 저장
        unsigned long long currentOffset = [handle offsetInFile];

        // 파일 크기 확인
        [handle seekToEndOfFile];
        unsigned long long fileSize = [handle offsetInFile];

        // 원래 위치로 복원
        [handle seekToFileOffset:currentOffset];

        // 현재 위치가 파일 끝과 같거나 크면 EOF
        return (currentOffset >= fileSize);
    }
    @catch (NSException* exception)
    {
        return true;
    }
}

// 파일 크기 확인 함수
size_t FileSystem::GetSize(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return 0;
    }

    NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;

    @try
    {
        // 현재 위치 저장
        unsigned long long currentOffset = [handle offsetInFile];

        // 파일 끝으로 이동하여 크기 확인
        [handle seekToEndOfFile];
        unsigned long long fileSize = [handle offsetInFile];

        // 원래 위치로 복원
        [handle seekToFileOffset:currentOffset];

        return static_cast<size_t>(fileSize);
    }
    @catch (NSException* exception)
    {
        return 0;
    }
}

// 디렉토리 경로 얻기 함수
std::string FileSystem::GetDirectory(const std::string& absolutePath)
{
    @autoreleasepool
    {
        NSString* path      = [NSString stringWithUTF8String:absolutePath.c_str()];
        NSString* directory = [path stringByDeletingLastPathComponent];

        std::string result = [directory UTF8String];
        if (result.length() > 0 && result.back() != HS_DIR_SEPERATOR)
        {
            result.push_back(HS_DIR_SEPERATOR);
        }
        return result;
    }
}

// 파일 확장자 얻기 함수
std::string FileSystem::GetExtension(const std::string& fileName)
{
    @autoreleasepool
    {
        NSString* path      = [NSString stringWithUTF8String:fileName.c_str()];
        NSString* extension = [path pathExtension];
        return [extension UTF8String];
    }
}

// 절대 경로 확인 함수
bool FileSystem::IsAbsolutePath(const std::string& path)
{
    @autoreleasepool
    {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        return [nsPath hasPrefix:@"/"] || [nsPath hasPrefix:@"file://"];
    }
}

// 실행 파일 기준 상대 경로 얻기 함수
std::string FileSystem::GetRelativePath(const std::string& absolutePath)
{
    SystemContext* context = SystemContext::Get();

    std::string exePath = context->executablePath;
    if (exePath.empty())
    {
        return absolutePath;
    }

    std::string baseDir = exePath.substr(0, exePath.find_last_of(HS_DIR_SEPERATOR));

    if (absolutePath.find(baseDir) == 0)
    {
        std::string relative = absolutePath.substr(baseDir.length());
        if (!relative.empty() && relative[0] == HS_DIR_SEPERATOR)
            relative = relative.substr(1);
        return relative;
    }

    return absolutePath;
}

// 실행 파일 기준 절대 경로 얻기 함수
std::string FileSystem::GetAbsolutePath(const std::string& relativePath)
{
    if (IsAbsolutePath(relativePath))
    {
        return relativePath;
    }

    SystemContext* context = SystemContext::Get();

    std::string exePath = context->executablePath;
    if (exePath.empty())
    {
        return relativePath;
    }

    std::string baseDir = exePath.substr(0, exePath.find_last_of(HS_DIR_SEPERATOR));
    return baseDir + HS_DIR_SEPERATOR + relativePath;
}

std::wstring FileSystem::Utf8ToUtf16(const std::string& utf8)
{
    // macOS에서는 wchar_t가 32비트이므로 UTF-32로 변환
    if (utf8.empty()) return std::wstring();

    std::wstring result;
    result.reserve(utf8.size());

    size_t i = 0;
    while (i < utf8.size())
    {
        uint32_t codepoint = 0;
        unsigned char c = static_cast<unsigned char>(utf8[i]);

        if (c < 0x80)
        {
            codepoint = c;
            i += 1;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            codepoint = c & 0x1F;
            if (i + 1 < utf8.size()) codepoint = (codepoint << 6) | (utf8[i + 1] & 0x3F);
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            codepoint = c & 0x0F;
            if (i + 1 < utf8.size()) codepoint = (codepoint << 6) | (utf8[i + 1] & 0x3F);
            if (i + 2 < utf8.size()) codepoint = (codepoint << 6) | (utf8[i + 2] & 0x3F);
            i += 3;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            codepoint = c & 0x07;
            if (i + 1 < utf8.size()) codepoint = (codepoint << 6) | (utf8[i + 1] & 0x3F);
            if (i + 2 < utf8.size()) codepoint = (codepoint << 6) | (utf8[i + 2] & 0x3F);
            if (i + 3 < utf8.size()) codepoint = (codepoint << 6) | (utf8[i + 3] & 0x3F);
            i += 4;
        }
        else
        {
            i += 1;
            continue;
        }

        result.push_back(static_cast<wchar_t>(codepoint));
    }

    return result;
}

std::string FileSystem::Utf16ToUtf8(const std::wstring& utf16)
{
    if (utf16.empty()) return std::string();

    std::string result;
    result.reserve(utf16.size() * 4);

    for (wchar_t wc : utf16)
    {
        uint32_t codepoint = static_cast<uint32_t>(wc);

        if (codepoint < 0x80)
        {
            result.push_back(static_cast<char>(codepoint));
        }
        else if (codepoint < 0x800)
        {
            result.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
            result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
        else if (codepoint < 0x10000)
        {
            result.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
            result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
        else
        {
            result.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
            result.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
    }

    return result;
}

// 파일명 얻기 함수
std::string FileSystem::GetFileName(const std::string& absolutePath)
{
    @autoreleasepool
    {
        NSString* path     = [NSString stringWithUTF8String:absolutePath.c_str()];
        NSString* fileName = [path lastPathComponent];
        return [fileName UTF8String];
    }
}

// 확장자 없는 파일명 얻기 함수
std::string FileSystem::GetFileNameWithoutExtension(const std::string& absolutePath)
{
    @autoreleasepool
    {
        NSString* path     = [NSString stringWithUTF8String:absolutePath.c_str()];
        NSString* fileName = [[path lastPathComponent] stringByDeletingPathExtension];
        return [fileName UTF8String];
    }
}

// 디렉토리 확인 함수
bool FileSystem::IsDirectory(const std::string& path)
{
    @autoreleasepool
    {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        BOOL isDir       = NO;
        BOOL exists      = [[NSFileManager defaultManager] fileExistsAtPath:nsPath isDirectory:&isDir];
        return exists && isDir;
    }
}

// 디렉토리 재귀 생성 함수
bool FileSystem::CreateDirectoryRecursive(const std::string& path)
{
    @autoreleasepool
    {
        if (path.empty())
        {
            return false;
        }

        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSError* error   = nil;

        BOOL success = [[NSFileManager defaultManager] createDirectoryAtPath:nsPath
                                                 withIntermediateDirectories:YES
                                                                  attributes:nil
                                                                       error:&error];
        return success && !error;
    }
}

// 디렉토리 내 파일 목록 얻기 함수
std::vector<std::string> FileSystem::GetFilesInDirectory(const std::string& directory, bool recursive)
{
    std::vector<std::string> files;

    @autoreleasepool
    {
        NSString* nsPath        = [NSString stringWithUTF8String:directory.c_str()];
        NSFileManager* fileMgr  = [NSFileManager defaultManager];

        if (recursive)
        {
            NSDirectoryEnumerator* enumerator = [fileMgr enumeratorAtPath:nsPath];
            NSString* fileName;
            while ((fileName = [enumerator nextObject]))
            {
                NSString* fullPath = [nsPath stringByAppendingPathComponent:fileName];
                BOOL isDir         = NO;
                if ([fileMgr fileExistsAtPath:fullPath isDirectory:&isDir] && !isDir)
                {
                    files.push_back([fullPath UTF8String]);
                }
            }
        }
        else
        {
            NSError* error    = nil;
            NSArray* contents = [fileMgr contentsOfDirectoryAtPath:nsPath error:&error];
            if (!error)
            {
                for (NSString* fileName in contents)
                {
                    NSString* fullPath = [nsPath stringByAppendingPathComponent:fileName];
                    BOOL isDir         = NO;
                    if ([fileMgr fileExistsAtPath:fullPath isDirectory:&isDir] && !isDir)
                    {
                        files.push_back([fullPath UTF8String]);
                    }
                }
            }
        }
    }

    return files;
}

// 하위 디렉토리 목록 얻기 함수
std::vector<std::string> FileSystem::GetSubDirectories(const std::string& directory)
{
    std::vector<std::string> directories;

    @autoreleasepool
    {
        NSString* nsPath       = [NSString stringWithUTF8String:directory.c_str()];
        NSFileManager* fileMgr = [NSFileManager defaultManager];
        NSError* error         = nil;

        NSArray* contents = [fileMgr contentsOfDirectoryAtPath:nsPath error:&error];
        if (!error)
        {
            for (NSString* fileName in contents)
            {
                NSString* fullPath = [nsPath stringByAppendingPathComponent:fileName];
                BOOL isDir         = NO;
                if ([fileMgr fileExistsAtPath:fullPath isDirectory:&isDir] && isDir)
                {
                    directories.push_back([fullPath UTF8String]);
                }
            }
        }
    }

    return directories;
}

HS_NS_END
