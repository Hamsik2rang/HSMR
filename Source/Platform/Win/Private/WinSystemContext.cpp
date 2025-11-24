#include "Platform/Win/WinSystemContext.h"

#include "Core/SystemContext.h"
#include "Core/HAL/FileSystem.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

HS_NS_BEGIN

bool SystemContext::initializePlatform()
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

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
	assetDirectory = executableDirectory + "Assets" + HS_DIR_SEPERATOR;

	return true;
}

void SystemContext::finalizePlatform()
{
	//...
}

HS_NS_END
