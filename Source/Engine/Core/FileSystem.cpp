#include "Engine/Core/FileSystem.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Core/Log.h"

#include <string>

#define HS_DIRECTORY_SEPERATOR '/'

HS_NS_BEGIN
// 파일 복사 함수
bool hs_file_copy(const std::string& src, const std::string& dst)
{
    @autoreleasepool {
        NSString* sourcePath = [NSString stringWithUTF8String:src.c_str()];
        NSString* destPath = [NSString stringWithUTF8String:dst.c_str()];
        
        NSFileManager* fileManager = [NSFileManager defaultManager];
        NSError* error = nil;
        
        // 목적지 파일이 이미 존재하면 삭제
        if ([fileManager fileExistsAtPath:destPath]) {
            [fileManager removeItemAtPath:destPath error:&error];
            if (error) {
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
    @autoreleasepool {
        NSString* path = [NSString stringWithUTF8String:absolutePath.c_str()];
        
        // 파일 접근 모드 설정
        NSFileHandle* fileHandle = nil;
        
        switch (access) {
            case EFileAccess::READ_ONLY:
                fileHandle = [NSFileHandle fileHandleForReadingAtPath:path];
                break;
                
            case EFileAccess::WRITE_ONLY:
                // 파일이 없으면 생성
                if (![[NSFileManager defaultManager] fileExistsAtPath:path]) {
                    [[NSFileManager defaultManager] createFileAtPath:path contents:nil attributes:nil];
                }
                fileHandle = [NSFileHandle fileHandleForWritingAtPath:path];
                break;
                
            case EFileAccess::READ_WRITE:
                // 파일이 없으면 생성
                if (![[NSFileManager defaultManager] fileExistsAtPath:path]) {
                    [[NSFileManager defaultManager] createFileAtPath:path contents:nil attributes:nil];
                }
                fileHandle = [NSFileHandle fileHandleForUpdatingAtPath:path];
                break;
        }
        
        if (fileHandle) {
            // 메모리 관리를 위해 retain 카운트 증가
            outFileHandle = (__bridge_retained void*)fileHandle;
            return true;
        }
        
        return false;
    }
}

// 파일 닫기 함수
bool hs_file_close(FileHandle fileHandle)
{
    @autoreleasepool {
        if (!fileHandle) {
            return false;
        }
        
        NSFileHandle* handle = (__bridge_transfer NSFileHandle*)fileHandle;
        
        @try {
            [handle closeFile];
            return true;
        }
        @catch (NSException* exception) {
            return false;
        }
    }
}

// 파일 읽기 함수
size_t hs_file_read(FileHandle fileHandle, void* buffer, size_t byteSize)
{
    @autoreleasepool {
        if (!fileHandle || !buffer) {
            return 0;
        }
        
        NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;
        
        @try {
            NSData* data = [handle readDataOfLength:byteSize];
            if (!data || [data length] == 0) {
                return 0;
            }
            
            // 데이터를 버퍼에 복사
            memcpy(buffer, [data bytes], [data length]);
            return [data length];
        }
        @catch (NSException* exception) {
            return 0;
        }
    }
}

// 파일 쓰기 함수 (오타 수정: wrtie -> write)
size_t hs_file_write(FileHandle fileHandle, void* buffer, size_t byteSize)
{
    @autoreleasepool {
        if (!fileHandle || !buffer) {
            return 0;
        }
        
        NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;
        
        @try {
            NSData* data = [NSData dataWithBytes:buffer length:byteSize];
            [handle writeData:data];
            return byteSize; // 성공 시 전체 바이트 크기 반환
        }
        @catch (NSException* exception) {
            return 0;
        }
    }
}

// 파일 위치 설정 함수
bool hs_file_set_pos(FileHandle fileHandle, const int64 pos)
{
    @autoreleasepool {
        if (!fileHandle) {
            return false;
        }
        
        NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;
        
        @try {
            [handle seekToFileOffset:pos];
            return true;
        }
        @catch (NSException* exception) {
            return false;
        }
    }
}

// 파일 버퍼 비우기 함수
bool hs_file_flush(FileHandle fileHandle)
{
    @autoreleasepool {
        if (!fileHandle) {
            return false;
        }
        
        NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;
        
        @try {
            [handle synchronizeFile];
            return true;
        }
        @catch (NSException* exception) {
            return false;
        }
    }
}

// 파일 끝 확인 함수
bool hs_file_is_eof(FileHandle fileHandle)
{
    @autoreleasepool {
        if (!fileHandle) {
            return true; // 유효하지 않은 핸들은 EOF로 간주
        }
        
        NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;
        
        @try {
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
        @catch (NSException* exception) {
            return true;
        }
    }
}

// 파일 크기 확인 함수
size_t hs_file_get_size(FileHandle fileHandle)
{
    @autoreleasepool {
        if (!fileHandle) {
            return 0;
        }
        
        NSFileHandle* handle = (__bridge NSFileHandle*)fileHandle;
        
        @try {
            // 현재 위치 저장
            unsigned long long currentOffset = [handle offsetInFile];
            
            // 파일 끝으로 이동하여 크기 확인
            [handle seekToEndOfFile];
            unsigned long long fileSize = [handle offsetInFile];
            
            // 원래 위치로 복원
            [handle seekToFileOffset:currentOffset];
            
            return static_cast<size_t>(fileSize);
        }
        @catch (NSException* exception) {
            return 0;
        }
    }
}

// 디렉토리 경로 얻기 함수
std::string hs_file_get_directory(const std::string& absolutePath)
{
    @autoreleasepool {
        NSString* path = [NSString stringWithUTF8String:absolutePath.c_str()];
        NSString* directory = [path stringByDeletingLastPathComponent];
        return [directory UTF8String];
    }
}

// 파일 확장자 얻기 함수
std::string hs_file_get_extension(const std::string& fileName)
{
    @autoreleasepool {
        NSString* path = [NSString stringWithUTF8String:fileName.c_str()];
        NSString* extension = [path pathExtension];
        return [extension UTF8String];
    }
}

// 리소스 경로 얻기 함수
std::string hs_file_get_resource_path(const std::string& relativePath)
{
    @autoreleasepool {
        NSString* path = [NSString stringWithUTF8String:relativePath.c_str()];
        
        // 확장자 및 디렉토리 분리
        NSString* directory = [path stringByDeletingLastPathComponent];
        NSString* filename = [path lastPathComponent];
        NSString* name = [filename stringByDeletingPathExtension];
        NSString* extension = [filename pathExtension];
        
        // 번들에서 리소스 경로 찾기
        NSString* resourcePath;
        
        if ([extension length] > 0) {
            resourcePath = [[NSBundle mainBundle] pathForResource:name ofType:extension inDirectory:directory];
        } else {
            // 확장자가 없는 경우
            resourcePath = [[NSBundle mainBundle] pathForResource:name ofType:nil inDirectory:directory];
        }
        
        if (resourcePath) {
            return [resourcePath UTF8String];
        }
        
        // 리소스를 찾지 못한 경우 원래 경로 반환
        return relativePath;
    }
}

// 절대 경로 확인 함수
bool hs_file_is_absolute_path(const std::string& path)
{
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        return [nsPath hasPrefix:@"/"] || [nsPath hasPrefix:@"file://"];
    }
}

// 상대 경로 얻기 함수
std::string hs_file_get_relative_path(const std::string& absolutePath)
{
    @autoreleasepool {
        NSString* absPath = [NSString stringWithUTF8String:absolutePath.c_str()];
        NSString* bundlePath = [[NSBundle mainBundle] bundlePath];
        
        // 번들 경로로 시작하면 상대 경로 계산
        if ([absPath hasPrefix:bundlePath]) {
            NSString* relativePath = [absPath substringFromIndex:[bundlePath length]];
            
            // 경로가 '/'로 시작하면 제거
            if ([relativePath hasPrefix:@"/"]) {
                relativePath = [relativePath substringFromIndex:1];
            }
            
            return [relativePath UTF8String];
        }
        
        // 절대 경로가 번들 경로 외부에 있으면 원래 경로 반환
        return absolutePath;
    }
}

// 절대 경로 얻기 함수
std::string hs_file_get_absolute_path(const std::string& relativePath)
{
    @autoreleasepool {
        // 이미 절대 경로인지 확인
        if (hs_file_is_absolute_path(relativePath)) {
            return relativePath;
        }
        
        NSString* relPath = [NSString stringWithUTF8String:relativePath.c_str()];
        NSString* bundlePath = [[NSBundle mainBundle] bundlePath];
        NSString* absolutePath = [bundlePath stringByAppendingPathComponent:relPath];
        
        return [absolutePath UTF8String];
    }
}

std::string hs_file_get_resource_path(const std::string& relativePath)
{
    return hs_engine_get_context()->executableDirectory + std::string("Resource/") + relativePath;
}

HS_NS_END
