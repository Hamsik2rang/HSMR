#include "Platform/SDL/SDLFileSystem.h"

#include <cstddef>
#include <string>
#include <algorithm>

#include <SDL3/SDL.h>

#include "Core/SystemContext.h"

HS_NS_BEGIN

// UTF-8 to UTF-16 conversion helper
std::wstring FileSystem::Utf8ToUtf16(const std::string& utf8)
{
    if (utf8.empty()) return std::wstring();

    std::wstring result;
    result.reserve(utf8.size());

    size_t i = 0;
    while (i < utf8.size())
    {
        uint32_t codepoint = 0;
        unsigned char c    = static_cast<unsigned char>(utf8[i]);

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

        if (sizeof(wchar_t) == 2)
        {
            // UTF-16 (Windows)
            if (codepoint <= 0xFFFF)
            {
                result.push_back(static_cast<wchar_t>(codepoint));
            }
            else
            {
                codepoint -= 0x10000;
                result.push_back(static_cast<wchar_t>(0xD800 + (codepoint >> 10)));
                result.push_back(static_cast<wchar_t>(0xDC00 + (codepoint & 0x3FF)));
            }
        }
        else
        {
            // UTF-32 (Linux/macOS wchar_t is 32-bit)
            result.push_back(static_cast<wchar_t>(codepoint));
        }
    }

    return result;
}

// UTF-16 to UTF-8 conversion helper
std::string FileSystem::Utf16ToUtf8(const std::wstring& utf16)
{
    if (utf16.empty()) return std::string();

    std::string result;
    result.reserve(utf16.size() * 3);

    size_t i = 0;
    while (i < utf16.size())
    {
        uint32_t codepoint = 0;

        if (sizeof(wchar_t) == 2)
        {
            wchar_t w = utf16[i];
            if (w >= 0xD800 && w <= 0xDBFF)
            {
                if (i + 1 < utf16.size())
                {
                    wchar_t w2 = utf16[i + 1];
                    codepoint  = ((w - 0xD800) << 10) + (w2 - 0xDC00) + 0x10000;
                    i += 2;
                }
                else
                {
                    i += 1;
                    continue;
                }
            }
            else
            {
                codepoint = static_cast<uint32_t>(w);
                i += 1;
            }
        }
        else
        {
            codepoint = static_cast<uint32_t>(utf16[i]);
            i += 1;
        }

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

bool FileSystem::Exist(const std::string& absolutePath)
{
    SDL_PathInfo info;
    return SDL_GetPathInfo(absolutePath.c_str(), &info);
}

// 파일 복사 함수
bool FileSystem::Copy(const std::string& src, const std::string& dst)
{
    SDL_IOStream* srcStream = SDL_IOFromFile(src.c_str(), "rb");
    if (!srcStream)
    {
        return false;
    }

    SDL_IOStream* dstStream = SDL_IOFromFile(dst.c_str(), "wb");
    if (!dstStream)
    {
        SDL_CloseIO(srcStream);
        return false;
    }

    constexpr size_t bufferSize = 8192;
    uint8_t buffer[bufferSize];
    bool success = true;

    while (true)
    {
        size_t bytesRead = SDL_ReadIO(srcStream, buffer, bufferSize);
        if (bytesRead == 0)
        {
            break;
        }

        size_t bytesWritten = SDL_WriteIO(dstStream, buffer, bytesRead);
        if (bytesWritten != bytesRead)
        {
            success = false;
            break;
        }
    }

    SDL_CloseIO(dstStream);
    SDL_CloseIO(srcStream);

    return success;
}

// 파일 열기 함수
bool FileSystem::Open(const std::string& absolutePath, EFileAccess access, FileHandle& outFileHandle)
{
    const char* mode = nullptr;

    switch (access)
    {
    case EFileAccess::READ_ONLY:
        mode = "rb";
        break;

    case EFileAccess::WRITE_ONLY:
    case EFileAccess::READ_WRITE:
        // 파일이 없으면 생성 후 다시 열기
        if (!FileSystem::Exist(absolutePath))
        {
            SDL_IOStream* temp = SDL_IOFromFile(absolutePath.c_str(), "wb");
            if (!temp)
            {
                return false;
            }
            SDL_CloseIO(temp);
        }
        mode = "r+b";
        break;

    default:
        return false;
    }

    SDL_IOStream* stream = SDL_IOFromFile(absolutePath.c_str(), mode);
    if (!stream)
    {
        return false;
    }

    outFileHandle = static_cast<FileHandle>(stream);
    return true;
}

// 파일 닫기 함수
bool FileSystem::Close(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return false;
    }

    SDL_IOStream* stream = static_cast<SDL_IOStream*>(fileHandle);
    return SDL_CloseIO(stream);
}

// 파일 읽기 함수
size_t FileSystem::Read(FileHandle fileHandle, void* buffer, size_t byteSize)
{
    if (!fileHandle || !buffer || byteSize == 0)
    {
        return 0;
    }

    SDL_IOStream* stream = static_cast<SDL_IOStream*>(fileHandle);
    return SDL_ReadIO(stream, buffer, byteSize);
}

// 파일 쓰기 함수
size_t FileSystem::Write(FileHandle fileHandle, void* buffer, size_t byteSize)
{
    if (!fileHandle || !buffer || byteSize == 0)
    {
        return 0;
    }

    SDL_IOStream* stream = static_cast<SDL_IOStream*>(fileHandle);
    return SDL_WriteIO(stream, buffer, byteSize);
}

// 파일 위치 설정 함수
bool FileSystem::SetPos(FileHandle fileHandle, const int64 pos)
{
    if (!fileHandle)
    {
        return false;
    }

    SDL_IOStream* stream = static_cast<SDL_IOStream*>(fileHandle);
    Sint64 result        = SDL_SeekIO(stream, static_cast<Sint64>(pos), SDL_IO_SEEK_SET);
    return (result >= 0);
}

// 파일 버퍼 비우기 함수
bool FileSystem::Flush(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return false;
    }

    SDL_IOStream* stream = static_cast<SDL_IOStream*>(fileHandle);
    return SDL_FlushIO(stream);
}

// 파일 끝 확인 함수
bool FileSystem::IsEOF(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return true; // 유효하지 않은 핸들은 EOF로 간주
    }

    SDL_IOStream* stream = static_cast<SDL_IOStream*>(fileHandle);

    Sint64 currentPos = SDL_TellIO(stream);
    if (currentPos < 0)
    {
        return true;
    }

    Sint64 fileSize = SDL_GetIOSize(stream);
    if (fileSize < 0)
    {
        return true;
    }

    return (currentPos >= fileSize);
}

// 파일 크기 확인 함수
size_t FileSystem::GetSize(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return 0;
    }

    SDL_IOStream* stream = static_cast<SDL_IOStream*>(fileHandle);

    Sint64 fileSize = SDL_GetIOSize(stream);
    if (fileSize < 0)
    {
        return 0;
    }

    return static_cast<size_t>(fileSize);
}

// 디렉토리 경로 얻기 함수
std::string FileSystem::GetDirectory(const std::string& absolutePath)
{
    if (absolutePath.empty())
    {
        return "";
    }

    size_t lastSeparator = absolutePath.find_last_of(HS_DIR_SEPERATOR);
    if (lastSeparator == std::string::npos)
    {
        return "";
    }

    return absolutePath.substr(0, lastSeparator + 1);
}

// 파일 확장자 얻기 함수
std::string FileSystem::GetExtension(const std::string& fileName)
{
    if (fileName.empty())
    {
        return "";
    }

    size_t lastDot = fileName.find_last_of('.');
    if (lastDot == std::string::npos)
    {
        return "";
    }

    size_t lastSeparator = fileName.find_last_of(HS_DIR_SEPERATOR);
    if (lastSeparator != std::string::npos && lastDot < lastSeparator)
    {
        return "";
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

    if (path[0] == HS_DIR_SEPERATOR)
    {
        return true;
    }

    // Windows drive letter (C:\, D:\, etc.)
    if (path.length() >= 3 && path[1] == ':' && path[2] == HS_DIR_SEPERATOR)
    {
        return true;
    }

    return false;
}

// 실행 파일 기준 상대 경로 얻기 함수
std::string FileSystem::GetRelativePath(const std::string& absolutePath)
{
    std::string baseDir = SystemContext::Get()->executableDirectory;
    if (baseDir.empty())
    {
        return absolutePath;
    }

    // 대소문자 무시 비교 (Windows 호환)
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
