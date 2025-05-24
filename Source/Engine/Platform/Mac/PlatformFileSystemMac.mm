
#include "Engine/Platform/Mac/PlatformFileSystemMac.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Core/Log.h"

#include <cstddef>
#include <string>

#include <sys/stat.h>

#include <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>
#include <mach-o/dyld.h>

HS_NS_BEGIN
// 파일 복사 함수
bool hs_file_copy(const std::string& src, const std::string& dst)
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
bool hs_file_open(const std::string& absolutePath, EFileAccess access, FileHandle& outFileHandle)
{
    NSString* path = [NSString stringWithUTF8String:absolutePath.c_str()];

    // 파일 접근 모드 설정
    NSFileHandle* fileHandle = nil;

    switch (access)
    {
        case EFileAccess::READ_ONLY:
            fileHandle = [NSFileHandle fileHandleForReadingAtPath:path];
            break;

        case EFileAccess::WRITE_ONLY:
            // 파일이 없으면 생성
            if (![[NSFileManager defaultManager] fileExistsAtPath:path])
            {
                [[NSFileManager defaultManager] createFileAtPath:path contents:nil attributes:nil];
            }
            fileHandle = [NSFileHandle fileHandleForWritingAtPath:path];
            break;

        case EFileAccess::READ_WRITE:
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
bool hs_file_close(FileHandle fileHandle)
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
size_t hs_file_read(FileHandle fileHandle, void* buffer, size_t byteSize)
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

// 파일 쓰기 함수 (오타 수정: wrtie -> write)
size_t hs_file_write(FileHandle fileHandle, void* buffer, size_t byteSize)
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
bool hs_file_set_pos(FileHandle fileHandle, const int64 pos)
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
bool hs_file_flush(FileHandle fileHandle)
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
bool hs_file_is_eof(FileHandle fileHandle)
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
size_t hs_file_get_size(FileHandle fileHandle)
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
std::string hs_file_get_directory(const std::string& absolutePath)
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
std::string hs_file_get_extension(const std::string& fileName)
{
    @autoreleasepool
    {
        NSString* path      = [NSString stringWithUTF8String:fileName.c_str()];
        NSString* extension = [path pathExtension];
        return [extension UTF8String];
    }
}

std::string hs_file_get_executable_path()
{
    @autoreleasepool
    {
        char path[PATH_MAX] = {0};
        uint32_t size       = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0)
        {
            char resolvedPath[PATH_MAX] = {0};
            realpath(path, resolvedPath);
            return std::string(resolvedPath);
        }
        return "";
    }
}

// 리소스 경로 얻기 함수
std::string hs_file_get_default_resource_directory()
{
    std::string executablePath = hs_file_get_executable_path();
    std::string executableDir  = hs_file_get_directory(executablePath);

    std::string resourceRootDir = executableDir + "Resource" + HS_DIR_SEPERATOR;

    return resourceRootDir;
}

// Resource 루트 디렉터리 기준으로의 상대경로 얻기 함수
std::string hs_file_get_default_resource_path(const std::string& relativePath)
{
    return (hs_file_get_default_resource_directory() + relativePath);
}

// 절대 경로 확인 함수
bool hs_file_is_absolute_path(const std::string& path)
{
    @autoreleasepool
    {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        return [nsPath hasPrefix:@"/"] || [nsPath hasPrefix:@"file://"];
    }
}

// 실행 파일 기준 상대 경로 얻기 함수
std::string hs_file_get_relative_path(const std::string& absolutePath)
{
    std::string exePath = hs_file_get_executable_path();
    if (exePath.empty()) return absolutePath;

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
std::string hs_file_get_absolute_path(const std::string& relativePath)
{
    if (hs_file_is_absolute_path(relativePath))
        return relativePath;

    std::string exePath = hs_file_get_executable_path();
    if (exePath.empty()) return relativePath;

    std::string baseDir = exePath.substr(0, exePath.find_last_of(HS_DIR_SEPERATOR));
    return baseDir + HS_DIR_SEPERATOR + relativePath;
}
HS_NS_END
