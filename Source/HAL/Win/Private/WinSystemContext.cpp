#include "HAL/Win/WinSystemContext.h"

#include "HAL/FileSystem.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

HS_NS_BEGIN

bool SystemContext::Initialize()
{
	char path[MAX_PATH] = { 0 };
	DWORD length = GetModuleFileNameA(nullptr, path, MAX_PATH);

	if (length == 0 || length == MAX_PATH)
	{
		// Try with longer buffer for long paths
		WCHAR longPath[HS_CHAR_INIT_LONG_LENGTH] = { 0 };
		DWORD longLength = GetModuleFileNameW(nullptr, longPath, HS_CHAR_INIT_LONG_LENGTH);

		if (longLength > 0 && longLength < 32768)
		{
			executablePath = FileSystem::Utf16ToUtf8(std::wstring(longPath));
		}
	}
	else
	{
		executablePath = std::string(path);
	}

	executableDirectory = FileSystem::GetDirectory(executablePath);
	resourceDirectory = executableDirectory + "Resource" + HS_DIR_SEPERATOR;

	return true;
}

void SystemContext::Finalize()
{
	//...
}

HS_NS_END
