#include "Engine/Core/FileSystem.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Core/Log.h"

#include <SDL3/SDL.h>

#include <string>

#define HS_DIRECTORY_SEPERATOR '/'

HS_NS_BEGIN

bool hs_file_copy(const std::string& src, const std::string& dst)
{
    bool result = SDL_CopyFile(src.c_str(), dst.c_str());

    return result;
}

bool hs_file_open(const std::string& absolutePath, EFileAccess access, FileHandle& outFileHandle)
{
    const char* mode;
    switch (access)
    {
        case EFileAccess::READ_ONLY:
        {
            mode = "rb";
            break;
        }
        case EFileAccess::WRITE_ONLY:
        {
            mode = "wb";
            break;
        }
        case EFileAccess::READ_WRITE:
        {
            mode = "r+b";
            break;
        }
        default:
        {
            outFileHandle = nullptr;
            return false;
        }
    }

    SDL_IOStream* file = SDL_IOFromFile(absolutePath.c_str(), mode);
    outFileHandle      = file;
    return (outFileHandle != nullptr);
}

bool hs_file_close(FileHandle fileHandle)
{
    if (!fileHandle)
    {
        return false;
    }
    return SDL_CloseIO(static_cast<SDL_IOStream*>(fileHandle));
}

size_t hs_file_read(FileHandle fileHandle, void* buffer, size_t byteSize)
{
    if (nullptr == fileHandle || nullptr == buffer)
    {
        return 0;
    }
    size_t readSize = SDL_ReadIO(reinterpret_cast<SDL_IOStream*>(fileHandle), buffer, byteSize);

    return readSize;
}

size_t hs_file_wrtie(FileHandle fileHandle, void* buffer, size_t byteSize)
{
    if (nullptr == fileHandle || nullptr == buffer)
    {
        return 0;
    }

    size_t writeSize = SDL_WriteIO(reinterpret_cast<SDL_IOStream*>(fileHandle), buffer, byteSize);

    return writeSize;
}

bool hs_file_set_pos(FileHandle fileHandle, const int64 pos)
{
    if (nullptr == fileHandle)
    {
        return false;
    }
    int64 result = SDL_SeekIO(reinterpret_cast<SDL_IOStream*>(fileHandle), pos, SDL_IO_SEEK_SET);
    return (result >= 0);
}

bool hs_file_flush(FileHandle fileHandle)
{
    if (nullptr == fileHandle)
    {
        return false;
    }
    //    // SDL3에는 직접적인 flush 함수가 없어서 현재 위치에서의 seek로 대체
    //    SDL_RWops* file       = reinterpret_cast<SDL_RWops*>(fileHandle);
    //    Sint64     currentPos = SDL_RWtell(file);
    //    if (currentPos < 0)
    //    {
    //        return false;
    //    }
    //    return SDL_RWseek(file, currentPos, RW_SEEK_SET) >= 0;

    return false;
}

bool hs_file_is_eof(FileHandle fileHandle)
{
    if (nullptr == fileHandle)
    {
        return true;
    }

    SDL_IOStream* file       = reinterpret_cast<SDL_IOStream*>(fileHandle);
    Sint64        currentPos = SDL_TellIO(file);
    Sint64        size       = SDL_GetIOSize(file);

    if (currentPos < 0 || size < 0)
    {
        return true;
    }

    return currentPos >= size;
}

size_t hs_file_get_size(FileHandle fileHandle)
{
    return SDL_GetIOSize(reinterpret_cast<SDL_IOStream*>(fileHandle));
}

std::string hs_file_get_directory(const std::string& absolutePath)
{
    auto index = absolutePath.find_last_of(HS_DIR_SEPERATOR);
    if (index == std::string::npos)
    {
        HS_THROW("Invalid Path!");
        return std::string();
    }
    std::string s = absolutePath.substr(0, index + 1);

    return s;
}

std::string hs_file_get_extension(const std::string& filePath)
{
    auto index = filePath.find_last_of(".");
    if (index == std::string::npos)
    {
        HS_THROW("Invalid Path!");
        return std::string();
    }
    std::string s = filePath.substr(0, index + 1);

    return s;
}

std::string hs_file_get_resource_path(const std::string& relativePath)
{
    return hs_engine_get_context()->executableDirectory + std::string("Resource/") + relativePath;
}

HS_NS_END
